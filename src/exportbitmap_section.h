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
// This class creates export description for the absent symbols.
//
// ***************FAQ*****************
// What is Exports bitmap?
// Exports bitmap contains char array, where every char represent exported symbol.
//
// Why it used?
// It is created then and only then if there is at least one "missed export".
//
// What if no "missed export"?
// No "missed export" == no Exports bitmap.
//
// How they represented in E32 Image?
// Via iExportDescSize, iExportDescType and iExportDesc.
//
// How they represented in E32 Image if no "missed export"?
// iExportDescSize = 0, iExportDescType = KImageHdr_ExpD_NoHoles,
// iExportDesc = 0 and it acts like placeholder.
//
// How they represented in E32 Image if any "missed export"?
// iExportDescSize,
// iExportDescType = (KImageHdr_ExpD_FullBitmap || KImageHdr_ExpD_SparseBitmap8),
// iExportDesc == count all exports
//
// What is "missed export"?
// It is placeholder for removed symbol to preserve stable ABI.
//

#ifndef EXPORTBITMAPPROCESSOR_H
#define EXPORTBITMAPPROCESSOR_H

#include "e32file.h"

class ExportBitmapSection
{
    public:
        ExportBitmapSection(uint32_t exportsCount, const E32SectionUnit& exportTable, uint32_t absentVal);
        ~ExportBitmapSection();

        E32Section CreateExportBitmap();
        uint8_t ExportDescType() const;
        uint16_t ExportDescSize() const;

    private:
        uint32_t iExportsCount = 0;
        const E32SectionUnit& iExportTable;
        uint32_t iAbsentVal = 0;
    private:
        uint16_t iExportDescSize = 0;
        uint8_t iExportDescType = 0;
};

#endif // EXPORTBITMAPPROCESSOR_H
