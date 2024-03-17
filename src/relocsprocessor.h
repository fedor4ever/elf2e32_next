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
// Parse and resolve ELF relocs
//
//
// Relocs resolution:
// 1) exports:
//      Elf32_Word* aRelocPlace = iElf->GetRelocationPlace(r_offset);
//      *aRelocPlace = (aReloc.iRela.r_addend<<16) | aOrdinal;
// 2) local:
//      Elf32_Word* aLoc = iElf->GetRelocationPlace(rel.iRela.r_offset);
//      aLoc[0] += rel.iSymbol->st_value;
//
// Note: if option "--namedlookup" supplied and "--definput" omitted E32
// image generated that tool differs from SDK tool output for code relocs
// section. Correctness unknown.
//
// Size of relocation section depends on relocs itself. See at RelocationsSize();
//
// Code relocs builded from 3 sources:
// 1. The dynamic segment relocations
// 2. Exported symbols
// 3. Symbol lookup table (optional)
//

#ifndef RELOCSPROCESSOR_H
#define RELOCSPROCESSOR_H

#include <map>
#include <string>
#include <vector>
#include "e32file.h"
#include "artifactbuilder.h"

class ElfParser;
struct RelocBlock;

struct VersionInfo
{
    std::string iSOName;
    std::string iLinkAs;
};

struct ElfImportRelocation
{
    uint32_t iSymNdx;
    Elf32_Rela iRela;
    std::string iLinkAs;
    std::string iSOName;
};

typedef std::vector<ElfImportRelocation> Relocations;
typedef std::map<std::string, Relocations> ImportLibs;

struct RelocLocalValues
{
    uint16_t iE32Reloc;  // = (uint16_t)((r.iRela.r_offset & 0xfff) | relocType);
    uint16_t iRelocType; // = rp->Fixup(r.iSymbol);
    int      iPage;      // = r.iRela.r_offset & 0xfffff000;
    std::string iSymbolName;
    const char* iType = "unknown";
};

struct LocalReloc
{
    uint32_t	iSymNdx;
    Elf32_Rela  iRela;
    Elf32_Phdr* iSegment      = nullptr;
    Elf32_Sym*  iSymbol       = nullptr;
    bool        iHasRela      = false;
    uint8_t	    iRelType      = 0;
    bool        iDelSym       = false; // true for absent symbols only
    bool        iVeneerSymbol = false;
    /// TODO (Administrator#1#06/26/20): Use relocation types directly:
    //      KReservedRelocType, KTextRelocType, KDataRelocType, KInferredRelocType
    ESegmentType iSegmentType = ESegmentUndefined;
    RelocLocalValues iIntermediates;
};

typedef std::map<std::string, std::vector<LocalReloc> > BadRelocs;

class RelocsProcessor
{
    public:
        RelocsProcessor(const ElfParser* elf, const Symbols& s, bool symlook);
        ~RelocsProcessor() {}
        void Process();

        E32Section CodeRelocsSection();
        E32Section DataRelocsSection();
        ImportLibs GetImports() const;

        uint32_t ImportsCount() const;
        uint32_t DllCount() const;
        void ProcessVerInfo();
        uint16_t Fixup(const Elf32_Sym* s);
        void ValidateLocalReloc(const LocalReloc& r,
                    const std::string& name);

    private:
        void RelocsFromSymbols();
        void ProcessSymbolInfo();
        void ProcessVeneers();
        void AddToImports(uint32_t index, Elf32_Rela rela);
        template <class T>
        void ProcessRelocations(const T* elfRel, const RelocBlock& r);
        void SortReloc(const LocalReloc& r);
        void SortRelocs();
        void ApplyLocalReloc(const LocalReloc& rel);

    private:
        void AddToLocalRelocations(uint32_t index,
                uint8_t relType, Elf32_Rela rela, const char* srcname);
        void AddToLocalRelocations(uint32_t aAddr, uint32_t index,
                uint8_t relType, Elf32_Sym* aSym,
                const char* srcname, bool aDelSym = false,
                bool veneerSymbol = false);

    private:
        const ElfParser* iElf = nullptr;
        std::vector<VersionInfo> iVerInfo;
        std::vector<LocalReloc> iCodeRelocations;
        std::vector<LocalReloc> iDataRelocations;
        const Symbols& iRelocSrc;
        ImportLibs iImports;
        uint16_t* iVersionTbl = nullptr;  //= iElf->VersionTbl();
        uint32_t iImportsCount = 0;
        size_t iExportTableAddress = 0;
        bool iSymLook = false;

    public: //TODO: make them private
        void PrintBadRelocs(const std::string& type, const BadRelocs& rel);
        BadRelocs iBadCodeReloc;
        BadRelocs iBadDataReloc;
};

#endif // RELOCSPROCESSOR_H
