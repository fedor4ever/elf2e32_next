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
// functions to (de)compress E32Images
//
//

#ifndef E32COMPRESSOR_H_INCLUDED
#define E32COMPRESSOR_H_INCLUDED

//! set input buffer as nullptr to decompress next block
uint32_t DecompressBPE(const char* src, char* dst);
//! set input and output buffers as nullptr to decompress next block
uint32_t CompressBPE(const char* src, uint32_t srcSize, char* dst, uint32_t dstSize);

void DeCompressInflate(unsigned char* source, int sourcesize,unsigned char* dest, int destsize);
uint32_t CompressDeflate(unsigned char* source, int sourcesize,unsigned char* dest, int destsize);

#endif // E32COMPRESSOR_H_INCLUDED
