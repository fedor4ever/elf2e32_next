// Copyright (c) 2019 Strizhniou Fiodar
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

E32ImportParser::E32ImportParser(uint32_t importFormat,
     const E32ImportSection* section, uint32_t numberOfImportDlls):
         iImportFormat(importFormat), iSection(section),
         iNumberOfImportDlls(numberOfImportDlls), iImportCounter(numberOfImportDlls)
{
    iNext = iSection->iImportBlock;
}

void E32ImportParser::NextImportBlock()
{
    --iImportCounter;
    uint32_t arrayMemberOffset = iNext->iNumberOfImports;
    if(iImportFormat == KImageImpFmt_PE2)
        arrayMemberOffset = 0;
    iNext = (E32ImportBlock*)(iNext->iImports + arrayMemberOffset);
}

const uint32_t E32ImportParser::GetOffsetOfDllName()
{
    return iNext->iOffsetOfDllName;
}

const uint32_t E32ImportParser::GetSectionSize()
{
    return iSection->iSize;
}

const uint32_t E32ImportParser::GetNumberOfImports()
{
    return iNext->iNumberOfImports;
}

const uint32_t E32ImportParser::GetImportOrdinal()
{
    return iImpOrdinal;
}

const uint32_t E32ImportParser::GetImportOffset(uint32_t index)
{
    return iNext->iImports[index];
}

bool E32ImportParser::HasImports()
{
    return iImportCounter > 0;
}
