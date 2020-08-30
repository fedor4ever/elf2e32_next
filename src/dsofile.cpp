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
// Create DSO aka import library from @Symbols
//
//

#include <stdio.h>
#include <cstring>
#include <fstream>

#include "symbol.h"
#include "elfdefs.h"
#include "dsofile.h"
#include "common.hpp"

using std::string;
using std::fstream;

Elf32_Ehdr* CreateElfHeader();
void AlignString(std::string& aStr);
void InfoPrint(const char* hdr, uint32_t& pos, const uint32_t offset);
void SetElfSymbols(Symbol* symbol, Elf32_Sym* elfSymbol, uint32_t index);

//enum for section index
enum SECTION_INDEX
{
    DUMMY_SECTION=0,
    CODE_SECTION,
    DYNAMIC_SECTION,
    HASH_TBL_SECTION,
    VER_DEF_SECTION,
    VERSION_SECTION,
    STRING_SECTION,
    SYMBOL_SECTION,
    SH_STR_SECTION,
    MAX_SECTIONS=SH_STR_SECTION
};

/**
 * Following array is indexed on the SECTION_INDEX enum
 */
const char* SECTION_NAME[] =
{
    "",
    "ER_RO",
    ".dynamic",
    ".hash",
    ".version_d",
    ".version",
    ".strtab",
    ".dynsym",
    ".shstrtab"
};

DSOFile::~DSOFile()
{
    delete iElfHeader;
    delete iSections;
    delete iElfDynSym;
    delete iVersionTbl;
    delete iVersionDef;
    delete iDSODaux;

    delete iProgHeader;
    delete iCodeSectionData;
    delete iHashBuf;
}

/**
This function creates that tables: symbol, code, hash, version.
*/
void DSOFile::CreateTablesFromSymbols(const Symbols& s)
{
    iDSOSymNameStrTbl.push_back(0);

    const char *absentSymbol = "_._.absent_export_";
    const int length = strlen(absentSymbol) + 7;
    uint32_t aPos = 0;
    for(auto x: s)
    {
        // Ordinal Number can be upto 0xffff which is 6 digits
        if(x->Ordinal() > 999999)
            ReportError(ErrorCodes::VALUEOVERFLOW, "absent symbols");
        // Update code section data..
        iCodeSectionData[aPos++] = x->Ordinal();

        //set symbol info..
        iElfDynSym[aPos].st_name = iDSOSymNameStrTbl.size();

        /* If a symbol is marked as Absent in the DEF file, replace the
         * symbol name with "_._.absent_export_<Ordinal Number>"
         */
        if(x->Absent())
        {
            char *symName = new char[length]();
            sprintf(symName, "_._.absent_export_%d", x->Ordinal());
            iDSOSymNameStrTbl += symName;
            delete[] symName;
        }
        else
            iDSOSymNameStrTbl += x->AliasName();
        iDSOSymNameStrTbl.push_back(0);

        SetElfSymbols(x, &iElfDynSym[aPos], aPos);

        //set version table info...
        iVersionTbl[aPos] = DEFAULT_VERSION;
        AddToHashTable(x->AliasName().c_str(), aPos);
    }
}

void DSOFile::WriteDSOFile(const string& dsoFile, const string& linkAs, const Symbols& s)
{
    iNSymbols = s.size() + 1;
    if(iNSymbols == 1)
        ReportError(ErrorCodes::EMPTYARGUMENT, "DSOFile::WriteDSOFile()", "--symbols");
    if(dsoFile.empty())
        ReportError(ErrorCodes::EMPTYARGUMENT, "DSOFile::WriteDSOFile()", "--filename");
    if(linkAs.empty())
        ReportError(ErrorCodes::EMPTYARGUMENT, "DSOFile::WriteDSOFile()", "--linkas");

    CreateSectionHeaders();
    CreateHashTable();
    CreateTablesFromSymbols(s);

    string DSOName(dsoFile);
    std::size_t found = DSOName.find_last_of("/\\");
    InitVersionTable(DSOName.substr(found+1), linkAs);

    //Fill section headers...
    InitProgramHeaderTable();

    //Copy dyn entries..
    InitDynamicEntries();

    //create code section data - this has the ordinal numbers...
    InitProgHeader();

    WriteElfContents(dsoFile.c_str());
}

