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

#include "e32common.h"
#include "e32importsprocessor.hpp"

/**
Return address of first import in this block.
For import format KImageImpFmt_ELF, imports are list of code section offsets.
For import format KImageImpFmt_PE, imports are a list of imported ordinals.
For import format KImageImpFmt_PE2, the import list is not present and should not be accessed.
*/
const uint32_t* E32ImportBlock::Imports() const {
    return (const uint32_t*)(this + 1);
}

/**
Return pointer to import block which immediately follows this one.
@param aImpFmt Import format as obtained from image header.
*/
const E32ImportBlock* E32ImportBlock::NextBlock(uint32_t aImpFmt) const
{
    const E32ImportBlock* next = this + 1;
    if(aImpFmt!=KImageImpFmt_PE2)
        next = (const E32ImportBlock*)( (uint8_t*)next + iNumberOfImports * sizeof(uint32_t) );
    return next;
}

/**
Return size of this import block.
@param aImpFmt Import format as obtained from image header.
*/
uint32_t E32ImportBlock::Size(uint32_t aImpFmt) const
{
	uint32_t r = sizeof(E32ImportBlock);
    if(aImpFmt!=KImageImpFmt_PE2)
        r += iNumberOfImports * sizeof(uint32_t);
    return r;
}

