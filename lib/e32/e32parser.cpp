// Copyright (c) 2018-2020 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Contributors:
//
// Description:
// Parse E32 Images
//
//

#include <cstring>
#include <cstdlib>

#include "common.hpp"
#include "e32common.h"
#include "e32parser.h"
#include "e32compressor.h"
#include "elf2e32_opt.hpp"

int32_t Adjust(int32_t size);

E32Parser* E32Parser::NewL(const std::string& arg)
{
    E32Parser* self = new E32Parser(arg);
    self->ConstructL();
    self->PostConstructL();
    return self;
}

E32Parser* E32Parser::NewL(const std::vector<char>& e32File)
{
    E32Parser* self = new E32Parser();
    self->ConstructL(e32File);
    self->PostConstructL();
    return self;
}

E32Parser::~E32Parser()
{
    delete iBufferedFile;
}

E32Parser::E32Parser(const std::string& arg):
    iE32File(arg)
{}

void E32Parser::ConstructL()
{
    iBufferedFile = (char*)ReadFile(iE32File.c_str(), iE32Size);

    if(!iBufferedFile)
        ReportError(ZEROBUFFER, "Buffered E32Image not set at all.");

    if(!iE32Size)
        ReportError(ZEROBUFFER, "Size for buffered E32Image not set.");

    if((iBufferedFile[1] == 'E')&&(iBufferedFile[2] == 'L')&&(iBufferedFile[3] == 'F'))
        ReportError(ELFFILEEXPECTEDE32);
}

void E32Parser::PostConstructL()
{
    iHdr = (E32ImageHeader*)iBufferedFile;
    isCompessed = iHdr->iCompressionType;
    size_t pos = sizeof(E32ImageHeader);

    if( *(uint32_t*)(iHdr->iSignature) != 0x434f5045) // 'EPOC'
        ReportError(ErrorCodes::ZEROBUFFER, "Signature for E32 image not found!\n");

/// TODO (Administrator#1#09/09/18): Stop here detection for pre-8 binaries
    iHdrJ = (E32ImageHeaderJ*)(iBufferedFile + pos);

    DecompressImage();
/// TODO (Administrator#1#09/11/18): Stop here detection for pre-9 binaries

    pos += sizeof(E32ImageHeaderJ);
    iHdrV = (E32ImageHeaderV*)(iBufferedFile + pos);

    ParseExportBitMap();
}

void E32Parser::ConstructL(const std::vector<char>& e32File)
{
    iE32Size = e32File.size();
    iBufferedFile = new char[iE32Size]();
    std::copy(e32File.begin(), e32File.end(), iBufferedFile);
}

void E32Parser::DecompressImage()
{
    if(!IsCompressed())
        return;

    const uint32_t offset = iHdr->iCodeOffset;
    const uint32_t extracted = iHdrJ->iUncompressedSize;
    size_t e32Size = Adjust(extracted + offset);

    if(e32Size != (extracted + offset))
        ReportError(ErrorCodes::WRONGFILESIZEFORDECOMPRESSION,
            extracted + offset, e32Size);

    const uint32_t compr = iHdr->iCompressionType;
    if((compr != KUidCompressionDeflate) && (compr != KUidCompressionBytePair))
        ReportError(ErrorCodes::UNKNOWNCOMPRESSION);

// allocate slightly more memory to minimize memory overrun
    char* uncompressed = new char[extracted + offset]();
    memcpy(uncompressed, iBufferedFile, offset);

    if(compr == KUidCompressionBytePair)
    {
        uint32_t uncompressedCodeSize = DecompressBPE(iBufferedFile + offset, uncompressed + offset);
        uint32_t uncompressedDataSize = DecompressBPE(nullptr, uncompressed + offset + uncompressedCodeSize);
        if((uncompressedCodeSize + uncompressedDataSize) != iHdrJ->iUncompressedSize)
            ReportWarning(ErrorCodes::BYTEPAIRINCONSISTENTSIZE);
    }else if(compr == KUidCompressionDeflate)
    {
        DeCompressInflate((unsigned char*)iBufferedFile + offset, iE32Size - offset, (unsigned char*)uncompressed + offset, extracted);
    }else
        ReportError(ErrorCodes::UNKNOWNCOMPRESSION);

    iE32Size = e32Size;
    delete[] iBufferedFile;
    iBufferedFile = nullptr;
    iBufferedFile = uncompressed;

    iHdr = (E32ImageHeader*)iBufferedFile;
    iHdrJ = (E32ImageHeaderJ*)(iBufferedFile + sizeof(E32ImageHeader));
}

