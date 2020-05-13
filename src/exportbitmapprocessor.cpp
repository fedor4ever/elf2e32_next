#include <assert.h>
#include "e32common.h"
#include "exportbitmapprocessor.h"

ExportBitmapProcessor::ExportBitmapProcessor(uint32_t exportsCount, const E32SectionUnit& exportTable, uint32_t absentVal):
    iExportsCount(exportsCount), iExportTable(exportTable), iAbsentVal(absentVal) {}

ExportBitmapProcessor::~ExportBitmapProcessor()
{
    //dtor
}

E32Section ExportBitmapProcessor::CreateExportBitmap()
{
    E32Section exportBitMap;
    exportBitMap.info = "EXPORTS";
    exportBitMap.type = E32Sections::EXPORTS;

    size_t memsz = (iExportsCount + 7) >> 3;  // size of complete bitmap
	size_t mbs = (memsz + 7) >> 3;	// size of meta-bitmap

    exportBitMap.section.insert(exportBitMap.section.begin(), memsz, 0xff);
    uint32_t* exports = ((uint32_t*)iExportTable.data()) + 1;

    for(int i = 0; i < iExportsCount; ++i)
    {
        if (exports[i] == iAbsentVal)
        {
            exportBitMap.section[i>>3] &= ~(1u << (i & 7));
            iMissingExports++;
        }
    }

    if(iMissingExports == 0)
    {
        exportBitMap.section.clear();
        exportBitMap.section.push_back(0);
        iExportDescType = 0;
        iExportDescSize = 0;
		return exportBitMap;
    }

    size_t nbytes = 0;
	for(uint32_t i = 0; i < memsz; i++) // check why used ++i???
    {
		if (exportBitMap.section[i] != 0xff) ++nbytes; // number of groups of 8
	}

	assert(memsz > 65536);
    assert(((mbs + nbytes) > 65536));

	iExportDescType = KImageHdr_ExpD_FullBitmap;
	iExportDescSize = memsz;

	if(mbs + nbytes >= memsz)
        return exportBitMap;

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
        if (exportBitMap.section[i] != 0xff)
        {
            mptr[i>>3] |= (1u << (i&7));
            *gptr++ = exportBitMap.section[i];
        }
    }

    exportBitMap.section.clear();
    exportBitMap.section.swap(tmp);

    return exportBitMap;
}

uint8_t ExportBitmapProcessor::ExportDescType() const
{
    return iExportDescType;
}

uint16_t ExportBitmapProcessor::ExportDescSize() const
{
    return iExportDescSize;
}

