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

#ifndef E32IMAGECOMPRESSOR_H
#define E32IMAGECOMPRESSOR_H


char* Decompress(const char* src, uint32_t srcsize,
        uint32_t& dstsize, uint32_t cmpalgorythm);

char* Compress(const char* src, uint32_t srcsize,
        uint32_t cmpalgorythm);

#endif // E32IMAGECOMPRESSOR_H