const TExceptionDescriptor* E32Parser::GetExceptionDescriptor() const
{
    uint32_t xd = iHdrV->iExceptionDescriptor;
    xd &= ~1;
    return (TExceptionDescriptor *)(iBufferedFile + iHdr->iCodeOffset + xd);
}

const E32ImageHeader* E32Parser::GetE32Hdr() const
{
    if(!iHdr)
        ReportError(ZEROBUFFER, "Buffer in E32Parser. Call first E32ImageHeader* GetFileLayout()");
    return iHdr;
}

const E32ImageHeaderJ* E32Parser::GetE32HdrJ() const
{
    return iHdrJ;
}

const E32ImageHeaderV* E32Parser::GetE32HdrV() const
{
    return iHdrV;
}

const char* E32Parser::GetBufferedImage() const
{
    return iBufferedFile;
}


uint32_t HdrFmtFromFlags(uint32_t aFlags)
{
    if (aFlags&KImageHdrFmtMask)
        return aFlags & KImageHdrFmtMask;
    if (aFlags&KImageOldJFlag)
        return KImageHdrFmt_J;
    return KImageHdrFmt_Original;
}

uint32_t ImpFmtFromFlags(uint32_t aFlags)
{
    return aFlags & KImageImpFmtMask;
}

int32_t E32Parser::UncompressedFileSize() const
{
    if(iHdr->iCompressionType==0)
        return -1;          // not compressed
    else
        return iHdrJ->iUncompressedSize + iHdr->iCodeOffset;
}

size_t RoundUp(size_t value, size_t rounding)
{
    size_t res = value%rounding;
    res = rounding - res;
    return res + value;
}

size_t E32Parser::GetFileSize() const
{
    return iE32Size;
}

const E32RelocSection* E32Parser::GetRelocSection(uint32_t offSet) const
{
    return (E32RelocSection*)(iBufferedFile + offSet);
}

const E32ImportSection* E32Parser::GetImportSection() const
{
    return (E32ImportSection*)(iBufferedFile + iHdr->iImportOffset);
}

const uint32_t* E32Parser::GetImportAddressTable() const
{
    return (uint32_t*)(iBufferedFile + iHdr->iCodeOffset + iHdr->iTextSize);
}

const char* E32Parser::GetImportTable() const
{
    return (iBufferedFile + iHdr->iCodeOffset);
}

const char* E32Parser::GetDLLName(uint32_t OffsetOfDllName) const
{
    return (iBufferedFile + iHdr->iImportOffset + OffsetOfDllName);
}

//! iExportDirOffset points after Export Table header
uint32_t* E32Parser::GetExportTable() const
{
    return (uint32_t*)(iBufferedFile + iHdr->iExportDirOffset - sizeof(uint32_t));
}

uint32_t E32Parser::ExpSymInfoTableOffset() const
{
    return iHdr->iExportDirOffset +
        iHdr->iExportDirCount * sizeof(uint32_t);
}

const E32EpocExpSymInfoHdr* E32Parser::GetEpocExpSymInfoHdr() const
{
    uint32_t* tbl = GetExportTable();
// We ignore that formula because my build elf2e32 set wrong header for Export Table.
// elf2e32 shipped with SDK crashes from internal error.
//    return (E32EpocExpSymInfoHdr*)(tbl + tbl[0] + 1);
    return (E32EpocExpSymInfoHdr*)(tbl + iHdr->iExportDirCount + 1);
}

