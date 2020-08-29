// Copyright (c) 2020 Strizhniou Fiodar
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
/// FIXME (Administrator#5#08/27/20): broken compression and remove E32Rebuilder usage.

#ifndef E32FILE_H
#define E32FILE_H

#include <list>
#include <vector>
#include "common.hpp"

struct Args;
class ElfParser;
struct E32ImageHeader;
class RelocsProcessor;
class ExportBitmapSection;
typedef std::vector<char> E32SectionUnit;

/// Sections for E32Image chunks in sorted order
enum class E32Sections: uint16_t
{
    EMPTY_SECTION = 0,
    HEADER  = 1,
    BITMAP  = 2,
    CODE    = 3,
    EXPORTS = 4,
    SYMLOOK = 5,
    DATA    = 6,
    IMPORTS = 7,
    CODERELOCKS = 8,
    DATARELOCKS = 9
};

struct E32Section
{
    E32Sections type = E32Sections::EMPTY_SECTION;
    std::string info;
    E32SectionUnit section;
};

typedef std::list<E32Section> E32image;

class E32File
{
    public:
        E32File(const Args* args, const ElfParser* elfParser, const Symbols& s);
        ~E32File();
        void WriteE32File();
    private:
        void PrepareData();
        void SetFixedAddress(E32ImageHeader* hdr);
        void Compress(uint32_t compression);
    private:
        const Args* iE32Opts = nullptr;
        const ElfParser* iElfSrc = nullptr;
        const Symbols& iSymbols;
    private:
        uint16_t iExportDescSize = 0;
        uint8_t  iExportDescType = 0;
        E32image iE32image;
        E32SectionUnit iHeader;
        ExportBitmapSection* iExportBitmap;
        RelocsProcessor* iRelocs = nullptr;
        std::vector<int32_t> iImportTabLocations;
};

#endif // E32FILE_H
