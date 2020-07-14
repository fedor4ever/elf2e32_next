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
// Parse ELF Images
//
//

#include <assert.h>
#include <string.h>

#include "elfdefs.h"
#include "common.hpp"
#include "e32common.h"
#include "elfparser.h"

using std::string;
using std::vector;

#define ELF_ENTRY_PTR(ptype, base, offset) \
	((ptype*)((char*)base + offset))

Elf32_Word SegmentSize(const Elf32_Phdr* h)
{
    if(h)return h->p_filesz;
    return 0;
}

Elf32_Word VirtualAddress(const Elf32_Phdr* p)
{
	if(p) return p->p_vaddr;
	return 0;
}

bool GlobalSymbol(Elf32_Sym* aSym)
{
	return (ELF32_ST_BIND(aSym->st_info) == STB_GLOBAL);
}

ElfParser::ElfParser(string elf): iFile(elf) {}

ElfParser::~ElfParser()
{
    delete[] iFileBuf;
}

const char* ElfParser::CodeSegment() const
{
    return iCodeSegment;
}

const char* ElfParser::DataSegment() const
{
    return iDataSegment;
}

uint32_t* ElfParser::GetDSOImportsOrdinals() const
{
    return (uint32_t*)iCodeSegment;
}

void ElfParser::GetElfFileLayout()
{
    iFileBuf = ReadFile(iFile.c_str(), iFileBufSize);
    iElfHeader = ELF_ENTRY_PTR(Elf32_Ehdr, iFileBuf, 0);

    ValidateElfImage();

    /* Get the program header..*/
	if(iElfHeader->e_phnum)
		iProgHeader = ELF_ENTRY_PTR(Elf32_Phdr, iElfHeader, iElfHeader->e_phoff);
    /* Get the Section base..*/
	if(iElfHeader->e_shnum)
		iSections = ELF_ENTRY_PTR(Elf32_Shdr, iElfHeader, iElfHeader->e_shoff);

    /* Get the section-header-string table..*/
	if(iElfHeader->e_shstrndx != SHN_UNDEF)
    {
		if(iElfHeader->e_shstrndx > iElfHeader->e_shnum)
			ReportError(ErrorCodes::ELFSHSTRINDEXERROR, iFile);

		iSectionHdrStrTbl = ELF_ENTRY_PTR(char, iElfHeader, iSections[iElfHeader->e_shstrndx].sh_offset);
	}
	ProcessSectionHeaders();
	ProcessProgHeaders();
	ProcessDynamicTable();
}

void ElfParser::ProcessProgHeaders()
{
    if(!iProgHeader)
        return;

    uint32_t idx = 0;
    while( idx < iElfHeader->e_phnum)
    {
        switch( iProgHeader[idx].p_type )
        {
        case PT_DYNAMIC:
            iDynSegmentHdr = &iProgHeader[idx];
            break;
        case PT_LOAD:
            {
                if( (iProgHeader[idx].p_flags) & (PF_X | PF_ARM_ENTRY) ) {
                    iCodeSegmentHdr = &iProgHeader[idx];
                    iCodeSegmentIdx = idx;
                    iCodeSegment = ELF_ENTRY_PTR(char, iElfHeader, iCodeSegmentHdr->p_offset);
                }
                else if( (iProgHeader[idx].p_flags) & (PF_W | PF_R) ) {
                    iDataSegmentHdr = &iProgHeader[idx];
                    iDataSegmentIdx = idx;
                    iDataSegment = ELF_ENTRY_PTR(char, iElfHeader, iDataSegmentHdr->p_offset);
                }
            }
            break;
        default:
            break;
        }
        idx++;
    }
}

uint32_t ElfParser::BssSegmentSize() const
{
	if (iDataSegmentHdr)
		return iDataSegmentHdr->p_memsz - iDataSegmentHdr->p_filesz;
	return 0;
}

uint32_t ElfParser::CodeSegmentSize() const
{
    return SegmentSize(iCodeSegmentHdr);
}

uint32_t ElfParser::DataSegmentSize() const
{
    return SegmentSize(iDataSegmentHdr);
}

uint32_t ElfParser::ROVirtualAddress() const
{
    return VirtualAddress(iCodeSegmentHdr);
}