uint32_t E32Parser::BSSOffset() const
{
    return iHdr->iCodeSize + iHdr->iDataSize;
}

/**
This function creates the export description after reading the E32 image file
*/
int32_t E32Parser::GetExportDescription() const
{
    uint32_t fm = HdrFmtFromFlags(iHdr->iFlags);
    if (fm < KImageHdrFmt_V && iMissingExports)
        return KErrCorrupt;

    if(!iHdrV)
        ReportError(ErrorCodes::ZEROBUFFER, "E32ImageHeaderV* member not set!\n");

    if (iHdrV->iExportDescType == KImageHdr_ExpD_NoHoles)
        return iMissingExports ? KErrCorrupt : KErrNone;

    int32_t nexp = iHdr->iExportDirCount;
    int32_t memsz = (nexp + 7) >> 3;	// size of complete bitmap
    int32_t mbs = (memsz + 7) >> 3;	// size of meta-bitmap
    int32_t eds = iHdrV->iExportDescSize;

    if (iHdrV->iExportDescType == KImageHdr_ExpD_FullBitmap)
    {
        if (eds != memsz)
            return KErrCorrupt;
        if (memcmp(iHdrV->iExportDesc, iExportBitMap, eds) == 0)
            return KErrNone;
        return KErrCorrupt;
    }

    if (iHdrV->iExportDescType != KImageHdr_ExpD_SparseBitmap8)
        return KErrNotSupported;

    int32_t nbytes = 0;
    for (int32_t i=0; i<memsz; ++i)
        if (iExportBitMap[i] != 0xff)
            ++nbytes;				// number of groups of 8

    int32_t exp_extra = mbs + nbytes;
    if (eds != exp_extra)
        return KErrCorrupt;

    const uint8_t* mptr = iHdrV->iExportDesc;
    const uint8_t* gptr = mptr + mbs;
    for (int32_t i=0; i<memsz; ++i)
    {
        uint32_t mbit = mptr[i>>3] & (1u << (i&7));
        if (iExportBitMap[i] != 0xff)
        {
            if (!mbit || *gptr++ != iExportBitMap[i])
                return KErrCorrupt;
        }
        else if (mbit)
            return KErrCorrupt;
    }

    return KErrNone;
}

void E32Parser::ParseExportBitMap()
{
    int32_t nexp = iHdr->iExportDirCount;
    int32_t memsz = (nexp + 7) >> 3;
    iExportBitMap = new uint8_t[memsz];
    memset(iExportBitMap, 0xff, memsz);
    uint32_t* exports = (uint32_t*)(iBufferedFile + iHdr->iExportDirOffset);
    uint32_t hdrfmt = HdrFmtFromFlags(iHdr->iFlags);

    uint32_t entryPoint = EntryPoint();

    iMissingExports = 0;

    for (int32_t i=0; i<nexp; i++)
    {
        if (exports[i] == entryPoint)
        {
            iExportBitMap[i>>3] &= ~(1u << (i & 7));
            ++iMissingExports;
        }
    }

    if (hdrfmt < KImageHdrFmt_V && iMissingExports)
        ReportError(BADEXPORTS);
}

//! Calculate entry point and value for all absent symbols st_value also.
// That is all absent symbols st_value share same value.
uint32_t E32Parser::EntryPoint() const
{
    uint32_t impfmt = ImpFmtFromFlags(iHdr->iFlags);
    uint32_t entryPointOffset = (impfmt == KImageImpFmt_ELF) ? iHdr->iCodeBase : 0;
    return(iHdr->iEntryPoint + entryPointOffset);
}

bool E32Parser::IsCompressed() const
{
    return isCompessed != KFormatNotCompressed;
}

int32_t Adjust(int32_t size)
{
    return ((size+0x3)&0xfffffffc);
}
