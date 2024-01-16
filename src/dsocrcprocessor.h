#ifndef DSOCRCProcessor_H
#define DSOCRCProcessor_H

#include <string>

#include "crcprocessor.h"

class Args;
class ElfParser;

struct CRCData
{
    uint32_t iDSOCrc = -1;
};

class DSOCRCProcessor: public CRCProcessor
{
    public:
        DSOCRCProcessor(const Args* arg);
        virtual ~DSOCRCProcessor();
    public:
        virtual void CRCFromFile() final override;
        virtual void SetCRCFiles() final override;
        virtual std::string CRCAsStr() final override;
        virtual void CRCFile(std::string& s) final override;
        virtual bool PrintInvalidTargetCRC() final override;
        virtual void ProcessTokens(const std::string&, uint32_t crc) final override;
    private:
        const ElfParser* iParser = nullptr;
        CRCData iCRCIn;
        CRCData iCRCOut;

};

#endif // DSOCRCProcessor_H
