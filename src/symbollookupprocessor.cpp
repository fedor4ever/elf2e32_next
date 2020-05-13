#include <assert.h>
#include "symbol.h"
#include "e32common.h"
#include "symbollookupprocessor.h"

SymbolLookupProcessor::SymbolLookupProcessor(const Symbols& s): iSymbols(s)
    {}

SymbolLookupProcessor::~SymbolLookupProcessor()
{
    //dtor
}

/** TODO (Administrator#1#04/15/17): The nullptr iElfSym position corresponds to the Absent function in def file
 but now it initalized, should we ignore absent symbols?*/
const char pad[] = {'\0', '\0', '\0', '\0'};
void SymbolLookupProcessor::ProcessSymbols()
{
    for(auto x: iSymbols)
    {
        iSymAddrTab.push_back(x->Elf_st_value());
        // The symbol names always start at a 4-byte aligned offset.
        iSymNameOffset = iSymbolNames.size() >> 2;
		iSymNameOffTab.push_back(iSymNameOffset);

		iSymbolNames += x->AliasName();
		iSymbolNames += '\0';
		uint32_t align = sizeof(uint32_t) - iSymbolNames.size()%sizeof(uint32_t);
        iSymbolNames.append(pad, align);
    }
}

E32Section SymbolLookupProcessor::SymlookSection()
{
    E32Section data;
    data.info = "SYMLOOK";
    data.type = E32Sections::SYMLOOK;
    E32EpocExpSymInfoHdr s;
    data.section.assign((char*)&s , (char*)&s + sizeof(s));
    E32EpocExpSymInfoHdr* aSymInf = (E32EpocExpSymInfoHdr*)data.section.data();
    InitHeader(*aSymInf);
    data.section.insert(data.section.end(), (char*)iSymAddrTab.data(), (char*)iSymAddrTab.data() + iSymAddrTab.size() * sizeof(uint32_t));

    uint32_t aOffLen = 2;
	if(aSymInf->iFlags & 1)
		aOffLen=4;

    uint32_t align = sizeof(uint32_t) - data.section.size()%sizeof(uint32_t);
    data.section.insert(data.section.end(), 0, align);
    data.section.insert(data.section.end(), (char*)iSymNameOffTab.data(), (char*)iSymNameOffTab.data() + iSymNameOffTab.size() * aOffLen);

    align = sizeof(uint32_t) - data.section.size()%sizeof(uint32_t);
    data.section.insert(data.section.end(), 0, align);
    data.section.insert(data.section.end(), iSymbolNames.begin(), iSymbolNames.end());
    return data;
}

void SymbolLookupProcessor::InitHeader(E32EpocExpSymInfoHdr& s)
{
    uint32_t offset = sizeof(E32EpocExpSymInfoHdr);
	assert(offset == s.iSymbolTblOffset);

	uint16_t nSymbols = (uint16_t)iSymAddrTab.size();
	s.iSymCount = nSymbols;
	offset += nSymbols * sizeof(uint32_t); // Symbol addresses

	uint32_t sizeofNames = sizeof(uint32_t);
    s.iFlags |= 1;//set the 0th bit
	if(iSymNameOffset < 0xffff) { // 		iSymNameOffset = iSymbolNames.size() >> 2;
		sizeofNames = sizeof(uint16_t);
		s.iFlags &= ~1;//reset the 0th bit
	}

//	uint32_t align = sizeof(uint32_t) - data.size()%sizeof(uint32_t);
//	offset += Align((nSymbols * sizeofNames), sizeof(uint32_t)); // Symbol name offsets
	s.iStringTableOffset = offset;

	s.iStringTableSz = iSymbolNames.size();
	offset += s.iStringTableSz; // Symbol names in string tab

//!!! init lateriSymNameOffTab
//	s.iDepDllZeroOrdTableOffset = offset;
//	s.iDllCount = iNumDlls;
//
//	offset += iNumDlls * sizeof(uint32_t); // Dependency list - ordinal zero placeholder
//	s.iSize = offset;
}
