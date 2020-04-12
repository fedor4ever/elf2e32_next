#include <assert.h>
#include "e32common.h"
#include "exportbitmapprocessor.h"

ExportBitmapProcessor::ExportBitmapProcessor(uint32_t exportsCount, const E32SectionUnit& exportTable, uint32_t absentVal):
    iExportsCount(exportsCount), iExportTable(exportTable), iAbsentVal(absentVal) {}

ExportBitmapProcessor::~ExportBitmapProcessor()
{
    //dtor
}

E32SectionUnit ExportBitmapProcessor::CreateExportBitmap()
{
    size_t memsz = (iExportsCount + 7) >> 3;  // size of complete bitmap
	size_t mbs = (memsz + 7) >> 3;	// size of meta-bitmap

    iExportBitMap.insert(iExportBitMap.begin(), memsz, 0xff);
    uint32_t* exports = ((uint32_t*)iExportTable.data()) + 1;

    for(int i = 0; i < iExportsCount; ++i)
    {
        if (exports[i] == iAbsentVal)
        {
            iExportBitMap[i>>3] &= ~(1u << (i & 7));
            iMissingExports++;
        }
    }

    if(iMissingExports == 0)
    {
        iExportBitMap.clear();
		return iExportBitMap;
    }

    size_t nbytes = 0;
	for(uint32_t i = 0; i < memsz; i++) // check why used ++i???
    {
		if (iExportBitMap[i] != 0xff) ++nbytes; // number of groups of 8
	}

	assert(memsz > 65536);
    assert(((mbs + nbytes) > 65536));

	iExportDescType = KImageHdr_ExpD_FullBitmap;
	iExportDescSize = memsz;

	if(mbs + nbytes >= memsz)
        return iExportBitMap;

    uint32_t extra_space = mbs + nbytes;
    extra_space = (extra_space + sizeof(uint32_t) - 1) &~ (sizeof(uint32_t) - 1);
    const char* aDesc = new char[extra_space]();

    iExportDescType = KImageHdr_ExpD_SparseBitmap8;
    iExportDescSize = mbs + nbytes;

    E32SectionUnit tmp;
    tmp.insert(tmp.begin(), memsz, 0);
    char* mptr = tmp.data();
    char* gptr = mptr + mbs;
    for (uint32_t i = 0; i < memsz; i++)  // check why used ++i???
    {
        if (iExportBitMap[i] != 0xff)
        {
            mptr[i>>3] |= (1u << (i&7));
            *gptr++ = iExportBitMap[i];
        }
    }

    iExportBitMap.clear();
    iExportBitMap.swap(tmp);

    return iExportBitMap;
}

uint8_t ExportBitmapProcessor::ExportDescType() const
{
    return iExportDescType;
}

uint16_t ExportBitmapProcessor::ExportDescSize() const
{
    return iExportDescSize;
}