void DSOFile::CreateSectionHeaders()
{
    iElfHeader        = ::CreateElfHeader();
    iSections        = new Elf32_Shdr[MAX_SECTIONS+1]();

    iElfDynSym        = new Elf32_Sym[iNSymbols]();
    iVersionTbl        = new Elf32_Half[iNSymbols]();
    iVersionDef        = new Elf32_Verdef[2]();
    iDSODaux        = new Elf32_Verdaux[2]();

    iProgHeader         = new Elf32_Phdr[2]();
    iCodeSectionData = new uint32_t[iNSymbols]();
}

void DSOFile::CreateHashTable()
{
    Elf32_Word sz = 2 + iNSymbols + iNSymbols /3 + iNSymbols%0x3;
    iHashBuf = new Elf32_Word[sz]();
    iHashTbl = (Elf32_HashTable*)iHashBuf;

    //premeditated
    iHashTbl->nBuckets = (iNSymbols/3) + (iNSymbols%0x3);
    iHashTbl->nChains = iNSymbols;

    iDSOBuckets = iHashBuf + sizeof(Elf32_HashTable)/sizeof(Elf32_Word);
    iDSOChains = iHashBuf + sizeof(Elf32_HashTable)/sizeof(Elf32_Word) + iHashTbl->nBuckets;
}

void DSOFile::InitVersionTable(const string& DSOName, const string& linkAs)
{
    //Fill verdef table...
    iVersionDef[0].vd_ndx = 1;
    iVersionDef[0].vd_cnt = 1;
    iVersionDef[0].vd_flags = 1;
    iVersionDef[0].vd_hash = elf_hash((const uint8_t*)DSOName.c_str());
    iVersionDef[0].vd_version = 1;

    iVersionDef[0].vd_aux = sizeof(Elf32_Verdef);
    iVersionDef[0].vd_next = sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux);

    iDSODaux[0].vda_name = iDSOSymNameStrTbl.size();
    iDSODaux[0].vda_next = 0;

    iDSOSymNameStrTbl += DSOName;
    iDSOSymNameStrTbl.push_back(0);

    iVersionDef[1].vd_ndx = DEFAULT_VERSION;
    iVersionDef[1].vd_cnt = 1;
    iVersionDef[1].vd_flags = 0;
    iVersionDef[1].vd_hash = elf_hash((const uint8_t*)linkAs.c_str());
    iVersionDef[1].vd_version = 1;

    iVersionDef[1].vd_aux = sizeof(Elf32_Verdef);
    iVersionDef[1].vd_next = 0;

    iDSODaux[1].vda_name = iDSOSymNameStrTbl.size();
    iDSODaux[1].vda_next = 0;

    iDSOSymNameStrTbl += linkAs;
    iDSOSymNameStrTbl.push_back(0);
}

void SetElfSymbols(Symbol *src, Elf32_Sym* dst, uint32_t aCodeIndex)
{
    dst->st_other = STV_DEFAULT;

    dst->st_info = (uint8_t) (ELF32_ST_INFO(STB_GLOBAL, src->CodeDataType()));
    dst->st_value    = (aCodeIndex - 1)*sizeof(Elf32_Word);

    if(src->CodeDataType() == SymbolTypeCode){
        dst->st_size = sizeof(Elf32_Word);
    }else{
        dst->st_size = src->SymbolSize();
    }
    dst->st_shndx = CODE_SECTION;
}

/**
This function adds an entry into the hash table based on the symbol name.
@param aSymName The Symbol name
@param aIndex The Symbol index in the Symbol table
*/
void DSOFile::AddToHashTable(const char* aSymName, uint32_t aIndex)
{
    Elf32_Word    hsh = elf_hash((uint8_t*)aSymName);
    Elf32_Word  aBIdx = hsh % iHashTbl->nBuckets;

    if(iDSOBuckets[aBIdx] == 0)
    {
        iDSOBuckets[aBIdx] = aIndex;
    }
    else
    {
        Elf32_Word aCIdx = iDSOBuckets[aBIdx];

        while(iDSOChains[aCIdx] != 0){

            /* Entry already added into the hash table*/
            if(iDSOChains[aCIdx] == aIndex)
                return;

            aCIdx = iDSOChains[aCIdx];
        }
        iDSOChains[aCIdx] = aIndex;
    }
}

