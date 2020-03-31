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
// Creates and fills E32Image header fields
//
//

#include "e32flags.h"
#include "e32common.h"
#include "symbiantime.h"
#include "elf2e32_opt.hpp"
#include "e32headerbuilder.h"

E32HeaderBuilder::E32HeaderBuilder(const Args* opts): iHeaderData(opts)
{
    //ctor
}

E32HeaderBuilder::~E32HeaderBuilder()
{
    //dtor
}

E32SectionUnit E32HeaderBuilder::MakeE32Header()
{
    // set CRC, compression type after E32Image done
    iHeader.reserve(sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ) +
                    sizeof(E32ImageHeaderV));
    E32ImageHeader* hdr = (E32ImageHeader*)iHeader.data();
    hdr->iUid1 = iHeaderData->iUid1;
    hdr->iUid2 = iHeaderData->iUid2;
    hdr->iUid3 = iHeaderData->iUid3;
    hdr->iModuleVersion = iHeaderData->iVersion;

    SymbianTime t;
    hdr->iTimeLo = t.TimeLo();
    hdr->iTimeHi = t.TimeHi();

    E32Flags flags(iHeaderData);
    hdr->iFlags = flags.Run();

    hdr->iHeapSizeMin = iHeaderData->iHeapMin;
    hdr->iHeapSizeMax = iHeaderData->iHeapMax;

    hdr->iStackSize = iHeaderData->iStack;
    hdr->iProcessPriority = iHeaderData->iPriority;

    E32ImageHeaderV* v = (E32ImageHeaderV*)iHeader[sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ)];
    v->iS.iSecureId = iHeaderData->iSid;
    v->iS.iVendorId = iHeaderData->iVid;
   // v->iS.iCaps = iHeaderData->iCapability;
    // ends with full bitmap or sparse bitmap
    return iHeader;
}
