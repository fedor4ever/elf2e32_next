#ifndef E32VALIDATOR_H
#define E32VALIDATOR_H

#include <cstdint>

class E32Parser;
struct E32ImageHeader;
struct E32ImageHeaderV;

void ValidateE32Image(const E32Parser* parser);

class E32Validator
{
    public:
        E32Validator(const E32Parser* parser);
        ~E32Validator();
        void ValidateE32Image();

    private:
        void ValidateHeader();
        void ValidateRelocations(uint32_t offset, uint32_t sectionSize);
        void ValidateImports() const;
        void ValidateExportDescription() const;

    private:
        const E32Parser* iParser = nullptr;
        const E32ImageHeader *iHdr = nullptr;
        const E32ImageHeaderV *iHdrV = nullptr;
        uint32_t iBufSize = 0;
        uint32_t iPointerAlignMask = 0;
};

#endif // E32VALIDATOR_H
