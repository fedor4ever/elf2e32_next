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
// This class creates symbol lookup section
//
//

#ifndef SYMBOLLOOKUPPROCESSOR_H
#define SYMBOLLOOKUPPROCESSOR_H

#include <string>
#include <vector>
#include "e32file.h"

struct E32EpocExpSymInfoHdr;

class SymbolLookupSection
{
    public:
        SymbolLookupSection(const Symbols& s, uint32_t dllCount);
        ~SymbolLookupSection(){}
        E32Section SymlookSection();
    private:
        void ProcessSymbols();
        void InitHeader(E32EpocExpSymInfoHdr& s);
    private:
        Symbols iSymbols;
        std::vector<uint32_t> iSymAddrTab;
        std::vector<uint32_t> iSymNameOffTab;
        std::string  iSymbolNames;
        uint32_t     iSymNameOffset = 0;
        uint32_t     iDllCount = 0;
};

#endif // SYMBOLLOOKUPPROCESSOR_H
