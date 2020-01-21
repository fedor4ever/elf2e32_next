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

