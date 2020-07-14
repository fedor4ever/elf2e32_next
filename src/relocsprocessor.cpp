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

#include <string.h>
#include <algorithm>

#include "symbol.h"
#include "elfdefs.h"
#include "e32common.h"
#include "elfparser.h"
#include "relocsprocessor.h"
#include "symbolprocessor.h"
#include "e32importsprocessor.hpp"

using std::string;

const uint32_t E32RelocSectionStatic = sizeof(uint32_t) * 2; //sizeof(E32RelocSection) w/out vla

#define ELF_ENTRY_PTR(ptype, base, offset) \
	((ptype*)((char*)base + offset))

template <class T>
inline T Align(T v)
{
	unsigned int inc = sizeof(uint32_t)-1;
	unsigned int res = ((uint32_t)v+inc) & ~inc;
	return (T)res;
}

bool Cmp(LocalReloc x, LocalReloc y)
{
	return x.iRela.r_offset < y.iRela.r_offset;
}

void RelocsProcessor::SortRelocs()
{
    std::sort(iCodeRelocations.begin(), iCodeRelocations.end(), Cmp);
    std::sort(iDataRelocations.begin(), iDataRelocations.end(), Cmp);
}

RelocsProcessor::RelocsProcessor(const ElfParser* elf, const Symbols& s): iElf(elf), iRelocSrc(s) {}

