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

#ifndef E32FILE_H
#define E32FILE_H

#include <list>
#include <vector>
#include "common.hpp"

struct Args;
class ElfParser;
struct E32ImageHeader;
class ExportBitmapProcessor;
typedef std::vector<char> E32SectionUnit;

/// Sections for E32Image chunks in sorted order
enum class E32Sections: uint16_t
{
    HEADER,
    BITMAP,
    CODE,
    EXPORTS,
    SYMLOOK,
    DATA,
    IMPORTS,
    CODERELOCKS,
    DATARELOCKS
};

struct E32Section
{
    E32Sections type;
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
    private:
        const Args* iE32Opts = nullptr;
        const ElfParser* iElfSrc = nullptr;
        const Symbols& iSymbols;
    private:
        uint16_t iExportDescSize = 0;
        uint8_t  iExportDescType = 0;
        E32image iE32image;
        E32SectionUnit iHeader;
        ExportBitmapProcessor* iExportBitmap;
};

#endif // E32FILE_H
