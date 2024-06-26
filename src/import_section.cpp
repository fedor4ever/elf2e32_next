// Copyright (c) 2020-2024 Strizhniou Fiodar
// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Use some code from original elf2e32
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

#include <string>
#include <vector>

#include "elfdefs.h"
#include "e32parser.h"
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

//! Set up the string table and record offsets into string table of each used
void ImportsSection::AllocStringTable()
{
    auto imps = iRelocs->GetImports();
    for (auto p: imps)
    {
        iStrTabOffsets.push_back(iStrTab.size());
        Relocations& imports = p.second;
        iStrTab += imports[0].iLinkAs;
        iStrTab.push_back(0);
    }
    while(iStrTab.size()%4)
        iStrTab.push_back(0);
}

string ImportsSection::FindDSO(const string& name)
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
		for(const auto& aReloc: imports)
        {
            const char* aSymName = iElf->GetSymbolNameFromStringTable(aReloc.iSymNdx);
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
			aImportSection.push_back(0); // Update made in UpdateImportTable()
		}
		idx++;
    }

    if((importSectionSize != aImportSection.size() * sizeof(Elf32_Word)) && !iOpts->iForceE32Build)
        ReportError(ErrorCodes::IMPORTSECTION, importSectionSize, aImportSection.size() * sizeof(Elf32_Word));

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

/** \brief Add empty entry which shall be updated with iImportTabLocations
 *
 * Fomula:
 *    offset = E32ImageHeader.iExportDirOffset + E32ImageHeader.iExportDirCount * sizeof(uint32_t) +
 *          (E32EpocExpSymInfoHdr.iDepDllZeroOrdTableOffset - E32ImageHeader.iCodeOffset)
 *    for(auto x: iImportTabLocations)
 *    {
 *        aImportTab[x] = offSet;
 *        offSet += sizeof(uint32_t);
 *    }
 */
void UpdateImportTable(const E32SectionUnit& s, const std::vector<int32_t>& iImportTabLocations, bool iSNamedlookup)
{
    if(!iSNamedlookup)
        return;
    E32Parser* p = E32Parser::NewL(s);
    const E32ImageHeader* h = p->GetE32Hdr();
    const E32EpocExpSymInfoHdr* sInf = p->GetEpocExpSymInfoHdr();
    size_t offSet = p->ExpSymInfoTableOffset();
    offSet += sInf->iDepDllZeroOrdTableOffset; // This points to the ordinal zero offset table now
    offSet -= h->iCodeOffset; // Starts from code section

    uint32_t* aImportTab = (uint32_t*)p->GetImportSection();
    for(auto x: iImportTabLocations)
    {
        aImportTab[x] = offSet;
        offSet += sizeof(uint32_t);
    }
    s.assign(p->GetBufferedImage(), p->GetBufferedImage() + p->GetFileSize());
    delete p;
}
