// Copyright (c) 2020-2024 Strizhniou Fiodar
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
// Create E32Image.
//
//


#include <string.h>
#include <cinttypes>

#include "symbol.h"
#include "e32file.h"
#include "e32info.h"
#include "argparser.h"
#include "elfparser.h"
#include "e32common.h"
#include "e32parser.h"
#include "e32rebuilder.h"
#include "e32validator.h"
#include "e32compressor.h"
#include "elf2e32_opt.hpp"
#include "import_section.h"
#include "relocsprocessor.h"
#include "e32header_section.h"
#include "exportbitmap_section.h"
#include "symbollookup_section.h"

E32Section CodeSection(const ElfParser* parser);
E32Section DataSection(const ElfParser* parser);
void PrintSymlookHdr(const E32Section& s);

bool CmpSections(const E32Section& first, const E32Section& second)
{
    return first.type < second.type;
}

E32File::E32File(const Args* args, const ElfParser* elfParser, const Symbols& s):
    iE32Opts(args), iElfSrc(elfParser), iSymbols(s){}

E32File::~E32File()
{
    delete iRelocs;
}

void E32File::SetFixedAddress(E32ImageHeader* hdr)
{
    if(iE32Opts->iFixedaddress)
    {
        if(iElfSrc->ImageIsDll())
            ReportLog("Warning: Cannot set fixed address for DLL.\n");
        else
            hdr->iFlags|=KImageFixedAddressExe;
    }
    else
        hdr->iFlags&=~KImageFixedAddressExe;
}

void E32File::WriteE32File()
{
// old behaviour for NDi targets
//    Symbols tmp;
//    for(auto x: iSymbols)
//    {
//        if(!x->Absent())
//            tmp.push_back(x);
//    }
//    iSymbols = tmp;

    iRelocs = new RelocsProcessor(iElfSrc, iSymbols, iE32Opts->iNamedlookup);
    iRelocs->Process();

    E32HeaderSection header(iE32Opts);
    iHeader = header.MakeE32Header();

    E32ImageHeader* hdr = (E32ImageHeader*)&iHeader[0];
    hdr->iDataSize = iElfSrc->DataSegmentSize();
    hdr->iBssSize = iElfSrc->BssSegmentSize();
    hdr->iEntryPoint = iElfSrc->EntryPointOffset();
    hdr->iCodeBase = iElfSrc->ROVirtualAddress();
    hdr->iDataBase = iElfSrc->RWVirtualAddress();
    hdr->iDllRefTableCount = iRelocs->DllCount();   // filling this in enables E32ROM to leave space for it
    hdr->iExportDirCount = iSymbols.size();
    hdr->iCompressionType = 0;

    if((hdr->iFlags & KImageDll) && iE32Opts->iNoDlldata)
    {
        if(hdr->iDataSize)
            ReportError(ErrorCodes::ZEROBUFFER, "ELF File: " + iE32Opts->iDso + " contains initialized writable data.");
        if(hdr->iBssSize)
            ReportError(ErrorCodes::ZEROBUFFER, "ELF File: " + iE32Opts->iDso + " contains uninitialized writable data.");
    }

    SetFixedAddress(hdr);

    if(iElfSrc->ImageIsDll())
        hdr->iFlags |= KImageDll;

    const uint32_t offset = sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ);
    E32ImageHeaderV* hdrv = (E32ImageHeaderV*)&iHeader[offset];

    hdrv->iExceptionDescriptor = iElfSrc->ExceptionDescriptor();

    PrepareData();

    for(auto x: iE32image)
    {
// we set this field outside switch because Symbian Post Linker, Elf2E32 V2.0
// set this field for exes as KImageHdr_ExpD_FullBitmap.
        hdrv->iExportDescType = this->iExportDescType;
        switch(x.type)
        {
        case E32Sections::HEADER:
            break;
        case E32Sections::BITMAP:
            hdrv->iExportDescSize = this->iExportDescSize;
            hdr->iCodeOffset = iHeader.size() + x.section.size();
            break;
        case E32Sections::EXPORTS: //falltru
            hdr->iExportDirOffset = iHeader.size() + sizeof(uint32_t); // point directly to exports
        case E32Sections::SYMLOOK: //falltru
        case E32Sections::CODE: //falltru
            hdr->iTextSize = hdr->iCodeSize = iHeader.size() + x.section.size() - hdr->iCodeOffset;
            break;
        case E32Sections::DATA:
            hdr->iDataOffset = iHeader.size();
            break;
        case E32Sections::IMPORTS:
            hdr->iImportOffset = iHeader.size();
            break;
        case E32Sections::CODERELOCKS:
            hdr->iCodeRelocOffset = iHeader.size();
            break;
        case E32Sections::DATARELOCKS:
            hdr->iDataRelocOffset = iHeader.size();
            break;
        default:
            ReportError(ErrorCodes::UNKNOWNSECTION);
            break;
        }

        if(VerboseOut()) {
            printf("Added Chunks has size: %06" PRIxMAX " for section:"
               " %s at address: %08" PRIxMAX "\n",
               (uintmax_t)x.section.size(), x.info.c_str(), (uintmax_t)iHeader.size());
        }

        iHeader.insert(iHeader.end(), x.section.begin(), x.section.end());
        hdr = (E32ImageHeader*)&iHeader[0];
        hdrv = (E32ImageHeaderV*)&iHeader[offset];
    }
    UpdateImportTable(iHeader, iImportTabLocations, iE32Opts->iNamedlookup);
    E32ImageHeaderJ* j = (E32ImageHeaderJ*)&iHeader[sizeof(E32ImageHeader)];
    j->iUncompressedSize = iHeader.size() - hdr->iCodeOffset;

