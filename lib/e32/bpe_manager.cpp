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
// functions to (de)compress E32Images with bytepair algorithm
//
//

#include <cstdint>

#include "byte_pair.h"
#include "e32compressor.h"

#define PAGE_SIZE 4096

/********************************************//**
 *  ... E32Image compressed with bytepair consists of 3 parts
 Header part - uncompressed, has size with E32ImageHeader::iCodeOffset
 Code part - compressed
 Data part - compressed

 Every compressed section begins with IndexTableHeader
 Size each of compressed section stored in IndexTableHeader::iSizeOfData
 Size each of decompressed section stored in IndexTableHeader::iDecompressedSize
 IndexTableHeader::iNumberOfPages holds number of pages in this block
 Pages means memory page, or virtual page with size limit 4096 bytes

 After IndexTableHeader begins Page index table as uint16_t array with
 size IndexTableHeader::iNumberOfPages.
 This table holds compressed size every page.

 Page table starts after Page index table.
 It has size calculate by formula:
   IndexTableHeader::iSizeOfData - sizeof(IndexTableHeader) - IndexTableHeader::iNumberOfPages * sizeof(uint16_t)
 Every page compressed separately. It size after compression stored in Page index table
 ***********************************************/
#pragma pack(push, 1)
struct IndexTableHeader
{
	int32_t	iSizeOfData;		// Includes the index and compressed pages
	int32_t	iDecompressedSize;
	uint16_t   iNumberOfPages;
};
#pragma pack(pop)

static uint8_t* BPEBlock = nullptr;
uint32_t DecompressBPE(const char* src, char* dst)
{
    if(!BPEBlock && !src)
        return 0;
    if(src)
        BPEBlock = (uint8_t*)src;

	const IndexTableHeader* h = (IndexTableHeader*)BPEBlock;

	uint32_t sz = 0;
	uint8_t* p;
	const uint16_t* pageIndexTable = (const uint16_t*)(BPEBlock + sizeof(IndexTableHeader));
	uint8_t* pages = (uint8_t*)(pageIndexTable + h->iNumberOfPages);

//	iterate over Page index table and pages.
// Every element contain compressed page size
	for(int i = 0; i < h->iNumberOfPages; i++)
	{
		sz += Unpak( ((uint8_t*)dst + i * PAGE_SIZE), pages, *pageIndexTable, p);
		pages = pages + *pageIndexTable++;
	}
    BPEBlock = BPEBlock + h->iSizeOfData;

	return sz;
}

static uint8_t* inBlock = nullptr;
static uint8_t* outBlock = nullptr;
uint32_t CompressBPE(const char* src, const uint32_t srcSize, char* dst, uint32_t dstSize)
{
    if(src)
        inBlock = (uint8_t*)src;
    if(dst)
        outBlock = (uint8_t*)dst;

    if(!inBlock && !outBlock)
        return 0;
    if(!srcSize || !dstSize)
        return 0;

    uint16_t numOfPages = (uint16_t)((srcSize + PAGE_SIZE - 1) / PAGE_SIZE);

    IndexTableHeader* indexHdr = (IndexTableHeader*)outBlock;
    uint16_t* pageIndexTable = (uint16_t*)(outBlock + sizeof(IndexTableHeader));
    uint8_t* pagesOut = (uint8_t*)(pageIndexTable + numOfPages);

    indexHdr->iNumberOfPages = numOfPages;
    indexHdr->iDecompressedSize = srcSize;
    indexHdr->iSizeOfData = sizeof(IndexTableHeader) + numOfPages * sizeof(uint16_t);

    uint32_t remain = srcSize;
    for(uint32_t i = 0; i < numOfPages; i++)
    {
        uint32_t offset = remain > PAGE_SIZE ? PAGE_SIZE : remain;
        uint16_t compressedSize = (uint16_t)Pak(pagesOut, inBlock, offset);
        pageIndexTable[i] = compressedSize;

        indexHdr->iSizeOfData += compressedSize;
        pagesOut += compressedSize;
        inBlock += offset;
        remain -= offset;
    }
    outBlock = pagesOut;

    return indexHdr->iSizeOfData;
}