void DSOFile::InitProgramHeaderTable()
{
    iDSOSectionNames.push_back(0);
    iCurrentSectionOffset = sizeof(Elf32_Ehdr) + (sizeof(Elf32_Shdr) * iElfHeader->e_shnum);

    //Check if the string table is 4-byte aligned..
    AlignString(iDSOSymNameStrTbl);

    for(uint32_t aIdx = 1; aIdx <= MAX_SECTIONS; aIdx++) {
        switch(aIdx)
        {
            case SYMBOL_SECTION:
                SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_DYNSYM, \
                           sizeof(Elf32_Sym), (iNSymbols * sizeof(Elf32_Sym)),\
                           STRING_SECTION, CODE_SECTION, 4, 0, 0);
                break;
            case STRING_SECTION:
                SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_STRTAB, \
                           1, iDSOSymNameStrTbl.size(), 0, \
                           0, 0, 0,0);
                break;
            case VERSION_SECTION:
                SetSectionFields(aIdx, SECTION_NAME[aIdx], 0x6fffffff, \
                           sizeof(Elf32_Half), (iNSymbols * sizeof(Elf32_Half)), SYMBOL_SECTION, \
                           0, 2, 0, 0);
                break;
            case VER_DEF_SECTION:
                SetSectionFields(aIdx, SECTION_NAME[aIdx],0x6ffffffd, \
                           sizeof(Elf32_Verdaux), (2*(sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux))),\
                           STRING_SECTION, DYNAMIC_SECTION, 4, 0, 0);
                break;
            case HASH_TBL_SECTION:
            {
                    uint32_t aSize = sizeof(Elf32_HashTable) + \
                            (sizeof(Elf32_Sword) * (iHashTbl->nBuckets + iHashTbl->nChains));

                SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_HASH, \
                           0, aSize, SYMBOL_SECTION, 0, 4, 0, 0);
            }
                break;
            case DYNAMIC_SECTION:
                SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_DYNAMIC, \
                           sizeof(Elf32_Dyn), ((MAX_DYN_ENTS + 1) *sizeof(Elf32_Dyn)),\
                           STRING_SECTION, 0, 4, 0,0);
                break;
            case CODE_SECTION:
                SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_PROGBITS, \
                           0, (iNSymbols *sizeof(uint32_t)),\
                           0, 0, 4, (SHF_ALLOC | SHF_EXECINSTR),0);
                break;
            case SH_STR_SECTION:
            {
                uint32_t aSize = iDSOSectionNames.size() + strlen(SECTION_NAME[aIdx]);
                SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_STRTAB, \
                           1, aSize, 0, 0, 0, 0, 0);
                //Check if the string table is 4-byte aligned..
                AlignString(iDSOSectionNames);
                iSections[aIdx].sh_size = iDSOSectionNames.size();
            }
                break;
            default:
                break;
        }
        iSections[aIdx].sh_offset = iCurrentSectionOffset;
        iCurrentSectionOffset += iSections[aIdx].sh_size;

        if(iCurrentSectionOffset %4) {
            iCurrentSectionOffset += (4 - (iCurrentSectionOffset %4));
        }
    }
}

/**
This function sets the section header fields and creates headers name table.
@param aSectionIndex The index of the section
@param aSectionName The name of the section
@param aType The type of the section
@param aEntSz The size of each entry of the section
@param aSectionSize The size of the section
@param aLink The section this section is linked to
@param aInfo Extra information. Depends on the section type of this section.
@param aAddrAlign The address alignment of this section.
@param aFlags Section flags
@param aAddr The address of this section in memory(Here it remains 0)
*/
void DSOFile::SetSectionFields(uint32_t aSectionIndex, const char* aSectionName, uint32_t aType, \
                                   uint32_t aEntSz, uint32_t aSectionSize, uint32_t aLink, \
                                   uint32_t aInfo, uint32_t aAddrAlign, uint32_t aFlags, \
                                   uint32_t aAddr)
{
    iSections[aSectionIndex].sh_name            = iDSOSectionNames.size();
    iDSOSectionNames += aSectionName;
    iDSOSectionNames.push_back(0);

    iSections[aSectionIndex].sh_type            = aType;
    iSections[aSectionIndex].sh_entsize        = aEntSz;
    iSections[aSectionIndex].sh_size            = aSectionSize;
    iSections[aSectionIndex].sh_link            = aLink;
    iSections[aSectionIndex].sh_flags        = aFlags;
    iSections[aSectionIndex].sh_addralign    = aAddrAlign;
    iSections[aSectionIndex].sh_info            = aInfo;
    iSections[aSectionIndex].sh_addr            = aAddr;
}

