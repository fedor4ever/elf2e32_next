// Copyright (c) 2021 Strizhniou Fiodar.
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Description:
// This class creates Exports bitmap
//

#include <assert.h>
#include "e32common.h"
#include "exportbitmap_section.h"

ExportBitmapSection::ExportBitmapSection(uint32_t exportsCount, const E32SectionUnit& exportTable, uint32_t absentVal):
    iExportsCount(exportsCount), iExportTable(exportTable), iAbsentVal(absentVal) {}

ExportBitmapSection::~ExportBitmapSection()
{
    //dtor
}

E32Section ExportBitmapSection::CreateExportBitmap()
{
    E32Section exportBitMap;
    exportBitMap.info = "BITMAP";
    exportBitMap.type = E32Sections::BITMAP;

    size_t memsz = (iExportsCount + 7) >> 3;  // size of complete bitmap
    size_t mbs = (memsz + 7) >> 3;    // size of meta-bitmap
    uint8_t* bitMap = new uint8_t[memsz](0xff);

    uint32_t* exports = ((uint32_t*)iExportTable.data());
    exports++; // skip header
    uint32_t absentExportsCnt = 0;

    for(int i = 0; i < iExportsCount; ++i)
    {
        if (exports[i] == iAbsentVal)
        {
            bitMap[i>>3] &= ~(1u << (i & 7));
            absentExportsCnt++;
        }
    }

    exportBitMap.section.insert(exportBitMap.section.begin(),
                        (char*)bitMap, (char*)bitMap + memsz);

    delete[] bitMap;

    if(absentExportsCnt == 0)
    { // No "missed export" == no Exports bitmap.
        exportBitMap.type = E32Sections::EMPTY_SECTION;
        return exportBitMap;
    }

    size_t nbytes = 0;
    for(uint32_t i = 0; i < memsz; i++) // check why used ++i???
    {
        if ((uint8_t)exportBitMap.section[i] != 0xff)
            ++nbytes; // number of groups of 8
    }

//   at first try deal with Full bitmap
    iExportDescType = KImageHdr_ExpD_FullBitmap;
    iExportDescSize = memsz;
    if (mbs + nbytes < memsz)
    {
        iExportDescType = KImageHdr_ExpD_SparseBitmap8;
        iExportDescSize = mbs + nbytes;
    }

    if (iExportDescType == KImageHdr_ExpD_FullBitmap)
    {
        assert(memsz < 65536);
        return exportBitMap;
    }
    else
    {
        uint32_t extra_space = mbs + nbytes - 1;
        extra_space = (extra_space + sizeof(uint32_t) - 1) &~ (sizeof(uint32_t) - 1);
        extra_space++;

        assert((mbs + nbytes) < 65536);
        uint8_t* aBuf = new uint8_t[extra_space]();
        uint8_t* mptr = aBuf;
        uint8_t* gptr = mptr + mbs;
        for (uint32_t i = 0; i < memsz; i++)
        {
            if ((uint8_t)exportBitMap.section[i] != 0xff)
            {
                mptr[i>>3] |= (1u << (i&7));
                *gptr++ = (uint8_t)exportBitMap.section[i];
            }
        }

        E32SectionUnit tmp;
        tmp.insert(tmp.begin(), (char*)aBuf, (char*)aBuf + extra_space);
        exportBitMap.section = tmp;
        delete[] aBuf;
    }
    return exportBitMap;
}

uint8_t ExportBitmapSection::ExportDescType() const
{
    return iExportDescType;
}

uint16_t ExportBitmapSection::ExportDescSize() const
{
    return iExportDescSize;
}
