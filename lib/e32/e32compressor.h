// Copyright (c) 2020-2024 Strizhniou Fiodar
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

#include <vector>

//! set input buffer as nullptr to decompress next block
uint32_t DecompressBPE(const char* src, char* dst);
//! set input and output buffers as nullptr to decompress next block
uint32_t CompressBPE(const char* src, uint32_t srcSize, char* dst, uint32_t dstSize);
std::vector<char> CompressBPE(std::vector<char> src);

void DeCompressInflate(unsigned char* source, int sourcesize, unsigned char* dst, int destsize);
uint32_t CompressDeflate(const char* source, int sourcesize, const char* dst, int destsize);
std::vector<char> CompressDeflate(std::vector<char> source);
std::vector<char> CompressE32Image(std::vector<char> source);

#endif // E32COMPRESSOR_H_INCLUDED
