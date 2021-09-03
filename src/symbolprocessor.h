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
// Got Symbols from ELF file, Def file and --sysdef option.
// Rules for output:
//    symbols from --sysdef add to symbols from def file:
//     1) add new symbol
//     2) add new symbol which ordinal already assigned to another symbol
//     3) change ordinal already defined symbol
//     4) change ordinal already defined symbol which ordinal already assigned to another symbol
//     got symbols from elf and sort by names and merge with processed symbols
//     5) ordinals starts from "1"
//
// Frozen mode(on by default, except for ecom dlls) - adding new symbols not allowed, change
//  symbol properties not allowed, option --unfrozen off this mode.
// Rules for conflict resolving for Frozen mode off:
//   sysdef symbols freely change def symbols.
//   in case 2) new symbol adds and other symbol gets new ordinal with next value after last ordinal
//   in case 4) already defined symbol gets new ordinal with next value after last ordinal
//
// In any case .def should be created because original elf2e32 does so.
//
// For a custom dll and for option "--excludeunwantedexports", the new exports should be filtered,
// so that only the exports from the frozen DEF file are considered.
//

#ifndef SYMBOLPROCESSOR_H
#define SYMBOLPROCESSOR_H

#include <list>
#include <string>
#include "common.hpp"

class Args;
class Symbol;
class ElfParser;

class SymbolProcessor
{
    public:
        SymbolProcessor(const Args* args, const ElfParser* elfParser = nullptr);
        Symbols GetExports();
        bool IsAbsentFound();
        ~SymbolProcessor();
    private:
        void ProcessPredefinedSymbols();
        void ProcessElfSymbols();
        bool SimpleSymbolsProcessing();
        Symbols GetDSOSymbols();
        Symbols GetElfExports();
        Symbols FromSysdef();
        void SetElf_st_value(const Symbols& fromElf);
        void CheckForErrors(bool unfrozen, std::list<std::string> missedSymbols, const std::string& src);
    private:
        const Args* iArgs = nullptr;
        const ElfParser* iElfParser = nullptr;
        Symbols iSymbols;
        bool iAbsent = false;
};

bool IsImportedSymbol(uint32_t symIdx, const ElfParser* parser);

#endif // SYMBOLPROCESSOR_H
