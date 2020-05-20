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

/// TODO (Administrator#1#05/20/20): integrate code: ...
///
///	// At the end, the dependencies are listed. They remain zeroes and shall be fixed up
///	// while relocating.
///
///	// Update the import table to have offsets to ordinal zero entries
///	uint32 *importTab = iImportSection;
///
///	uint32 offset = aBaseOffset - iHdr->iCodeOffset;// This gives the offset of syminfo table base
///										// wrt the code section start
///	offset += expHdr.iDepDllZeroOrdTableOffset; // This points to the ordinal zero offset table now
///	for(auto x: iImportTabLocations) {
///		uint32 *aLocation = (importTab + x);
///		*aLocation = offset;
///		offset += sizeof(uint32);
///	}
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
