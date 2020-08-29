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
// Convenient functions to compress E32Image.
//
//

#include <ios>
#include "e32common.h"
#include "e32compressor.h"

std::vector<char> CompressBPE(std::vector<char> source)
{
    E32ImageHeader* h = (E32ImageHeader*)&source[0];
    uint32_t offset = h->iCodeOffset;
    std::streamsize iFileSize = source.size();
    const char* compressed = new char[iFileSize * 2]();

    int32_t BPECodeSize = CompressBPE(&source[offset], h->iCodeSize, compressed + offset, iFileSize - offset);
    int32_t srcOffset = offset + h->iCodeSize;
    int32_t BPEDataSize = CompressBPE(nullptr, iFileSize - srcOffset,
                                  nullptr, iFileSize - BPECodeSize);
    uint32_t compressedSize = BPECodeSize + BPEDataSize;

    source.erase(source.begin() + offset, source.end());
    source.insert(source.end(), compressed, compressed + compressedSize);
    return source;
}

std::vector<char> CompressDeflate(std::vector<char> source)
{
    E32ImageHeader* h = (E32ImageHeader*)&source[0];
    uint32_t offset = h->iCodeOffset;
    std::streamsize iFileSize = source.size();
    const char* compressed = new char[iFileSize * 2]();

    uint32_t compressedSize = CompressDeflate(&source[offset], iFileSize - offset, compressed + offset, iFileSize * 2 - offset);

    source.erase(source.begin() + offset, source.end());
    source.insert(source.end(), compressed, compressed + compressedSize);
    return source;
}