uint32_t ElfParser::RWVirtualAddress() const
{
    return VirtualAddress(iDataSegmentHdr);
}

uint32_t ElfParser::EntryPointOffset() const
{
    uint32_t x = EntryPoint();
    if(!x)
        return x;
    return x - iCodeSegmentHdr->p_vaddr;
}

uint32_t ElfParser::EntryPoint() const
{
	if (!(iElfHeader->e_entry) && !(iCodeSegmentHdr->p_vaddr))
	{
	    ReportWarning(ErrorCodes::UNDEFINEDENTRYPOINT);
		return 0;
	}
	else if (!(iElfHeader->e_entry))
        ReportError(ErrorCodes::ENTRYPOINTNOTSET);
    return iElfHeader->e_entry;
}

void ElfParser::ProcessSectionHeaders()
{
    // Find the static symbol table and string table
    for(uint32_t i = 0; i < iElfHeader->e_shnum; i++)
    {
        if (iSections[i].sh_type == SHT_SYMTAB)
        {
            iSymTab = ELF_ENTRY_PTR(Elf32_Sym, iElfHeader, iSections[i].sh_offset);
            iLim = ELF_ENTRY_PTR(Elf32_Sym, iSymTab, iSections[i].sh_size);
            if (iStrTab) break;
        }
        else if (iSections[i].sh_type == SHT_STRTAB)
        {
            char* secName = iSectionHdrStrTbl + iSections[i].sh_name;
            if (!strcmp(secName, ".strtab"))
            {
                iStrTab = ELF_ENTRY_PTR(char, iElfHeader, iSections[i].sh_offset);
                if (iSymTab) break;
            }
        }
        else if(iSections[i].sh_type == SHT_PROGBITS)
        {
            const char commentSection[] = ".comment";
            size_t length = strlen(commentSection);
            char* aSectionName = iSectionHdrStrTbl + iSections[i].sh_name;

            if(!strncmp(aSectionName, commentSection, length))
                iCommentSection = ELF_ENTRY_PTR(char, iElfHeader, iSections[i].sh_offset);
        }
    }
}

Elf32_Word ElfParser::Addend(const Elf32_Rel* r) const
{
	Elf32_Phdr* h = GetSegmentAtAddr(r->r_offset);
	uint32_t off = h->p_offset + r->r_offset - h->p_vaddr;
	Elf32_Word* aAddendPlace = ELF_ENTRY_PTR(Elf32_Word, iElfHeader, off);
	return *aAddendPlace;
}

uint32_t ElfParser::Addend(const Elf32_Rela* r) const
{
    return r->r_addend;
}


vector<RelocBlock> ElfParser::GetRelocs()
{
    vector<RelocBlock> r;
    if(iRel.rel)
        r.push_back(iRel);
    if(iRela.rela)
        r.push_back(iRela);
    if(iPltRel.rel)
        r.push_back(iPltRel);
    if(iPltRela.rela)
        r.push_back(iPltRela);
    assert(!r.empty());
    return r;
}

