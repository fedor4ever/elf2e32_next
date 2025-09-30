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
#include "common.hpp"
#include "e32common.h"
#include "e32compressor.h"

typedef std::vector<char> E32Buf;
int32_t Adjust(int32_t size);

E32Buf DeCompressE32Image(const E32Buf& source)
{
    E32ImageHeader* h = (E32ImageHeader*)&source[0];
    if(h->iCompressionType == KFormatNotCompressed)
        return source;

    E32Buf buf;
    size_t pos = sizeof(E32ImageHeader);
    E32ImageHeaderJ* iHdrJ = (E32ImageHeaderJ*)&source[pos];

    const uint32_t offset = h->iCodeOffset;
    const uint32_t extracted = iHdrJ->iUncompressedSize;
    size_t e32Size = Adjust(extracted + offset);

    if(e32Size != (extracted + offset))
        ReportError(ErrorCodes::WRONGFILESIZEFORDECOMPRESSION,
            extracted + offset, e32Size);
    buf.reserve(e32Size);
    buf.insert(buf.end(), source.begin(), source.begin() + offset);
    buf.insert(buf.end(), e32Size, '0');

    const uint32_t compr = h->iCompressionType;


    if(h->iCompressionType == KUidCompressionBytePair)
    {
        uint32_t uncompressedCodeSize = DecompressBPE(&source[offset], &buf[offset]);
        uint32_t uncompressedDataSize = DecompressBPE(nullptr, &buf[offset + uncompressedCodeSize]);
        if((uncompressedCodeSize + uncompressedDataSize) != iHdrJ->iUncompressedSize)
            ReportWarning(ErrorCodes::BYTEPAIRINCONSISTENTSIZE);
    }else if(h->iCompressionType == KUidCompressionDeflate)
    {
        DeCompressInflate((unsigned char*)&source[offset], source.size() - offset, (unsigned char*)&buf[offset], extracted);
    }else
        ReportError(ErrorCodes::UNKNOWNCOMPRESSION);
    return buf;
}

// Compress if needed or return source
E32Buf CompressE32Image(const E32Buf& source)
{
    E32ImageHeader* h = (E32ImageHeader*)&source[0];
    uint32_t compressedSize = 0;
    uint32_t offset = h->iCodeOffset;
    E32Buf compressed(source);

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

int32_t Adjust(int32_t size)
{
    return ((size+0x3)&0xfffffffc);
}
