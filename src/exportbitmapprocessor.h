#ifndef EXPORTBITMAPPROCESSOR_H
#define EXPORTBITMAPPROCESSOR_H

#include "e32file.h"

class ExportBitmapProcessor
{
    public:
        ExportBitmapProcessor(uint32_t exportsCount, const E32SectionUnit& exportTable, uint32_t absentVal);
        ~ExportBitmapProcessor();

        E32Section CreateExportBitmap();
        uint8_t ExportDescType() const;
        uint16_t ExportDescSize() const;

    private:
        uint32_t iExportsCount = 0;
        const E32SectionUnit& iExportTable;
        uint32_t iAbsentVal = 0;
    private:
        uint16_t iExportDescSize = 0;
        uint8_t iExportDescType = 0;
};

#endif // EXPORTBITMAPPROCESSOR_H