void ElfParser::ProcessDynamicTable()
{
    bool aPltRelTypeSeen = false, aJmpRelSeen = false;
    uint32_t iJmpRelOffset = 0, iPltRelType = 0;

    uint32_t i = 0, nSymbols = 0;
    Elf32_Dyn* aDyn = ELF_ENTRY_PTR(Elf32_Dyn, iElfHeader, iDynSegmentHdr->p_offset);
    while( aDyn[i].d_tag != DT_NULL )
    {
        if(aDyn[i].d_val & DF_1_PIE)
            ReportError(ErrorCodes::ELFPIEERROR, iFile);

        switch (aDyn[i].d_tag)
        {
		case DT_ARM_SYMTABSZ:
			nSymbols = aDyn[i].d_val;
			break;
		case DT_HASH:
			iHashTbl = ELF_ENTRY_PTR(Elf32_HashTable, iElfHeader, aDyn[i].d_val);
			break;
		case DT_STRTAB:
			iStringTable = ELF_ENTRY_PTR(char, iElfHeader, aDyn[i].d_val);
			break;
		case DT_SYMTAB:
			iElfDynSym = ELF_ENTRY_PTR(Elf32_Sym, iElfHeader, aDyn[i].d_val);
			break;
		case DT_SONAME:
			iSONameOffset = aDyn[i].d_val;
			break;
		case DT_JMPREL:
			aJmpRelSeen = true;
			iJmpRelOffset = aDyn[i].d_val;
			break;
		case DT_REL:
            iRel.rel = ELF_ENTRY_PTR(Elf32_Rel, iElfHeader, aDyn[i].d_val);
			break;
		case DT_RELSZ:
			iRel.size = aDyn[i].d_val;
			break;
		case DT_RELA:
			iRela.rela = ELF_ENTRY_PTR(Elf32_Rela, iElfHeader, aDyn[i].d_val);
			break;
		case DT_RELASZ:
			iRela.size = aDyn[i].d_val;
			break;
		case DT_VERSYM:
			iVersionTbl = ELF_ENTRY_PTR(Elf32_Half, iElfHeader, aDyn[i].d_val);
			break;
        case DT_VERDEF:
            iVersionDef = ELF_ENTRY_PTR(Elf32_Verdef, iElfHeader, aDyn[i].d_val);
            break;
		case DT_VERDEFNUM:
            iVerInfoCount += aDyn[i].d_val;
			break;
		case DT_VERNEED:
			iVersionNeed = ELF_ENTRY_PTR(Elf32_Verneed, iElfHeader, aDyn[i].d_val);
			break;
        case DT_VERNEEDNUM:
            iVerInfoCount += aDyn[i].d_val;
            break;
		default:
			//cout << "Unknown entry in dynamic table Tag=0x%x Value=0x%x",aDyn[i].d_tag, aDyn[i].d_val);
			break;
        }
        i++;
    }

    if(iHashTbl)
	{
		//The number of symbols should be same as the number of chains in hashtable
		if (nSymbols && (nSymbols != iHashTbl->nChains))
			ReportError(ErrorCodes::SYMBOLCOUNTMISMATCHERROR, iFile);
	}

    if(aPltRelTypeSeen && aJmpRelSeen)
    {
		if (iPltRelType == DT_REL)
		{
			iPltRel.rel = ELF_ENTRY_PTR(Elf32_Rel, iElfHeader, iJmpRelOffset);
			// check to see if PltRels are included in iRel. If they are
			// ignore them since we don't care about the distinction
			if (iRel.rel <= iPltRel.rel && iPltRel.rel < ELF_ENTRY_PTR(Elf32_Rel, iRel.rel, iRel.size))
				iPltRel.rel = nullptr;
		}
		else
		{
			iPltRela.rela = ELF_ENTRY_PTR(Elf32_Rela, iElfHeader, iJmpRelOffset);
			// check to see if PltRels are included in iRel.  If they are
			// ignore them since we don't care about the distinction
			if (iRela.rela <= iPltRela.rela && iPltRela.rela < ELF_ENTRY_PTR(Elf32_Rela, iRela.rela, iRela.size))
				iPltRela.rela = nullptr;
		}
	}
}

Elf32_Verdef* ElfParser::GetElf32_Verdef() const
{
    return iVersionDef;
}

Elf32_Half* ElfParser::VersionTbl() const
{
    return iVersionTbl;
}

const char* ElfParser::SOName() const
{
    return iStringTable + iSONameOffset;
}

uint32_t ElfParser::ImportsCount() const
{
    return iHashTbl->nChains;
}

const char* ElfParser::GetSymbolNameFromStringTable(uint32_t index) const
{
    return iStringTable + iElfDynSym[index].st_name;
}

const char* ElfParser::GetNameFromStringTable(uint32_t offset) const
{
    if(!iStringTable)
        return nullptr;
    return iStringTable + offset;
}

