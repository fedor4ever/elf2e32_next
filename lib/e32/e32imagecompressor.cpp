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
// (De)compression routines for the elf2e32 tool
//
//

#include "common.hpp"
#include "e32imagecompressor.h"

//param dstsize - after first function call hold offset to data part of the image.
//So used for Bytepair compression only.
char* Decompress(const char* src, uint32_t srcsize,
    uint32_t& dstsize, uint32_t algorythm)
{
//    if(dstsize != extractedsize) ReportWarning(HUFFMANINCONSISTENTSIZE)
    return nullptr;
}

char* Compress(const char* src, uint32_t srcsize,
    uint32_t algorythm)
{
//    if(dstsize != extractedsize) ReportError(HUFFMANINCONSISTENTSIZE)
    return nullptr;
}
