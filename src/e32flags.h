// Copyright (c) 2018-2024 Strizhniou Fiodar
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
// Class for first-time initialization of E32ImageHeader::iFlags.
//      For details see E32Flags::Run().
//
// Errata: by mistake for dll some tools set on KImageDebuggable
// flag if --debuggable option provided.

#ifndef E32FLAGS_H
#define E32FLAGS_H

#include <cstdint>

struct Args;

class E32Flags
{
    public:
        E32Flags(const Args* args);
        ~E32Flags();
        uint32_t Run();

    private:
        void SetFPU();
        void SetPaged();
        void SetSmpSafe();
        void SetCallentry();
        void SetDebuggable();
        void SetSymbolLookup();

    private:
        const Args* iArgs = nullptr;
        uint32_t iFlags = 0;
};

#endif // E32FLAGS_H