//    Compress(iE32Opts->iCompressionMethod);
    hdr = (E32ImageHeader*)&iHeader[0];
    hdr->iCompressionType = iE32Opts->iCompressionMethod;

    SetE32ImageCrc(iHeader.data());

// Compression doesn't work so we use E32Rebuilder to compress and save to file.
//    E32SectionUnit s(iHeader);
//    E32Parser* p = new E32Parser(s.data(), s.size());
//    p->GetFileLayout();
//    ValidateE32Image(p);

    Args arg;
    arg.iCompressionMethod = iE32Opts->iCompressionMethod;
    arg.iOutput = iE32Opts->iOutput;
    arg.iForceE32Build = iE32Opts->iForceE32Build;
    arg.iFileCrc = iE32Opts->iFileCrc;
    arg.iElfinput = iE32Opts->iElfinput;

    //call E32Info::HeaderInfo() for verbose output
    if(VerboseOut() && !DisableLongVerbosePrint())
    {
        hdr->iCompressionType = KFormatNotCompressed;

        E32Info* info = new E32Info(iE32Opts, iHeader);
        info->HeaderInfo();
        delete info;

        hdr->iCompressionType = iE32Opts->iCompressionMethod;
    }

    E32Rebuilder* rb = new E32Rebuilder(&arg);
    rb->Compress(iHeader);
    delete rb;
    iHeader.clear();
//    SaveFile(iE32Opts->iOutput.c_str(), iHeader.data(), iHeader.size());
}

/// Return count of exported functions or 0th ordinal
//	RLibrary library;
//	E32EpocExpSymInfoHdr *readHdr;
//	test(library.Load(_L("t_dll1.dll")) == KErrNone);
//	readHdr = (E32EpocExpSymInfoHdr*)library.Lookup(0); !!!0th ordinal
//	test(readHdr == NULL);
uint32_t InitExportHeader(uint32_t symbols_num, bool symlook, Elf32_Addr* aPlace)
{
    uint32_t x = symbols_num;
// !!!pain!!! =(
// original code for symlook table:
//    iTable[0] = ((uintptr_t)iElf->ExportTable()) - 4 + (symbols_num + 1) * sizeof(char*)
// where sizeof(char*) == 4 on msvc and sizeof(char*) == 8 on mingw
// so expand variables assuming msvc sizeof(char*) == 4
// because SDK elfe32 builded when sizeof(char*) == 4
//    iTable[0] = ((uintptr_t)iElf->ExportTable()) + symbols_num * 4
    if(symlook)
        x = ((uintptr_t)aPlace) + symbols_num * 4;
    return x;
}

// Export Section consist of uint32_t array, first element is section's header.
// Absent symbols values set E32 image entry point, other set to their elf st_value.
// If --namedlookup: export table header points to E32EpocExpSymInfoHdr, otherwise number of exports
E32Section ExportSection(const Symbols& s, bool symlook, Elf32_Addr* aPlace)
{
    E32Section exports;
    if(s.empty())
        return exports;

    exports.info = "EXPORTS";
    exports.type = E32Sections::EXPORTS;
    // The export table has a header containing the number of entries
	// before the entries themselves. So add 1 to number of exports
    uint32_t sz = (s.size() + 1) * sizeof(uint32_t);
    exports.section.insert(exports.section.begin(), sz, 0);

    uint32_t* iTable = (uint32_t*)exports.section.data();
    iTable[0] = InitExportHeader(s.size(), symlook, aPlace);

    uint32_t i = 1;
    for(auto x: s)
        iTable[i++] = x->Elf_st_value();
    return exports;
}

