#include <vector>
#include <cstdint>

#include "byte_pair.h"
#include "e32compressor.h"

using std::vector;

#define PAGE_SIZE 4096
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
        BPEBlock = src;

	const IndexTableHeader* h = (IndexTableHeader*)BPEBlock;
	vector<uint16_t> sizeOfCompressedPageData;
	sizeOfCompressedPageData.reserve(h->iNumberOfPages);

//	iterate over Page index table. Every element contain compressed page size
	const uint16_t* pageIndexTable = (uint16_t*)(BPEBlock + sizeof(IndexTableHeader));
	for(int i = 0; i < h->iNumberOfPages; i++)
	{
		sizeOfCompressedPageData.push_back(*pageIndexTable++);
	}

	const uint8_t* pages = (const uint8_t*)pageIndexTable;
	uint32_t sz = 0;
	uint8_t* p;
	for(int i = 0; i < h->iNumberOfPages; i++)
	{
		sz += Unpak( (dst + i * PAGE_SIZE), (uint8_t*)pages, sizeOfCompressedPageData.at(i), p);
		pages = pages + sizeOfCompressedPageData.at(i);
	}
    BPEBlock = BPEBlock + h->iSizeOfData;

	return sz;
}

static uint8_t* inBlock = nullptr;
static uint8_t* outBlock = nullptr;
uint32_t CompressBPE(const char* src, const uint32_t srcSize, char* dst, uint32_t dstSize)
{
    if(src)
        inBlock = src;
    if(dst)
        outBlock = dst;

    if(!inBlock && !outBlock)
        return 0;
    if(!srcSize || !dstSize)
        return 0;

    uint16_t numOfPages = (uint16_t)((srcSize + PAGE_SIZE - 1) / PAGE_SIZE);

    IndexTableHeader* indexHdr = (IndexTableHeader*)outBlock;
    uint16_t* pageIndexTable = (uint16_t*)(outBlock + sizeof(IndexTableHeader));
    uint8_t* pagesOut = (const uint8_t*)(pageIndexTable + numOfPages);

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