void RelocsProcessor::Process()
{
    ProcessVerInfo();
    iVersionTbl = iElf->VersionTbl();
    const std::vector<RelocBlock> r = iElf->GetRelocs();
    for(auto x: r)
    {
        if(x.rel)
            ProcessRelocations(x.rel, x);
        else
            ProcessRelocations(x.rela, x);
    }
    RelocsFromSymbols(); //CreateExportTable()
    ProcessSymbolInfo(); //ProcessSymbolInfo()
    ProcessVeneers();
    SortRelocs();
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

size_t RelocationsSize(const std::vector<LocalReloc>& relocs)
{
    size_t bytecount = 0;
    int page = -1;
    for(auto x: relocs)
    {
        int p = x.iRela.r_offset & 0xfffff000;
        if (page != p)
        {
            if (bytecount%4 != 0)
                bytecount += sizeof(uint16_t);
            bytecount += E32RelocSectionStatic; // page, block size
            page = p;
        }
        bytecount += sizeof(uint16_t);
    }
    if (bytecount%4 != 0)
        bytecount += sizeof(uint16_t);
    return bytecount;
}

void RelocsProcessor::RelocsFromSymbols()
{
    Elf32_Addr* aPlace = iElf->ExportTable();
    uint32_t i = 1;
    for(auto x: iRelocSrc)
    {
        iExportTableAddress = (uintptr_t)aPlace;
        /// TODO (Administrator#1#06/16/20): use to set reloc
        AddToLocalRelocations(iExportTableAddress, i, R_ARM_ABS32,
                              x->GetElf32_Sym(), x->Absent());
        aPlace++;
        i++;
    }
}

void RelocsProcessor::ProcessSymbolInfo()
{
    Elf32_Addr elfAddr = iExportTableAddress - 4;// This location points to 0th ord.;
    AddToLocalRelocations(elfAddr, 0, R_ARM_ABS32, nullptr);

/// TODO (Administrator#1#06/25/20): find how os linker handle syminfo section. ...
///Is commented code wrong?
    elfAddr += ((iRelocSrc.size() + 1) * sizeof(uint32_t));// now points to the symInfo
//	uint32 *aZerothOrd = iUseCase->GetExportTable();
//	aZerothOrd[0] = elfAddr;
	elfAddr += sizeof(E32EpocExpSymInfoHdr);// now points to the symbol address
											// which is just after the syminfo header.
    for(auto x: iRelocSrc)
    {
        if(x->Absent())
            continue;
        AddToLocalRelocations(elfAddr, 0, R_ARM_ABS32, x->GetElf32_Sym());
        elfAddr += sizeof(uint32_t);
    }
}

void RelocsProcessor::ProcessVeneers()
{
    int length = strlen("$Ven$AT$L$$");
    Elf32_Sym* symTab = iElf->SymTab();
    const char* strTab = iElf->StrTab();
    if (!(symTab && strTab))
        return;
    // Process the symbol table to find Long ARM to Thumb Veneers
    // i.e. symbols of the form '$Ven$AT$L$$'
    for(; symTab < iElf->Lim(); symTab++)
    {
        if (!symTab->st_name) continue;
        char* aSymName = strTab + symTab->st_name;
        Elf32_Sym* aSym;

        if (!strncmp(aSymName, "$Ven$AT$L$$", length))
        {
            aSym = symTab;
            Elf32_Addr r_offset = aSym->st_value;
            const Elf32_Addr aOffset = r_offset + 4;
            Elf32_Word	aInstruction = iElf->GetRelocationPlace(r_offset);
            bool aRelocEntryFound = false;

            for(auto x: iCodeRelocations)
            {
                // Check if there is a relocation entry for the veneer symbol
                if (x.iRela.r_offset == aOffset)
                {
                    aRelocEntryFound = true;
                    break;
                }
            }
            Elf32_Word aPointer = iElf->GetRelocationPlace(aOffset);

            /* If the symbol addresses a Thumb instruction, its value is the
             * address of the instruction with bit zero set (in a
             * relocatable object, the section offset with bit zero set).
             * This allows a linker to distinguish ARM and Thumb code symbols
             * without having to refer to the map. An ARM symbol will always have
             * an even value, while a Thumb symbol will always have an odd value.
             * Reference: Section 4.5.3 in Elf for the ARM Architecture Doc
             * aIsThumbSymbol will be 1 for a thumb symbol and 0 for ARM symbol
             */
            int aIsThumbSymbol = aPointer & 0x1;

            /* The relocation entry should be generated for the veneer only if
             * the following three conditions are satisfied:
             * 1) Check if the instruction at the symbol is as expected
             *    i.e. has the bit pattern 0xe51ff004 == 'LDR pc,[pc,#-4]'
             * 2) There is no relocation entry generated for the veneer symbol
             * 3) The instruction in the location provided by the pointer is a thumb symbol
             */
            if (aInstruction == 0xE51FF004 && !aRelocEntryFound && aIsThumbSymbol)
                AddToLocalRelocations(aOffset, 0, R_ARM_NONE, aSym, false, true);
        }
    }
}


/**
This function creates Code and Data relocations from the corresponding
ELF form to E32 form.
*/
E32Section CreateRelocations(std::vector<LocalReloc>& aRelocations, E32Section& aRelocs, RelocsProcessor* rp)
{
	size_t rsize = RelocationsSize(aRelocations);
	if(!rsize)
        return E32Section();

    size_t aRelocsSize = Align(rsize + E32RelocSectionStatic);
    aRelocs.section.insert(aRelocs.section.begin(), aRelocsSize, 0);

    E32RelocSection* section = (E32RelocSection*)&aRelocs.section[0];
    section->iSize = rsize;
    section->iNumberOfRelocs = aRelocations.size();

    E32RelocBlock* block = section->iRelocBlock;
    // data should point to iEntry field, set later.
    uint16_t* data = (uint16_t*)section->iRelocBlock;

    const uint32_t aBase = (*aRelocations.begin()).iSegment->p_vaddr;
    int page = -1;
    int pagesize = sizeof(E32RelocSectionStatic);
    for (auto r: aRelocations)
    {
        int p = r.iRela.r_offset & 0xfffff000;
        if (page != p)
        {            if(pagesize%4 != 0)
            {
                // at first run does nothing
                *data++ = 0;
                pagesize += sizeof(uint16_t);
            }
            if (page == -1) page = p;
            block->iPageOffset = page - aBase;
            block->iBlockSize = pagesize;

            pagesize = E32RelocSectionStatic;
            page = p;
            block = (E32RelocBlock*)data;
            data = block->iEntry; // now data points to iEntry field
        }
        uint16_t relocType = rp->Fixup(r.iSymbol);
        *data++ = (uint16_t)((r.iRela.r_offset & 0xfff) | relocType);
        pagesize += sizeof(uint16_t);
    }
    if (pagesize%4 != 0)
    {
        *data++ = 0;
        pagesize += sizeof(uint16_t);
    }
    block->iPageOffset = page - aBase;
    block->iBlockSize = pagesize;
    return aRelocs;
}

bool IsLocalReloc(const LocalReloc& rel)
{
    return ((rel.iHasRela && !rel.iVeneerSymbol) &&
            (rel.iRelType == R_ARM_ABS32 || rel.iRelType == R_ARM_GLOB_DAT));
}

uint16_t RelocsProcessor::Fixup(const Elf32_Sym* s)
{
	if(s)
		return iElf->Segment(s);
    return KTextRelocType;
}

void RelocsProcessor::ApplyLocalReloc(const LocalReloc& rel)
{

	if(IsLocalReloc(rel))
	{
        Elf32_Word* aLoc = iElf->GetRelocationPlace(rel.iRela.r_offset);
        aLoc[0] += rel.iSymbol->st_value;
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
            Elf32_Rela tmp;
            tmp.r_offset = elfRel->r_offset;
            tmp.r_info = elfRel->r_info;
            tmp.r_addend = iElf->Addend(elfRel);
			if(IsImportedSymbol(aSymIdx, iElf))
			{
			    iImportsCount++;
                AddToImports(aSymIdx, tmp);
			}
			else
                AddToLocalRelocations(aSymIdx, aType, tmp);
		}
		elfRel++;
	}
}

