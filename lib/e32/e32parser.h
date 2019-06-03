// Copyright (c) 2018-2019 Strizhniou Fiodar
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
// @internalComponent
// @released
//
//

#ifndef E32PARSER_H
#define E32PARSER_H

#include <cstdint>

struct E32ImageHeader;
struct E32ImageHeaderJ;
struct E32ImageHeaderV;
struct E32RelocSection;
struct E32ImportSection;

class E32Parser
{
    public:
        E32Parser(const char* fileBuf,
                  const std::streamoff& bufsize, bool compressed = true);
        ~E32Parser() {}

        const E32ImageHeader *GetFileLayout();
        const E32ImageHeaderJ *GetE32HdrJ() const;
        const E32ImageHeaderV *GetE32HdrV() const;

        uint32_t BSSOffset() const;
        const TExceptionDescriptor *GetExceptionDescriptor() const;
        const E32ImportSection *GetImportSection() const;
        const char *GetImportAddressTable() const;

        const E32EpocExpSymInfoHdr *GetEpocExpSymInfoHdr() const;

        const char *GetDLLName(uint32_t OffsetOfDllName) const;

        const char *GetBufferedImage() const;
        int32_t UncompressedFileSize() const;
        size_t GetFileSize() const;
        int32_t GetExportDescription();
        const E32RelocSection *GetRelocSection(uint32_t offSet);
    private:
        void ParseExportBitMap();
        void DecompressImage();

    private:
        const char *iBufferedFile = nullptr;
        const std::streamoff iE32Size = 0;
        // for example bulded from scratch E32 Image is uncompressed and may have compression flag
        bool iAlreadyUncompressed = true;

    private:
        const E32ImageHeader *iHdr = nullptr;
        const E32ImageHeaderJ *iHdrJ = nullptr;
        const E32ImageHeaderV *iHdrV = nullptr;

        //used in ParseExportBitMap()
        uint8_t *iExportBitMap = nullptr;
        size_t iMissingExports = 0;
};

uint32_t HdrFmtFromFlags(uint32_t flags);
uint32_t ImpFmtFromFlags(uint32_t flags);
size_t RoundUp(size_t value, size_t rounding = 4);

#endif // E32PARSER_H
