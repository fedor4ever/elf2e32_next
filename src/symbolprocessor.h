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

#ifndef SYMBOLPROCESSOR_H
#define SYMBOLPROCESSOR_H

#include "common.hpp"

class ElfParser;
class Args;

class SymbolProcessor
{
    public:
        SymbolProcessor(const ElfParser* elfParser, const Args* args);
        Symbols Process();
        ~SymbolProcessor();
    private:
        ElfParser* iElfParser = nullptr;
        Args* iArgs = nullptr;
};

#endif // SYMBOLPROCESSOR_H