void RelocsProcessor::AddToImports(uint32_t index, Elf32_Rela rela)
{
    uint32_t i = iVersionTbl[index];
    string linkAs = iVerInfo[i].iLinkAs;
    string SOname = iVerInfo[i].iSOName;
    iImports[linkAs].insert(iImports[linkAs].end(),
           ElfImportRelocation {
                iSymNdx : index,
                iRela : rela,
                iLinkAs : {linkAs},
                iSOName : {SOname}
                } );
}

void RelocsProcessor::AddToLocalRelocations(uint32_t index,
                     uint8_t relType, Elf32_Rela rela)
{
    LocalReloc loc;
    loc.iSegment = iElf->GetSegmentAtAddr(rela.r_offset);
    loc.iSymNdx = index;
    loc.iRela = Elf32_Rela(rela);
    loc.iHasRela = true;
    loc.iRelType = relType;
    loc.iSymbol = iElf->GetSymbolTableEntity(index);
    loc.iSegmentType = iElf->SegmentType(rela.r_offset);
    UpdateRelocs(loc);
}

//Elf32_Word aAddend = Addend(aElfRel);
//tmp.r_addend = iElf->Addend(elfRel);
//aAddr = tmp.r_offset
void RelocsProcessor::AddToLocalRelocations(uint32_t aAddr, uint32_t index,
            uint8_t relType, Elf32_Sym* aSym, bool aDelSym, bool veneerSymbol)
{
    LocalReloc loc;
    uint16_t type = Fixup(aSym);
    loc.iSegment = iElf->Segment(&type);
    loc.iSymNdx = index;
    loc.iRela.r_offset = aAddr;
    loc.iHasRela = false;
    loc.iSymbol = aSym;
    loc.iRelType = relType;
    loc.iDelSym = aDelSym; // true for absent symbols only
    loc.iVeneerSymbol = veneerSymbol;
    loc.iSegmentType = ESegmentType::ESegmentRO;
    UpdateRelocs(loc);
}

void RelocsProcessor::UpdateRelocs(const LocalReloc& r)
{
    ApplyLocalReloc(r);
    switch(r.iSegmentType)
    {
    case ESegmentType::ESegmentRO:
        iCodeRelocations.push_back(r);
        break;
    case ESegmentType::ESegmentRW:
        iDataRelocations.push_back(r);
        break;
    default:
        break;
    }
}

void RelocsProcessor::ProcessVerInfo()
{
	uint32_t aSz = iElf->VerInfoCount() + 1;
	iVerInfo.insert(iVerInfo.begin(), aSz, VersionInfo());

	const char*  aSoName = nullptr;
	const char*  aLinkAs = nullptr;

	Elf32_Verdef* aDef = iElf->GetElf32_Verdef();
	while(aDef)
    {
		Elf32_Verdaux* daux = ELF_ENTRY_PTR( Elf32_Verdaux, aDef, aDef->vd_aux);
		aLinkAs = iElf->GetNameFromStringTable(daux->vda_name);
		aSoName = iElf->SOName();
		iVerInfo[aDef->vd_ndx].iLinkAs = aLinkAs;
		iVerInfo[aDef->vd_ndx].iSOName = aSoName;

		if( !aDef->vd_next )
			break;
		aDef = ELF_ENTRY_PTR(Elf32_Verdef, aDef, aDef->vd_next);
	}

	Elf32_Verneed *aNeed = iElf->GetElf32_Verneed();
	while(aNeed)
    {
		Elf32_Vernaux* aNaux = ELF_ENTRY_PTR(Elf32_Vernaux, aNeed, aNeed->vn_aux);
		aLinkAs = iElf->GetNameFromStringTable(aNaux->vna_name);
		aSoName = iElf->GetNameFromStringTable(aNeed->vn_file);

		iVerInfo[aNaux->vna_other].iLinkAs = aLinkAs;
		iVerInfo[aNaux->vna_other].iSOName = aSoName;

        iLinkAsNames.push_back(aLinkAs);
        iDllCount++;
		if(!aNeed->vn_next)
			break;
		aNeed = ELF_ENTRY_PTR(Elf32_Verneed, aNeed, aNeed->vn_next);
	}
	std::sort(iLinkAsNames.begin(), iLinkAsNames.end());
}

ImportLibs RelocsProcessor::GetImports()
{
    return iImports;
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

E32Section RelocsProcessor::CodeRelocsSection()
{
    E32Section codeRel;
    codeRel.type = E32Sections::CODERELOCKS;
    codeRel.info = "CODERELOCKS";
    return CreateRelocations(iCodeRelocations, codeRel, this);
}

E32Section RelocsProcessor::DataRelocsSection()
{
    E32Section dataRel;
    dataRel.type = E32Sections::DATARELOCKS;
    dataRel.info = "DATARELOCKS";
    return CreateRelocations(iDataRelocations, dataRel, this);
}
