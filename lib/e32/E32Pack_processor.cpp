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
// Convenient functions to compress E32Image.
//
//

#include <ios>
#include "e32common.h"
#include "e32compressor.h"

// Compress if needed or return source
std::vector<char> CompressE32Image(std::vector<char> source)
{
    E32ImageHeader* h = (E32ImageHeader*)&source[0];
    uint32_t compressedSize = 0;
    uint32_t offset = h->iCodeOffset;
    std::vector<char> compressed(source);

    if(h->iCompressionType == KUidCompressionBytePair)
    {
        int32_t BPECodeSize = CompressBPE(&source[offset], h->iCodeSize, &compressed[offset], source.size() - offset);
        int32_t srcOffset = offset + h->iCodeSize;
        int32_t BPEDataSize = CompressBPE(nullptr, source.size() - srcOffset,
                                          nullptr, source.size() - BPECodeSize);

        compressedSize = BPECodeSize + BPEDataSize;
    }else if(h->iCompressionType == KUidCompressionDeflate)
    {
        compressedSize = CompressDeflate(&source[offset], source.size() - offset, &compressed[offset], source.size() - offset);
    }else if(h->iCompressionType == KFormatNotCompressed)
        return source;

    compressed.resize(offset + compressedSize);
    return compressed;
}
