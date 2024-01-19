#ifndef CRCPROCESSOR_H
#define CRCPROCESSOR_H
#include <string>

struct Args;

void PrintIfNEQ(uint32_t in, uint32_t out, const std::string& msg);


class CRCProcessor
{
    public:
        CRCProcessor(const Args* arg);
        virtual ~CRCProcessor();
        virtual void Run();
    private:
        virtual void SetCRCFiles() = 0;
        virtual void CRCFromFile() = 0;
        virtual std::string CRCAsStr() = 0;
        virtual void CRCFile(std::string& s) = 0;
        virtual bool PrintInvalidTargetCRC() = 0;
        virtual void ProcessTokens(const std::string& type, uint32_t crc) = 0;
    private:
        void DeduceCRCFiles();
        void ParseFile();
        void CRCToFile();
        void PrintInvalidCRCs();
        void Tokenize(const std::string& line);
    protected:
        void ReadOrCreateCRCFile(const std::string& file);
    protected:
        const Args* iArgs = nullptr;
        std::string iFileIn;
        std::string iFileOut;
};

#endif // CRCPROCESSOR_H