E32Section CodeSection(const ElfParser* parser)
{
    E32Section code;
    code.info = "CODE";
    code.type = E32Sections::CODE;
    code.section.insert(code.section.begin(), parser->CodeSegment(), parser->CodeSegment() + parser->CodeSegmentSize());
    return code;
}

E32Section DataSection(const ElfParser* parser)
{
    E32Section data;
    if(parser->DataSegmentSize() == 0)
        return data;
    data.info = "DATA";
    data.type = E32Sections::DATA;
    data.section.insert(data.section.begin(), parser->DataSegment(), parser->DataSegment() + parser->DataSegmentSize());
    return data;
}

bool IsEXE(TargetType type)
{
    if( (type == TargetType::EExe) || (type == TargetType::EExexp) ||
        (type == TargetType::EStdExe) )
        return true;
    return false;
}

void E32File::PrepareData()
{
    E32Section tmp;
    tmp = ExportSection(iSymbols, iE32Opts->iNamedlookup, iElfSrc->ExportTable());

    if(IsEXE(iE32Opts->iTargettype))
        iExportDescType = KImageHdr_ExpD_FullBitmap;

    if(tmp.type > E32Sections::EMPTY_SECTION)
    {
        iE32image.push_back(tmp);

        ExportBitmapSection* proc =
            new ExportBitmapSection(iSymbols.size(), tmp.section, iElfSrc->EntryPoint());
        tmp = proc->CreateExportBitmap();
        if(tmp.type > E32Sections::EMPTY_SECTION)
        {
            iHeader.pop_back(); // remove E32ImageHeaderV::iExportDesc[1]
            iE32image.push_back(tmp);
            iExportDescSize = proc->ExportDescSize();
            iExportDescType = proc->ExportDescType();
        }
        delete proc;
    }

// While import section builds their relocs implicitly apply
// for code and data sections. Therefore should run first
    ImportsSection* proc = new ImportsSection(iElfSrc, iRelocs, iE32Opts);
    tmp = proc->Imports();
    iImportTabLocations = proc->ImportTabLocations();
    iE32image.push_back(tmp);
    delete proc;

    tmp = CodeSection(iElfSrc);
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    if(iE32Opts->iNamedlookup)
    {
        uint32_t r = iRelocs->DllCount();
        SymbolLookupSection* look = new SymbolLookupSection(iSymbols, r);
        tmp = look->SymlookSection();
        if(tmp.type == E32Sections::EMPTY_SECTION)
            ReportError(ErrorCodes::BADEXPORTS);
//        PrintSymlookHdr(tmp);
        iE32image.push_back(tmp);
        delete look;
    }

    tmp = DataSection(iElfSrc);
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    tmp = iRelocs->CodeRelocsSection();
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    tmp = iRelocs->DataRelocsSection();
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);
    iE32image.sort([](auto A, auto B){return A.type < B.type;});
}

void BuildE32Image(const Args* args, const ElfParser* elfParser, const Symbols& s)
{
    E32File file(args, elfParser, s);
    file.WriteE32File();
}

void E32File::Compress(uint32_t compression)
{
    if(!compression)
        return;
    if(compression == KUidCompressionDeflate)
        iHeader = CompressDeflate(iHeader);
    else if(compression == KUidCompressionBytePair)
        iHeader = CompressBPE(iHeader);
    else
        ReportError(INVALIDARGUMENT, "--compressionmethod", std::to_string(compression));
}

void PrintSymlookHdr(const E32Section& s)
{
    if(s.type != E32Sections::SYMLOOK)
        ReportError(ErrorCodes::ZEROBUFFER, "Expected symlook section. But got different one!\n");
    E32EpocExpSymInfoHdr* h = (E32EpocExpSymInfoHdr*)&s.section[0];
    ReportLog("E32EpocExpSymInfoHdr field values:\n");
    ReportLog("iSize: 0x%x\n", h->iSize);
    ReportLog("iFlags: 0x%x\n", h->iFlags);
    ReportLog("iSymCount: 0x%x\n", h->iSymCount);
    ReportLog("iSymbolTblOffset: 0x%x\n", h->iSymbolTblOffset);
    ReportLog("iStringTableSz: 0x%x\n", h->iStringTableSz);
    ReportLog("iStringTableOffset: 0x%x\n", h->iStringTableOffset);
    ReportLog("iDllCount: 0x%x\n", h->iDllCount);
    ReportLog("iDepDllZeroOrdTableOffset: 0x%x\n", h->iDepDllZeroOrdTableOffset);
}