void DSOFile::InitDynamicEntries()
{
    for(uint32_t aIdx = 0; aIdx <= MAX_DYN_ENTS; aIdx++ ) {
        switch(aIdx) {
            case DSO_DT_DSONAME:
                iDSODynTbl[aIdx].d_tag = DT_SONAME;
                iDSODynTbl[aIdx].d_val = iDSODaux[0].vda_name;
                break;
            case DSO_DT_SYMTAB:
                iDSODynTbl[aIdx].d_tag = DT_SYMTAB;
                iDSODynTbl[aIdx].d_val = iSections[SYMBOL_SECTION].sh_offset;
                break;
            case DSO_DT_SYMENT:
                iDSODynTbl[aIdx].d_tag = DT_SYMENT;
                iDSODynTbl[aIdx].d_val = iSections[SYMBOL_SECTION].sh_entsize;
                break;
            case DSO_DT_STRTAB:
                iDSODynTbl[aIdx].d_tag = DT_STRTAB;
                iDSODynTbl[aIdx].d_val = iSections[STRING_SECTION].sh_offset;
                break;
            case DSO_DT_STRSZ:
                iDSODynTbl[aIdx].d_tag = DT_STRSZ;
                iDSODynTbl[aIdx].d_val = iDSOSymNameStrTbl.size();
                break;
            case DSO_DT_VERSYM:
                iDSODynTbl[aIdx].d_tag = DT_VERSYM;
                iDSODynTbl[aIdx].d_val = iSections[VERSION_SECTION].sh_offset;
                break;
            case DSO_DT_VERDEF:
                iDSODynTbl[aIdx].d_tag = DT_VERDEF;
                iDSODynTbl[aIdx].d_val = iSections[VER_DEF_SECTION].sh_offset;
                break;
            case DSO_DT_VERDEFNUM:
                iDSODynTbl[aIdx].d_tag = DT_VERDEFNUM;
                iDSODynTbl[aIdx].d_val = 2;
                break;
            case DSO_DT_HASH:
                iDSODynTbl[aIdx].d_tag = DT_HASH;
                iDSODynTbl[aIdx].d_val = iSections[HASH_TBL_SECTION].sh_offset;
                break;
            case DSO_DT_NULL:
                iDSODynTbl[aIdx].d_tag = DT_NULL;
                iDSODynTbl[aIdx].d_val = 0;
                break;
            default:
                break;
        }
    }
}

Elf32_Ehdr* CreateElfHeader()
{
    Elf32_Ehdr* h = new Elf32_Ehdr();
    //create ELF header
    const unsigned char c[EI_NIDENT] = {0x7f, 'E', 'L', 'F',
        ELFCLASS32, ELFDATA2LSB, 1, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};       // e_ident

    for (uint32_t i=0; i <EI_NIDENT;i++)
        h->e_ident[i] = c[i];

    h->e_type        = ET_DYN;
    h->e_machine    = EM_ARM;
    h->e_version    = EV_CURRENT;
    h->e_entry        = 0;
    h->e_shoff        = sizeof(Elf32_Ehdr);
    h->e_flags        = EF_ARM_BPABI_VERSION | EF_ARM_SYMSARESORTED;
    h->e_ehsize    = sizeof(Elf32_Ehdr);
    h->e_phentsize = sizeof(Elf32_Phdr);
    h->e_shentsize = sizeof(Elf32_Shdr);
    h->e_shnum        = MAX_SECTIONS + 1;
    h->e_shstrndx    = SH_STR_SECTION;
    h->e_phnum        = 2;
    return h;
}

void DSOFile::InitProgHeader()
{
    //Update the program header offset..
    iElfHeader->e_phoff        = iCurrentSectionOffset;

    iProgHeader[0].p_align    = 4;
    iProgHeader[0].p_offset = iSections[CODE_SECTION].sh_offset;
    iProgHeader[0].p_type    = PT_LOAD;
    iProgHeader[0].p_flags    = (PF_X | PF_ARM_ENTRY);
    iProgHeader[0].p_filesz = iSections[CODE_SECTION].sh_size;
    iProgHeader[0].p_paddr    = 0;
    iProgHeader[0].p_vaddr    = 0;
    iProgHeader[0].p_memsz  = iSections[CODE_SECTION].sh_size;

    iProgHeader[1].p_align    = 4;
    iProgHeader[1].p_offset = iSections[DYNAMIC_SECTION].sh_offset;
    iProgHeader[1].p_type    = PT_DYNAMIC;
    iProgHeader[1].p_flags    = (PF_R );
    iProgHeader[1].p_filesz = iSections[DYNAMIC_SECTION].sh_size;
    iProgHeader[1].p_paddr    = 0;
    iProgHeader[1].p_vaddr    = 0;
    iProgHeader[1].p_memsz  = 0;
}

