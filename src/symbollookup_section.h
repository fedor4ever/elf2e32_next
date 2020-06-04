#ifndef SYMBOLLOOKUPPROCESSOR_H
#define SYMBOLLOOKUPPROCESSOR_H

#include <string>
#include <vector>
#include "e32file.h"

struct E32EpocExpSymInfoHdr;

class SymbolLookupSection
{
    public:
        SymbolLookupSection(const Symbols& s, uint32_t dllCount);
        ~SymbolLookupSection(){}
        E32Section SymlookSection();
    private:
        void ProcessSymbols();
        void InitHeader(E32EpocExpSymInfoHdr& s);
    private:
        Symbols iSymbols;
        std::vector<uint32_t> iSymAddrTab;
        std::vector<uint32_t> iSymNameOffTab;
        std::string  iSymbolNames;
        uint32_t     iSymNameOffset = 0;
        uint32_t     iDllCount = 0;
};

#endif // SYMBOLLOOKUPPROCESSOR_H