void ElfParser::ValidateElfImage()
{
    /* Is the ELF-Magic is correct*/
	if(!(iElfHeader->e_ident[EI_MAG0] == ELFMAG0) &&
		(iElfHeader->e_ident[EI_MAG1] == ELFMAG1) &&
		(iElfHeader->e_ident[EI_MAG2] == ELFMAG2) &&
		(iElfHeader->e_ident[EI_MAG3] == ELFMAG3) )
    {
        ReportError(ErrorCodes::ELFMAGICERROR, iFile);
	}

	/* 32-bit ELF file*/
	if(iElfHeader->e_ident[EI_CLASS] != ELFCLASS32)
		ReportError(ErrorCodes::ELFCLASSERROR, iFile);

	/* Is the ELF file in Little endian format*/
	if(iElfHeader->e_ident[EI_DATA] != ELFDATA2LSB)
		ReportError(ErrorCodes::ELFLEERROR, iFile);

	/* The ELF executable must be a DLL or an EXE*/
	if(iElfHeader->e_type != ET_EXEC && iElfHeader->e_type != ET_DYN)
		ReportError(ErrorCodes::ELFEXECUTABLEERROR, iFile);
}

// ARM linker prior version ARM linker 2.2 build 616 doesn't generate
// relocation entries for Long ARM to Thumb veneers.
// The linker problem is resolved in ARM Linker version RVCT 2.2 Build 616.
// This method check linker name and version.
bool ElfParser::IsRelocationFixRequired() const
{
    // no comment section - no fix
    if(!iCommentSection)
        return false;

    char const aARMCompiler[] = "ARM Linker, RVCT";
    const size_t length = strlen(aARMCompiler);
    if(strncmp(iCommentSection, aARMCompiler, length))
        return false;

    /* RVCTVersion contains the following string
     * "<MajorVersion>.<MinorVersion> [Build <BuildNumber>]"
     * Example: "2.2 [Build 616]"
     */
    string RVCTVersion(iCommentSection, length);
    size_t pos1 = RVCTVersion.find("2.2");
    size_t pos2 = RVCTVersion.find("616");
    if((pos1 == string::npos) || (pos2 == string::npos))
        return false;
    if(pos2 <= pos1)
        return false;

    return true;
}

ESegmentType ElfParser::SegmentType(Elf32_Addr addr) const
{
	Elf32_Phdr* phdr = GetSegmentAtAddr(addr);
	if(!phdr)
		return ESegmentUndefined;

	if(phdr == iCodeSegmentHdr)
		return ESegmentRO;
	else if(phdr == iDataSegmentHdr)
		return ESegmentRW;
	return ESegmentUndefined;
}

Elf32_Phdr* ElfParser::GetSegmentAtAddr(Elf32_Addr addr) const
{
    if(iCodeSegmentHdr)
    {
		uint32_t base = iCodeSegmentHdr->p_vaddr;
		if( (base <= addr) && (addr < (base + iCodeSegmentHdr->p_memsz)) )
			return iCodeSegmentHdr;
	}
	if(iDataSegmentHdr)
	{
		uint32_t base = iDataSegmentHdr->p_vaddr;
		if( (base <= addr) && (addr < (base + iDataSegmentHdr->p_memsz)) )
			return iDataSegmentHdr;
	}
    return nullptr;
}

Elf32_Sym* ElfParser::GetSymbolTableEntity(uint32_t index) const
{
    return &iElfDynSym[index];
}

Elf32_Word ElfParser::VerInfoCount() const
{
    return iVerInfoCount;
}

uint32_t ElfParser::ExceptionDescriptor() const
{
    const char aExDescName[] = "Symbian$$CPP$$Exception$$Descriptor";
    Elf32_Sym* sym = LookupStaticSymbol(aExDescName);
    if(!sym)
        ReportError(ErrorCodes::MISSEDEXCEPTIONDESCRIPTOR);

    uint32_t symAddr = sym->st_value;
    uint32_t codeSegmentAddr = ROVirtualAddress();
    uint32_t codeSegmentSize = CodeSegmentSize() + codeSegmentAddr;
    //check its in RO segment
    if((symAddr < codeSegmentAddr) || (symAddr >= codeSegmentSize))
        ReportError(ErrorCodes::OUTOFBOUNDSEXCEPTIONDESCRIPTOR);

    // Set bottom bit so 0 in header slot means an old binary.
    // The decriptor is always aligned on a 4 byte boundary.
    return((symAddr - codeSegmentAddr) | 0x00000001);
}

Elf32_Sym* ElfParser::SymTab() const
{
    return iSymTab;
}

Elf32_Sym* ElfParser::Lim() const
{
    return iLim;
}

const char* ElfParser::StrTab() const
{
    return iStrTab;
}