/**
This function aligns the string table to a 4-byte boundary
*/
void AlignString(string& aStr)
{
    if( aStr.size() %4 )
    {
        uint32_t aPad = 4 - (aStr.size() %4);
        while(aPad--)
            aStr.push_back(0);
    }
}

void DSOFile::WriteElfContents(const char* dsoFile)
{
    uint32_t pos = 0;

    std::fstream fs(dsoFile, std::fstream::binary | std::fstream::out | std::fstream::trunc);
    if(!fs)
        ReportError(FILEOPENERROR, dsoFile);

    // The ELF header..
    fs.write((const char*)iElfHeader, sizeof(Elf32_Ehdr));
    InfoPrint("Elf Header", pos, sizeof(Elf32_Ehdr));

    //Section headers
    fs.write((const char*)iSections, (MAX_SECTIONS + 1) * sizeof(Elf32_Shdr));
    InfoPrint("Section Headers", pos, sizeof(Elf32_Shdr) * (MAX_SECTIONS + 1));

    //Each section..

        //code
        fs.write((const char*)iCodeSectionData, sizeof(uint32_t) * iNSymbols);
        InfoPrint(" Code sections", pos, sizeof(uint32_t) * iNSymbols);

        //dyn table
        fs.write((const char*)iDSODynTbl, sizeof(Elf32_Dyn) * (MAX_DYN_ENTS + 1)); // Elf32_Dyn  iDSODynTbl[MAX_DYN_ENTS+1];
        InfoPrint(" Dyn table", pos, sizeof(Elf32_Dyn) * (MAX_DYN_ENTS + 1));

        //hash table
        uint32_t aSz = 2 + iNSymbols + iNSymbols /3 + iNSymbols%0x3;
        fs.write((const char*)iHashBuf, sizeof(Elf32_Word) * aSz);
        InfoPrint(" Hash table", pos, sizeof(Elf32_Word) * aSz);

        //version def table
        for(uint32_t index = 0; index < 2; index++) {
            fs.write((const char*)&iVersionDef[index], sizeof(Elf32_Verdef));
            fs.write((const char*)&iDSODaux[index], sizeof(Elf32_Verdaux));
        }
        InfoPrint(" Version def table", pos,
                  (sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux)) * 2);

        //version table
        fs.write((const char*)iVersionTbl, iNSymbols * sizeof(Elf32_Half));

        uint32_t nPads = 4 - (iSections[VERSION_SECTION].sh_size %4);
        if(nPads){
            const char c[4] = {};
            fs.write(c, nPads);
        }
        InfoPrint(" Version table", pos,
            sizeof(Elf32_Half) * iNSymbols + nPads);

        //string table
        aSz = iDSOSymNameStrTbl.size();
        fs.write(iDSOSymNameStrTbl.data(), aSz);
        InfoPrint(" String table", pos, aSz);

        //Sym table
        fs.write((const char*)iElfDynSym, sizeof(Elf32_Sym) * iNSymbols);
        InfoPrint(" Sym table", pos, sizeof(Elf32_Sym) * iNSymbols);

        //headers name table
        aSz = iDSOSectionNames.size();
        fs.write(iDSOSectionNames.data(), aSz);
        InfoPrint(" Section header", pos, aSz);

    //program header
    fs.write((const char*)iProgHeader, sizeof(Elf32_Phdr) * 2);
    InfoPrint("Program header", pos, sizeof(Elf32_Phdr) * 2);
    InfoPrint("File end", pos, 0);

    if(!fs)
        ReportError(FILESTORERROR, dsoFile);
    fs.close();
}

void InfoPrint(const char* hdr, uint32_t& pos, const uint32_t offset)
{
#ifdef EXPLORE_DSO_BUILD
    printf("%s starts at: %08x and ends at: %08x"
       " with size: %06x\n\n", hdr, pos, pos + offset, offset);
    pos += offset;
#endif // EXPLORE_DSO_BUILD
}

