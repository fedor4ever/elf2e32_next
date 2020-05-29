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
// Parse ELF relocs
//
//

#include <algorithm>

#include "string.h"
#include "elfdefs.h"
#include "elfparser.h"
#include "relocsprocessor.h"
#include "symbolprocessor.h"

using std::string;

#define ELF_ENTRY_PTR(ptype, base, offset) \
	((ptype*)((char*)base + offset))

bool StringPtrLess::operator() (const char * lhs, const char * rhs) const
{
	return strcmp(lhs, rhs) < 0;
}

RelocsProcessor::RelocsProcessor(const ElfParser* elf): iElf(elf) {}

void RelocsProcessor::Process()
{
    ProcessVerInfo();
    iVersionTbl = iElf->VersionTbl();
    std::vector<RelocBlock> r = iElf->GetRelocs();
    for(auto x: r)
    {
        if(x.rel)
            ProcessRelocations(x.rel, x);
        else
            ProcessRelocations(x.rela, x);
    }
}

bool ValidRelocEntry(uint8_t aType)
{
    switch(aType)
    {
    case R_ARM_ABS32:
    case R_ARM_GLOB_DAT:
    case R_ARM_JUMP_SLOT:
    case R_ARM_RELATIVE:
    case R_ARM_GOT_BREL:
        return true;
    case R_ARM_NONE:
        return false;
    default:
        return false;
    }
}

template <class T>
void RelocsProcessor::ProcessRelocations(const T* elfRel, const RelocBlock& r)
{
	if(!elfRel)
		return;

	T* elfRelLimit = ELF_ENTRY_PTR(T, elfRel, r.size);
	while(elfRel < elfRelLimit)
    {
		uint8_t aType = ELF32_R_TYPE(elfRel->r_info );
		if(ValidRelocEntry(aType))
        {
			uint32_t aSymIdx = ELF32_R_SYM(elfRel->r_info);
			bool isImported = IsImportedSymbol(aSymIdx, iElf);
            Elf32_Rela tmp;
            tmp.r_offset = elfRel->r_offset;
            tmp.r_info = elfRel->r_info;
            tmp.r_addend = iElf->Addend(elfRel);
			if(isImported)
			{
			    iImportsCount++;
//                AddToImports(aSymIdx, tmp);
			}
			else
            {
//				 ElfRelocation* aRel = new ElfLocalRelocation(this, elfRel->r_offset, aAddend,
//						aSymIdx, aType, &tmp);
//				 AddToLocalRelocations((ElfLocalRelocation*)aRel);
			}
		}
		elfRel++;
	}
}

void RelocsProcessor::AddToImports(uint32_t index, Elf32_Rela rela)
{
    string linkAs = iVerInfo[ iVersionTbl[index]].iLinkAs;
    string SOname = iVerInfo[ iVersionTbl[index]].iLinkAs;
}

void RelocsProcessor::ProcessVerInfo()
{
	uint32_t aSz = iElf->VerInfoCount() + 1;
	std::vector<VersionInfo> iVerInfo(aSz);
//	iVerInfo.reserve(aSz);

	Elf32_Verdef* aDef = iElf->GetElf32_Verdef();
	const char*  aSoName = nullptr;
	const char*  aLinkAs = nullptr;

//	while(aDef)
//    {
//		Elf32_Verdaux* daux = ELF_ENTRY_PTR( Elf32_Verdaux, aDef, aDef->vd_aux);
//		aLinkAs = iElf->GetNameFromStringTable(daux->vda_name);
//		aSoName = iElf->SOName();
//		iVerInfo[aDef->vd_ndx].iLinkAs = aLinkAs;
//		iVerInfo[aDef->vd_ndx].iSOName = aSoName;
//		printf("aLinkAs: %s\n", aLinkAs);
//		printf("aSoName: %s\n", aSoName);
//
//		if( !aDef->vd_next )
//			break;
//		aDef = ELF_ENTRY_PTR(Elf32_Verdef, aDef, aDef->vd_next);
//	}

	Elf32_Verneed *aNeed = iElf->GetElf32_Verneed();

	while(aNeed)
    {
		Elf32_Vernaux* aNaux = ELF_ENTRY_PTR(Elf32_Vernaux, aNeed, aNeed->vn_aux);
		aLinkAs = iElf->GetNameFromStringTable(aNaux->vna_name);
		aSoName = iElf->GetNameFromStringTable(aNeed->vn_file);

//		iVerInfo[aNaux->vna_other].iLinkAs = aLinkAs;
                        printf("aLinkAs: %s\n", aLinkAs);
//		iVerInfo[aNaux->vna_other].iSOName = aSoName;
                        printf("aSoName: %s\n", aSoName);

        iLinkAsNames.push_back(aLinkAs);
        iDllCount++;
		if(!aNeed->vn_next)
			break;
		aNeed = ELF_ENTRY_PTR(Elf32_Verneed, aNeed, aNeed->vn_next);
	}
	std::sort(iLinkAsNames.begin(), iLinkAsNames.end());
}

uint32_t RelocsProcessor::ImportsCount() const
{
    return iImportsCount;
}

uint32_t RelocsProcessor::DllCount() const
{
    return iDllCount;
}

std::vector<std::string> RelocsProcessor::StrTableData() const
{
    return iLinkAsNames;
}
