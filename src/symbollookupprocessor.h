#ifndef SYMBOLLOOKUPPROCESSOR_H
#define SYMBOLLOOKUPPROCESSOR_H

#include <string>
#include <vector>
#include "e32file.h"

struct E32EpocExpSymInfoHdr;

class SymbolLookupProcessor
{
    public:
        SymbolLookupProcessor(const Symbols& s);
        ~SymbolLookupProcessor();
        E32Section SymlookSection();
    private:
        void ProcessSymbols();
        void InitHeader(E32EpocExpSymInfoHdr& s);
    private:
        Symbols iSymbols;
        std::vector<uint32_t> iSymAddrTab;
        std::vector<uint32_t> iSymNameOffTab;
        std::string  iSymbolNames;
        uint32_t     iSymNameOffset=0;
};

#endif // SYMBOLLOOKUPPROCESSOR_H
