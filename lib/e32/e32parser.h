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
// Parse E32 Images for the elf2e32 tool
// For decompression purpose clients should use that code
// before creation E32Parser instance:
//    if( ((E32ImageHeader*)(iFile))->iCompressionType)
//    {
//        uint32_t extracted = ((E32ImageHeader*)(iFile))->iCodeOffset;
//        extracted += ((E32ImageHeaderJ*)(iFile + sizeof(E32ImageHeader) ))->iUncompressedSize;
//        const char* newfile = new char[extracted]();
//        memcpy(newfile, iFile, iFileSize);
//        delete[] iFile;
//        iFile = nullptr;
//        iFile = newfile;
//        iFileSize = extracted;
//    }
//
//

#ifndef E32PARSER_H
#define E32PARSER_H

#include <ios>
#include <cstdint>

struct E32ImageHeader;
struct E32ImageHeaderJ;
struct E32ImageHeaderV;
struct E32RelocSection;
struct E32ImportSection;
struct TExceptionDescriptor;
struct E32EpocExpSymInfoHdr;

class E32Parser
{
    public:
        E32Parser(const char* fileBuf,
                  const std::streamoff& bufsize);
        ~E32Parser() {}

        const E32ImageHeader* GetFileLayout();

        const E32ImageHeader* GetE32Hdr() const;
        const E32ImageHeaderJ* GetE32HdrJ() const;
        const E32ImageHeaderV* GetE32HdrV() const;

        uint32_t BSSOffset() const;
        const TExceptionDescriptor* GetExceptionDescriptor() const;
        const E32ImportSection* GetImportSection() const;

        uint32_t* GetExportTable() const;
        int32_t GetExportDescription();

        const uint32_t* GetImportAddressTable() const;
        uint32_t ExpSymInfoTableOffset() const;
        const E32EpocExpSymInfoHdr* GetEpocExpSymInfoHdr() const;
        const char* GetImportTable() const;

        const char* GetDLLName(uint32_t OffsetOfDllName) const;

        const char* GetBufferedImage() const;
        int32_t UncompressedFileSize() const;
        size_t GetFileSize() const;
        const E32RelocSection* GetRelocSection(uint32_t offSet) const;

        uint32_t EntryPoint() const;
    private:
        void ParseExportBitMap();
        void DecompressImage();

    private:
        const char* iBufferedFile = nullptr;
        std::streamoff iE32Size = 0;

    private:
        const E32ImageHeader* iHdr = nullptr;
        const E32ImageHeaderJ* iHdrJ = nullptr;
        const E32ImageHeaderV* iHdrV = nullptr;

        //used in ParseExportBitMap()
        uint8_t* iExportBitMap = nullptr;
        size_t iMissingExports = 0;
};

uint32_t HdrFmtFromFlags(uint32_t flags);
uint32_t ImpFmtFromFlags(uint32_t flags);
size_t RoundUp(size_t value, size_t rounding = 4);

#endif // E32PARSER_H
