// Copyright (c) 2018-2024 Strizhniou Fiodar
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
//

#ifndef E32PARSER_H
#define E32PARSER_H

#include <ios>
#include <vector>
#include <cstdint>

struct E32ImageHeader;
struct E32ImageHeaderJ;
struct E32ImageHeaderV;
struct E32RelocSection;
struct E32ImportSection;
struct E32EpocExpSymInfoHdr;
struct TExceptionDescriptor;

class E32Parser
{
        void ConstructL();
        E32Parser(const std::string& arg);

        void ConstructL(const std::vector<char>& e32File);
        E32Parser() = default;
        void PostConstructL();
    public:
        static E32Parser* NewL(const std::string& arg);
        static E32Parser* NewL(const std::vector<char>& e32File);
        ~E32Parser();

        const E32ImageHeader* GetE32Hdr() const;
        const E32ImageHeaderJ* GetE32HdrJ() const;
        const E32ImageHeaderV* GetE32HdrV() const;

        uint32_t BSSOffset() const;
        const TExceptionDescriptor* GetExceptionDescriptor() const;
        const E32ImportSection* GetImportSection() const;

        uint32_t* GetExportTable() const;
        int32_t GetExportDescription() const;

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
        bool IsCompressed() const;
    private:
        void ParseExportBitMap();
        void DecompressImage();

    private:
        std::streamsize iE32Size = 0;
        uint32_t isCompessed = 0;
        char* iBufferedFile = nullptr;

    private:
        const std::string iE32File;
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
