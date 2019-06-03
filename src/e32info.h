// Copyright (c) 2018-2019 Strizhniou Fiodar
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
// Class E32Info prints info for specified E32 image
// @internalComponent
// @released
//
//

#ifndef E32INFO_H
#define E32INFO_H

#include <cstdint>
#include <cstddef>
#include <string>

#include "e32common.h"

class E32Parser;
struct E32ImageHeader;
struct Args;

class E32Info
{
    public:
        E32Info(Args *param);
        ~E32Info();
        void Run();
    public:
//        These functions print for that dump options [hscdeit]:
        void HeaderInfo(); //h
        void SecurityInfo(bool PrintCapabilityNames = false); //s
        void CodeSection(); //c // use PrintHexData()
        void DataSection(); //d // use PrintHexData()
        void ExportTable(); //e
        void ImportTableInfo(); //i
        void SymbolInfo(); //t
    private:
        void CPUIdentifier(uint16_t CPUType, bool &isARM);
        void ImagePriority(TProcessPriority priority) const;
    private:
        Args *iParam = nullptr;
        std::string iFlags = nullptr;
        const char *iE32File = nullptr;
        E32Parser *iE32 = nullptr;
        const E32ImageHeader *iHdr1 = nullptr;
};

#endif // E32INFO_H
