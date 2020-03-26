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

#include <string.h>

#include "elfdefs.h"
#include "common.hpp"
#include "elfparser.h"

using std::string;

#define ELF_ENTRY_PTR(ptype, base, offset) \
	((ptype*)((char*)base + offset))

ElfParser::ElfParser(string elf): iFile(elf) {}

ElfParser::~ElfParser()
{
    delete[] iFileBuf;
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

uint32_t ElfParser::CodeSegmentSize()
{
    return iCodeSegmentHdr->p_filesz;
}

uint32_t ElfParser::DataSegmentSize()
{
    return iDataSegmentHdr->p_filesz;
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

void ElfParser::ProcessDynamicTable()
{
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
		case DT_STRTAB:
			iStringTable = ELF_ENTRY_PTR(char, iElfHeader, aDyn[i].d_val);
			break;
		case DT_HASH:
			iHashTbl = ELF_ENTRY_PTR(Elf32_HashTable, iElfHeader, aDyn[i].d_val);
			break;
		case DT_SYMTAB:
			iElfDynSym = ELF_ENTRY_PTR(Elf32_Sym, iElfHeader, aDyn[i].d_val);
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
}

uint32_t ElfParser::ImportsCount() const
{
    return iHashTbl->nChains;
}

const char* ElfParser::GetSymbolNameFromStringTable(uint32_t index) const
{
    return iStringTable + iElfDynSym[index].st_name;
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
bool ElfParser::IsRelocationFixRequired()
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

ESegmentType ElfParser::SegmentType(Elf32_Addr aAddr) const
{
	Elf32_Phdr* phdr = GetSegmentAtAddr(aAddr);
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
		if( base <= addr && addr < (base + iCodeSegmentHdr->p_memsz) ) {
			return iCodeSegmentHdr;
		}
	}
	if(iDataSegmentHdr)
	{
		uint32_t base = iDataSegmentHdr->p_vaddr;
		if( base <= addr && addr < (base + iDataSegmentHdr->p_memsz) ) {
			return iDataSegmentHdr;
		}
	}
    return nullptr;
}


Elf32_Sym* ElfParser::GetSymbolTableEntity(uint32_t index) const
{
    return &iElfDynSym[index];
}













