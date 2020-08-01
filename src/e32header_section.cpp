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
// Creates and fills E32Image header fields from command line arguments.
//
//

#include "e32flags.h"
#include "e32common.h"
#include "symbiantime.h"
#include "elf2e32_opt.hpp"
#include "e32header_section.h"

E32HeaderSection::E32HeaderSection(const Args* opts): iHeaderData(opts)
{}

E32SectionUnit E32HeaderSection::MakeE32Header()
{
    // set CRC, compression type after E32Image done
    E32ImageHeader* hdr = new E32ImageHeader();
    hdr->iUid1 = iHeaderData->iUid1;
    hdr->iUid2 = iHeaderData->iUid2;
    hdr->iUid3 = iHeaderData->iUid3;
    hdr->iModuleVersion = iHeaderData->iVersion;
    hdr->iCompressionType = iHeaderData->iCompressionMethod;

    SymbianTime t;
    hdr->iTimeLo = t.TimeLo();
    hdr->iTimeHi = t.TimeHi();

    E32Flags flags(iHeaderData);
    hdr->iFlags = flags.Run();

    hdr->iHeapSizeMin = iHeaderData->iHeapMin;
    hdr->iHeapSizeMax = iHeaderData->iHeapMax;

    hdr->iStackSize = iHeaderData->iStack;
    hdr->iProcessPriority = iHeaderData->iPriority;

    iHeader.insert(iHeader.begin(), (char*)hdr, (char*)hdr + sizeof(E32ImageHeader));
    delete hdr;

    E32ImageHeaderJ* hdrJ = new E32ImageHeaderJ();
    iHeader.insert(iHeader.end(), (char*)hdrJ, (char*)hdrJ + sizeof(E32ImageHeaderJ));
    delete hdrJ;

    E32ImageHeaderV* v = new E32ImageHeaderV();
    v->iS.iSecureId = iHeaderData->iSid;
    v->iS.iVendorId = iHeaderData->iVid;
    v->iS.iCaps = ProcessCapabilities(iHeaderData->iCapability);
    iHeader.insert(iHeader.end(), (char*)v, (char*)v + sizeof(E32ImageHeaderV));
    delete v;
    hdr = (E32ImageHeader*)&iHeader[0];
    hdr->iCodeOffset = iHeader.size();
    return iHeader;
}
