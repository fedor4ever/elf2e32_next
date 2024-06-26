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
// Parse ELF Images
//
//

#ifndef ELFPARSER_H
#define ELFPARSER_H

#include <string>
#include <vector>
#include "elfdefs.h"

struct Elf32_Sym;
struct Elf32_Rel;
struct Elf32_Rela;

struct RelocBlock
{
    uint32_t    size;
    Elf32_Rel*  rel  = nullptr;
    Elf32_Rela* rela = nullptr;
    const char* type = nullptr; // or rel, rela, pltrel, pltrela
};

class ElfParser
{
    public:
        ElfParser(const std::string& elf);
        void GetElfFileLayout();
        ~ElfParser();
        bool IsRelocationFixRequired() const;
        uint32_t BssSegmentSize() const;

        uint32_t CodeSegmentSize() const;
        const char* CodeSegment() const;

        uint32_t DataSegmentSize() const;
        const char* DataSegment() const;

        Elf32_Word GetRelocationOffset(Elf32_Addr r_offset) const;
        Elf32_Word GetRelocationValue(Elf32_Addr r_offset) const;
        Elf32_Word* GetRelocationPlace(Elf32_Addr r_offset) const;
    public:
        uint32_t ImportsCount() const;
        uint32_t ROVirtualAddress() const;
        uint32_t RWVirtualAddress() const;
        uint32_t ExceptionDescriptor() const;
    public:
        const char* GetSymbolNameFromStringTable(uint32_t index) const;
        const char* GetNameFromStringTable(uint32_t offset) const;
        uint32_t GetSymbolOrdinal(const char* aSymName) const;
        Elf32_Sym* FindSymbol(const char* aName) const;
        uint32_t GetSymbolOrdinal(const Elf32_Sym* aSym) const;
    public:
        Elf32_Sym* GetSymbolTableEntity(uint32_t index) const;
        Elf32_Sym* LookupStaticSymbol(const char* aName) const;
        Elf32_Phdr* GetSegmentAtAddr(Elf32_Addr addr) const;
        Elf32_Phdr* Segment(uint16_t aType) const;
        ESegmentType SegmentType(Elf32_Addr addr) const;
        uint32_t* GetDSOImportsOrdinals() const;
    public:
        uint32_t EntryPoint() const;
        uint32_t EntryPointOffset() const;
        Elf32_Verneed* GetElf32_Verneed() const;
        Elf32_Verdef*  GetElf32_Verdef() const;
        Elf32_Word VerInfoCount() const;
        const char* SOName() const;
    public:
        uint32_t Addend(const Elf32_Rel*  r) const;
        uint32_t Addend(const Elf32_Rela* r) const;
        Elf32_Half* VersionTbl() const;
        std::vector<RelocBlock> GetRelocs() const;
    public:
        uint16_t Segment(const Elf32_Sym* s) const;
        bool ImageIsDll() const;
    public:
        Elf32_Addr* ExportTable() const;
        Elf32_Sym* Lim() const;
        Elf32_Sym* SymTab() const;
        const char* StrTab() const;
    public: // only for DSO CRC32 checksum, see DSOFile::CreateSectionHeaders()
        const char* ElfWithFixedHashTable();
        std::streamsize FileSize();
    private:
        void ValidateElfImage();
        void ProcessSectionHeaders();
        void ProcessProgHeaders();
        void ProcessDynamicTable();
    private:
        std::string iFile;
        const char* iFileBuf = nullptr;
        std::streamsize iFileBufSize = 0;
    private:
        /** Elf header */
        Elf32_Ehdr* iElfHeader = nullptr;

        /** Section header table. */
        Elf32_Shdr* iSections = nullptr;
        /** Section header names table. */
        char* iSectionHdrStrTbl = nullptr;
    private:
        /** The static symbol table.*/
        Elf32_Sym* iSymTab = nullptr;
        char* iStrTab = nullptr;
        Elf32_Sym* iLim = nullptr;
    private:
        const char* iCommentSection = nullptr;
    private:
        /** The dynamic table*/
        Elf32_HashTable* iHashTbl = nullptr;
    private:
        /** Program headers table*/
        Elf32_Phdr* iProgHeader = nullptr;
        Elf32_Phdr* iDynSegmentHdr = nullptr;

        Elf32_Phdr* iCodeSegmentHdr = nullptr;
        uint32_t iCodeSegmentIdx = 0;
        const char* iCodeSegment = nullptr;

        Elf32_Phdr* iDataSegmentHdr = nullptr;
        uint32_t iDataSegmentIdx = 0;
        const char* iDataSegment = nullptr;
        	/** The dynamic symbol array.*/
        Elf32_Sym* iElfDynSym = nullptr;//The ELF symbol

        Elf32_Verneed* iVersionNeed = nullptr;

    private:
        Elf32_Verdef* iVersionDef = nullptr;
        Elf32_Word iVerInfoCount = 0;
        Elf32_Half* iVersionTbl = nullptr;
        //others
        char* iStringTable = nullptr;
        uint32_t iSONameOffset = 0;
    private:
        std::vector<RelocBlock> iRelocs;
        RelocBlock iRel;
        RelocBlock iRela;
        RelocBlock iPltRel;
        RelocBlock iPltRela;
};



#endif // ELFPARSER_H
