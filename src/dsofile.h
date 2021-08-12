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

/*
///////////////////////////
// The ELF header..      //
///////////////////////////
//Section headers        //
///////////////////////////
  S  //code              //
  e  //dyn table         //
  c  //hash table        //
  t  //version def table //
  i  //version table     //
  o  //string table      //
  n  //Sym table         //
  s  //header name table //
///////////////////////////
//program header         //
///////////////////////////*/

/*
Properties of each section:
    ELF header - fixed size(sizeof(Elf32_Ehdr))
  *  Section headers - fixed size(sizeof(Elf32_Shdr) * (MAX_SECTIONS + 1))
  *  code section - dynamic size(sizeof(Elf32_Sword) * iNSymbols)
  *  dyn table - fixed size(sizeof(Elf32_Dyn) * (MAX_DYN_ENTS + 1))
  *  hash table - dynamic size(sizeof(Elf32_Word) * (2 + Elf32_HashTable::nBuckets + Elf32_HashTable::nChains))
  *  version def table - fixed size( (sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux)) * 2)
  *  version table - dynamic size(sizeof(Elf32_Half) * iNSymbols), round up by 4 with padding by '\0'
  *  string table - dynamic size(length all symbol names + (Symbols::size() - 1) * sizeof(char)),
     round up by 4 with padding by 0
  *  Sym table - dynamic size(sizeof(Elf32_Sym) * iNSymbols)
  *  header name table - fixed size(length all predefined section names separated by 0)
*/
#if !defined(DSOFILE_H)
#define DSOFILE_H

#include <list>
#include <memory>
#include <string>
#include <vector>
#include "elfdefs.h"

#define DEFAULT_VERSION 2

struct Args;
struct Elf32_Ehdr;
class Symbol;

typedef std::list<Symbol*> Symbols;
/// TODO (Administrator#9#05/08/20): Use smartpointers to prevent memory leaks
//typedef std::list   <std::shared_ptr<Symbol*>> Symbols;

//enum for DYN entries
enum DYN_ENTRIES {
    DSO_DT_DSONAME=0,
    DSO_DT_SYMTAB,
    DSO_DT_SYMENT,
    DSO_DT_STRTAB,
    DSO_DT_STRSZ,
    DSO_DT_VERSYM,
    DSO_DT_VERDEF,
    DSO_DT_VERDEFNUM,
    DSO_DT_HASH,
    DSO_DT_NULL,
    MAX_DYN_ENTS=DSO_DT_NULL,
};

//! This class generates the import library in elf header, where exported function names stored in Elf string table
// and their ordinals used by dynamic linkes(ex: gnu ld.so) stored in code section.
class DSOFile
{
public:
    ~DSOFile();
    void WriteDSOFile(const Args* opts, const Symbols& sym);
private:
    void CreateSectionHeaders();
    void CreateTablesFromSymbols(const Symbols& s);
    void InitVersionTable(const Args* opts);
    void InitProgramHeaderTable();
    void InitDynamicEntries();
    void InitProgHeader();
private:
    void CreateHashTable();
    void AddToHashTable(const char* aSymName, uint32_t aIndex);
    void SetSectionFields(uint32_t aSectionIndex, const char* aSectionName,
                        uint32_t aType, uint32_t aEntSz, uint32_t aSectionSize,
                        uint32_t aLink, uint32_t aInfo, uint32_t aAddrAlign,
                        uint32_t aFlags, uint32_t aAddr);

    void WriteElfContents(const char* dsoFile);

private:
    uint32_t        iNSymbols = 0; // = Symbols.size() + 1

    /** This indicates the start of current section in processed DSO.*/
    uint32_t        iCurrentSectionOffset = 0;

    /*DSO content Fields*/

    /** The Elf version definition auxiliary section*/
    Elf32_Verdaux*   iDSODaux=nullptr;

    /** The Elf Dynamic section table*/
    Elf32_Dyn        iDSODynTbl[MAX_DYN_ENTS+1];

    /** The code section*/
    uint32_t*        iCodeSectionData=nullptr;

    /** The Elf string table*/
    std::string      iDSOSymNameStrTbl;

    /** The Elf Section-header string table*/
    std::string      iDSOSectionNames;

private:
    /** The elf header pointer which points to the base of the file records */
    Elf32_Ehdr*    iElfHeader = nullptr;

    /** This member points to the base of the section header table. */
    Elf32_Shdr*    iSections   = nullptr;
    Elf32_Verdef*  iVersionDef = nullptr;
    Elf32_Half*    iVersionTbl = nullptr;

    /** The dynamic program header of the elf file */
    Elf32_Phdr*    iProgHeader = nullptr;

    /** The dynamic symbol array. */
    Elf32_Sym*     iElfDynSym = nullptr;//The ELF symbol

private:
    Elf32_Word*        iHashBuf = nullptr;

    Elf32_HashTable*   iHashTbl = nullptr;
    /** The Buckets for the hash table*/
    Elf32_Word*        iDSOBuckets=nullptr;

    /** The chains pointed to by the buckets belonging to the hash table*/
    Elf32_Word*        iDSOChains=nullptr;
};

#endif // DSOFILE_H
