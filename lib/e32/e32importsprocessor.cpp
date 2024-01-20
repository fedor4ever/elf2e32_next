// Copyright (c) 2019-2024 Strizhniou Fiodar
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
// Process imports for the elf2e32 tool
//
//

#include <inttypes.h>
#include <stdio.h>

#include "e32common.h"
#include "e32importsprocessor.hpp"


uint32_t E32ImportBlock::Size(uint32_t aImpFmt) const
{
    uint32_t r = sizeof(E32ImportBlockPE2);
    if(aImpFmt != KImageImpFmt_PE2)
        r += iNumberOfImports * sizeof(uint32_t);
    return r;
}

E32ImportParser::E32ImportParser(uint32_t importCount, uint32_t importFormat,
    const E32ImportSection* section): iImportCounter(importCount),
         iImportFormat(importFormat), iSection(section), iNext(section->iImportBlock)
{
}

void E32ImportParser::NextImportBlock()
{
    --iImportCounter;
    uint32_t arrayMemberOffset = iNext->iNumberOfImports;
    if(iImportFormat == KImageImpFmt_PE2)
        arrayMemberOffset = 0;
    iNext = (E32ImportBlock*)(iNext->iImports + arrayMemberOffset);
}

uint32_t E32ImportParser::GetOffsetOfDllName() const
{
    return iNext->iOffsetOfDllName;
}

uint32_t E32ImportParser::GetSectionSize() const
{
    return iSection->iSize;
}

uint32_t E32ImportParser::GetNumberOfImports() const
{
    return iNext->iNumberOfImports;
}

uint32_t E32ImportParser::GetImportOrdinal() const
{
    return iImpOrdinal;
}

uint32_t E32ImportParser::GetImportOffset(uint32_t index) const
{
    return iNext->iImports[index];
}

bool E32ImportParser::HasImports() const
{
    return iImportCounter > 0;
}
