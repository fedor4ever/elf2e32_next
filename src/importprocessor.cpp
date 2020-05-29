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
#include "importprocessor.h"
#include "relocsprocessor.h"

using std::string;
using std::vector;

#define ELF_ENTRY_PTR(ptype, base, offset) \
	((ptype*)((char*)base + offset))

template <class T>
inline T Align(T v)
{
	unsigned int inc = sizeof(uint32_t)-1;
	unsigned int res = ((uint32_t)v+inc) & ~inc;
	return (T)res;
}

ImportProcessor::ImportProcessor(const ElfParser* elf, const RelocsProcessor* r,
            const Args* opts): iElf(elf), iRelocs(r), iOpts(opts)
{
    //ctor
}

ImportProcessor::~ImportProcessor()
{
    //dtor
}

void ImportProcessor::AllocStringTable()
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
}

bool IsFileExist(std::string& s)
{
    return access(s.c_str(), 0) == 0;
}

string ImportProcessor::FindDSO(std::string name)
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

E32Section ImportProcessor::Imports()
{
    AllocStringTable();
// Imports stored as uint32_t, therefore
// we calculate count uint32_t cells without vla parts(E32ImportBlock::iImports[])
    size_t importSectionSize = sizeof(/*E32ImportSection*/ int32_t)/4 +
        ( (sizeof(E32ImportBlock) - sizeof(int32_t))/4 * iRelocs->DllCount() +
        iRelocs->ImportsCount() );

// These are the 0th ordinals imported into the import table, one
// entry for each DLL.
    if(iOpts->iNamedlookup)
		importSectionSize += iRelocs->DllCount();

//    vector<Elf32_Word> aImportSection;
    vector<Elf32_Word> aImportSection(importSectionSize);
// This is the 'E32ImportSection' header - fill with 0 for the moment
	aImportSection.push_back(0);

    while(iStrTab.size()%4)
        iStrTab.push_back(0);
	size_t totalSize = Align(importSectionSize + iStrTab.size());

	// Fill in the section header now we have the correct value.
	aImportSection[0] = totalSize;
    E32Section imports;
    imports.type = E32Sections::IMPORTS;
    imports.info = "IMPORTS";
    imports.section.insert(imports.section.begin(), (char*)&aImportSection.at(0),
               (char*)&aImportSection.at(0) + importSectionSize * sizeof(uint32_t));
    imports.section.insert(imports.section.end(), iStrTab.data(),
                           iStrTab.data() + iStrTab.size());
    return imports;
}

vector<int32_t> ImportProcessor::ImportTabLocations()
{
    return iImportTabLocations;
}
