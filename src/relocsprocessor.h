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


#ifndef RELOCSPROCESSOR_H
#define RELOCSPROCESSOR_H

#include <map>
#include <string>
#include <vector>

class ElfParser;
struct RelocBlock;

struct VersionInfo
{
    std::string iSOName;
    std::string iLinkAs;
};

struct StringPtrLess
{
    bool operator() (const char * lhs, const char * rhs) const;
};

class RelocsProcessor
{
    public:
        RelocsProcessor(const ElfParser* elf);
        ~RelocsProcessor() {}
        void Process();
        uint32_t ImportsCount() const;
        uint32_t DllCount() const;
        void ProcessVerInfo();
        std::vector<std::string> StrTableData() const;

    private:
        void AddToImports(uint32_t index, Elf32_Rela rela);
        template <class T>
        void ProcessRelocations(const T* elfRel, const RelocBlock& r);

    private:
        const ElfParser* iElf = nullptr;
        std::vector<std::string> iDSONames;
        std::vector<std::string> iLinkAsNames;
        std::vector<VersionInfo> iVerInfo;
        uint16_t* iVersionTbl = nullptr;  //= iElf->VersionTbl();
        uint32_t iImportsCount = 0;
        uint32_t iDllCount = 0;
};

#endif // RELOCSPROCESSOR_H
