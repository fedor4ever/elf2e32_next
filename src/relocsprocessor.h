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

struct LocalReloc
{
    Elf32_Phdr* iSegment      = nullptr;
    uint32_t	iSymNdx;
    Elf32_Rela  iRela;
    Elf32_Sym*  iSymbol       = nullptr;
    bool        iHasRela      = false;
    uint8_t	    iRelType      = 0;
    bool        iDelSym       = false; // true for absent symbols only
    bool        iVeneerSymbol = false;
    /// TODO (Administrator#1#06/26/20): Use relocation types directly:
    //      KReservedRelocType, KTextRelocType, KDataRelocType, KInferredRelocType
    ESegmentType iSegmentType = ESegmentUndefined;
};

class RelocsProcessor
{
    public:
        RelocsProcessor(const ElfParser* elf, const Symbols& s, bool symlook);
        ~RelocsProcessor() {}
        void Process();

        E32Section CodeRelocsSection();
        E32Section DataRelocsSection();
        ImportLibs GetImports();

        uint32_t ImportsCount() const;
        uint32_t DllCount() const;
        void ProcessVerInfo();
        std::vector<std::string> StrTableData() const;
        uint16_t Fixup(const Elf32_Sym* s);
        size_t ExportTableAddress() const;

    private:
        void RelocsFromSymbols();
        void ProcessSymbolInfo();
        void ProcessVeneers();
        void AddToImports(uint32_t index, Elf32_Rela rela);
        void AddToLocalRelocations(uint32_t index,
                uint8_t relType, Elf32_Rela rela);
        void AddToLocalRelocations(uint32_t aAddr, uint32_t index,
                uint8_t relType,
                Elf32_Sym* aSym, bool aDelSym = false,
                bool veneerSymbol = false);
        template <class T>
        void ProcessRelocations(const T* elfRel, const RelocBlock& r);
        void SortReloc(const LocalReloc& r);
        void SortRelocs();
        void ApplyLocalReloc(const LocalReloc& rel);

    private:
        const ElfParser* iElf = nullptr;
        std::vector<std::string> iDSONames;
        std::vector<std::string> iLinkAsNames;
        std::vector<VersionInfo> iVerInfo;
        std::vector<LocalReloc> iCodeRelocations;
        std::vector<LocalReloc> iDataRelocations;
        const Symbols& iRelocSrc;
        ImportLibs iImports;
        uint16_t* iVersionTbl = nullptr;  //= iElf->VersionTbl();
        uint32_t iImportsCount = 0;
        uint32_t iDllCount = 0;
        size_t iExportTableAddress = 0;
        bool iSymLook = false;
};

#endif // RELOCSPROCESSOR_H
