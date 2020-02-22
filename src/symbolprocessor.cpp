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
//
//

#include "argparser.h"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "symbolprocessor.h"

SymbolProcessor::SymbolProcessor(const ElfParser* elfParser, const Args* args):
        iElfParser(elfParser), iArgs(args) {}

SymbolProcessor::~SymbolProcessor()
{
    //dtor
}

Symbols SymbolProcessor::Process()
{
    /// TODO (Administrator#1#02/23/20): Handle ELF and --sysdef
    return SymbolsFromDef(iArgs->iDefinput.c_str());;
}