Elf32_Sym* ElfParser::LookupStaticSymbol(const char* aName)
{
	if (!iElfHeader->e_shnum)
        ReportError(ErrorCodes::NOSTATICSYMBOLS);

	if(!iSymTab || !iStrTab)
        ReportError(ErrorCodes::NOSTATICSYMBOLS);

    Elf32_Sym* aSymTab = iSymTab;
	while(aSymTab++ < iLim)
	{
		if (!aSymTab->st_name) continue;
		char* aSymName = iStrTab + aSymTab->st_name;
		if (!strcmp(aSymName, aName))
			return aSymTab;
	}
    return nullptr;
}

Elf32_Verneed* ElfParser::GetElf32_Verneed() const
{
    return iVersionNeed;
}

Elf32_Sym* ElfParser::FindSymbol(char* aName)
{
	if(!aName )
		return nullptr;

	uint32_t aHashVal = elf_hash((const unsigned char*)aName);

	Elf32_Sword* aBuckets = ELF_ENTRY_PTR(Elf32_Sword, iHashTbl, sizeof(Elf32_HashTable) );
	Elf32_Sword* aChains = ELF_ENTRY_PTR(Elf32_Sword, aBuckets, sizeof(Elf32_Sword)*(iHashTbl->nBuckets) );

	Elf32_Sword aIdx = aHashVal % iHashTbl->nBuckets;
	aIdx = aBuckets[aIdx];
	do
    {
		char *symName = ELF_ENTRY_PTR(char, iStringTable, iElfDynSym[aIdx].st_name);
		if( !strcmp(symName, aName) )
			return &iElfDynSym[aIdx];
		aIdx = aChains[aIdx];
	} while( aIdx > 0 );
	return nullptr;
}

uint32_t ElfParser::GetSymbolOrdinal(char* aSymName) const
{
    Elf32_Sym* s = FindSymbol(aSymName);
    if(!s)
        return (uint32_t)-1;
    return GetSymbolOrdinal(s);
}

uint32_t ElfParser::GetSymbolOrdinal(Elf32_Sym* aSym) const
{
    uint32_t ord = (uint32_t)-1;
    if( aSym->st_shndx == ESegmentRO)
    {
        Elf32_Word aOffset = iCodeSegmentHdr->p_offset + aSym->st_value - iCodeSegmentHdr->p_vaddr;
        Elf32_Word *aLocation = ELF_ENTRY_PTR(Elf32_Word, iElfHeader, aOffset);
        ord = *aLocation;
    }
    return ord;
}

/// return offset from section's start
Elf32_Word ElfParser::GetRelocationOffset(Elf32_Addr r_offset) const
{
    Elf32_Phdr* hdr = GetSegmentAtAddr(r_offset);
    return r_offset - hdr->p_vaddr;
}

Elf32_Word* ElfParser::GetRelocationPlace(Elf32_Addr r_offset) const
{
	Elf32_Phdr* hdr = GetSegmentAtAddr(r_offset);
	uint32_t off = hdr->p_offset + r_offset - hdr->p_vaddr;
	Elf32_Word* place = ELF_ENTRY_PTR(Elf32_Word, iElfHeader, off);
	return place;
}

uint16_t ElfParser::Segment(const Elf32_Sym* s) const
{
    if(!s)
        return KReservedRelocType;
    Elf32_Phdr* hdr = GetSegmentAtAddr(s->st_value);
    if (hdr == iCodeSegmentHdr) return KTextRelocType;
    else if (hdr == iDataSegmentHdr) return KDataRelocType;
}

Elf32_Addr* ElfParser::ExportTable()
{
    Elf32_Phdr* ROHdr = iCodeSegmentHdr;
    // The export table starts after the header. NB this is a virtual address in the RO
    // segment of the E32Image. It is outside the ELF RO segment.
    return ELF_ENTRY_PTR(Elf32_Addr, (intptr_t)ROHdr->p_vaddr, ROHdr->p_filesz) + 1;
}

Elf32_Phdr* ElfParser::Segment(uint16_t aType)
{
    switch(aType)
    {
    case KTextRelocType:
        return iCodeSegmentHdr;
    case KDataRelocType:
        return iDataSegmentHdr;
    default:
        return nullptr;
	}
}
