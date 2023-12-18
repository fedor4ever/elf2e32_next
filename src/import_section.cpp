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
// Create E32Image import section.
//
//

#include <string>
#include <vector>
#include <assert.h>
#include <unistd.h>
#include <algorithm>
        #include <stdio.h>

#include "elfdefs.h"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "import_section.h"
#include "relocsprocessor.h"

using std::string;
using std::vector;

#define ELF_ENTRY_PTR(ptype, base, offset) \
	((ptype*)((char*)base + offset))

ImportsSection::ImportsSection(const ElfParser* elf, const RelocsProcessor* r,
            const Args* opts): iElf(elf), iRelocs(r), iOpts(opts)
{
    //ctor
}

ImportsSection::~ImportsSection()
{
    //dtor
}

void ImportsSection::AllocStringTable()
{
    auto imps = iRelocs->StrTableData();
// First set up the string table and record offsets into string table of each
// LinkAs name.
	for (auto x: imps)
	{
		iStrTabOffsets.push_back(iStrTab.size());
        iStrTab += x;
        iStrTab.push_back(0);
	}
    while(iStrTab.size()%4)
        iStrTab.push_back(0);
}

bool IsFileExist(std::string& s)
{
    return access(s.c_str(), 0) == 0;
}

string ImportsSection::FindDSO(std::string name)
{
	if(IsFileExist(name))
		return name;

	string paths = iOpts->iLibpath;
	string aDSOPath;
	size_t end = 0, start = 0;
	while(end != std::string::npos)
    {
        end = paths.find_first_of(";");
        aDSOPath = paths.substr(start, end);
        start = end;
        size_t s = aDSOPath.find_first_of("\\");
        if(s != std::string::npos)
            aDSOPath += "\\";
        else
            aDSOPath += "/";
        aDSOPath += name;

		if(IsFileExist(aDSOPath))
			return aDSOPath;
    }
	ReportError(ErrorCodes::FILEOPENERROR, aDSOPath);
	return string(); //silence gcc warning
}

E32Section ImportsSection::Imports()
{
    AllocStringTable();
// We calculate count bytes without vla parts(E32ImportBlock::iImports[])
    size_t importSectionSize = sizeof(/*E32ImportSection*/ int32_t) +
        ( (sizeof(E32ImportBlock) - sizeof(int32_t)) * iRelocs->DllCount() +
        iRelocs->ImportsCount() * sizeof(uint32_t));

// These are the 0th ordinals imported into the import table, one
// entry for each DLL.
    if(iOpts->iNamedlookup)
		importSectionSize += iRelocs->DllCount() * sizeof(uint32_t);

	size_t totalSize = importSectionSize + iStrTab.size();
    vector<Elf32_Word> aImportSection;
    aImportSection.push_back(totalSize); // E32ImportSection::iSize = totalSize

    ImportLibs imps = iRelocs->GetImports();
	int idx = 0;
	for (auto p: imps)
    {
        Relocations& imports = p.second;
        // E32ImportBlock::iOffsetOfDllName
        aImportSection.push_back(iStrTabOffsets[idx] + importSectionSize);
		// Take the additional 0th ordinal import into account
		size_t nImports = imports.size();
		if(iOpts->iNamedlookup) nImports++;
		aImportSection.push_back(nImports); // E32ImportBlock::iNumberOfImports

        string aDSO = FindDSO(imports[0].iSOName);
		ElfParser parser(aDSO);
		parser.GetElfFileLayout();
		for(auto aReloc: imports)
        {
            char* aSymName = (char*)iElf->GetSymbolNameFromStringTable(aReloc.iSymNdx);
            uint32_t aOrdinal = parser.GetSymbolOrdinal(aSymName);

//check the reloc refers to Code Segment
            Elf32_Addr r_offset = aReloc.iRela.r_offset;
            if (iElf->SegmentType(r_offset) != ESegmentType::ESegmentRO)
                ReportError(ErrorCodes::ILLEGALEXPORTFROMDATASEGMENT, aSymName, iOpts->iElfinput);
            Elf32_Word off = iElf->GetRelocationOffset(r_offset);
            aImportSection.push_back(off);

            Elf32_Word* aRelocPlace = iElf->GetRelocationPlace(r_offset);
            *aRelocPlace = (aReloc.iRela.r_addend<<16) | aOrdinal;
        }
        if(iOpts->iNamedlookup)
        {
            // Keep track of the location of the entry
			iImportTabLocations.push_back(aImportSection.size());
			// Put the entry as 0 now, which shall be updated
			aImportSection.push_back(0);
		}
		idx++;
    }

    if(importSectionSize != aImportSection.size() * sizeof(Elf32_Word))
        ReportError(ErrorCodes::IMPORTSECTION, importSectionSize, aImportSection.size());

    E32Section imports;
    imports.type = E32Sections::IMPORTS;
    imports.info = "IMPORTS";
    imports.section.insert(imports.section.begin(), (char*)&aImportSection.at(0),
               (char*)&aImportSection.at(0) + importSectionSize);
    imports.section.insert(imports.section.end(), iStrTab.data(),
                           iStrTab.data() + iStrTab.size());
    return imports;
}

vector<int32_t> ImportsSection::ImportTabLocations()
{
    return iImportTabLocations;
}
