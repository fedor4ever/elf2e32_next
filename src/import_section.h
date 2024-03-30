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
// Create E32Image import section.
//
// Import section contains of string table and record offsets into string table of each used.
// Note: used imports only!
//
// There example of library which have two imported library but only one used.
// See at tests\t-client.dll
//
//

#ifndef IMPORTPROCESSOR_H
#define IMPORTPROCESSOR_H

#include <vector>
#include "e32file.h"
#include "e32importsprocessor.hpp"

struct Args;
class RelocsProcessor;

// import section:
//struct E32ImportSection
//{
//    int32_t iSize;     ///< Size of E32ImportBlock arrays
//    const E32ImportBlock iImportBlock[E32ImageHeader::iDllRefTableCount];
//};
//
//struct E32ImportBlock
//{
//    uint32_t Size(uint32_t aImpFmt) const;
//    uint32_t iOffsetOfDllName = 0;        ///< Offset from start of import section for a NULL terminated executable (DLL or EXE) name.
//    int32_t  iNumberOfImports = 0;        ///< Number of imports from this executable.
//    uint32_t iImports[iNumberOfImports];  ///< For ELF-derived executes: list of code section offsets. For PE, list of imported ordinals. Omitted in PE2 import format
//};
//
// stringtable at end

class ImportsSection
{
    public:
        ImportsSection(const ElfParser* elf, const RelocsProcessor* rel,
                        const Args* opts);
        ~ImportsSection();
        E32Section Imports();
        std::vector<int32_t> ImportTabLocations();
    private:
        void AllocStringTable();
        std::string FindDSO(const std::string& aName);
    private:
        const ElfParser* iElf = nullptr;
        const RelocsProcessor* iRelocs = nullptr;
        const Args* iOpts = nullptr;
        std::vector<int32_t> iStrTabOffsets;
        std::string iStrTab;
        std::vector<std::string> iDsoNames;
        std::vector<int32_t> iImportTabLocations;
        bool iNamedLookUp = false;
};

void UpdateImportTable(const E32SectionUnit& s, const std::vector<int32_t>& iImportTabLocations, bool iSNamedlookup);

#endif // IMPORTPROCESSOR_H
