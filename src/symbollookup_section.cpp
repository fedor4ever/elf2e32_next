// Copyright (c) 2024 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Contributors:
//
// Description:
// This class creates symbol lookup section
//
//

#include <assert.h>
#include "symbol.h"
#include "e32common.h"
#include "symbollookup_section.h"

template <class T>
inline T Align(T v)
{
	unsigned int inc = sizeof(uint32_t)-1;
	unsigned int res = ((uint32_t)v+inc) & ~inc;
	return (T)res;
}

SymbolLookupSection::SymbolLookupSection(const Symbols& s, uint32_t dllCount):
            iSymbols(s), iDllCount(dllCount){}

const char pad[] = {'\0', '\0', '\0', '\0'};
void SymbolLookupSection::ProcessSymbols()
{
    for(auto x: iSymbols)
    {
        if(x->Absent())
            continue;
        iSymAddrTab.push_back(x->Elf_st_value());
        // The symbol names always start at a 4-byte aligned offset.
        iSymNameOffset = iSymbolNames.size() >> 2;
		iSymNameOffTab.push_back(iSymNameOffset);

		iSymbolNames += x->AliasName();
		iSymbolNames += '\0';

		uint32_t align = Align(iSymbolNames.size());
		align -= iSymbolNames.size();
		if(align % 4)
            iSymbolNames.append(pad, align);
    }
}

E32Section SymbolLookupSection::SymlookSection()
{
    E32Section data;
    data.info = "SYMLOOK";
    data.type = E32Sections::SYMLOOK;

    ProcessSymbols();
    E32EpocExpSymInfoHdr s;
    InitHeader(s);
    data.section.assign((char*)&s , (char*)&s + sizeof(s));

    data.section.insert(data.section.end(), (char*)iSymAddrTab.data(),
                        (char*)iSymAddrTab.data() + iSymAddrTab.size() * sizeof(uint32_t));

    E32EpocExpSymInfoHdr* symInf = (E32EpocExpSymInfoHdr*)&data.section[0];
    uint32_t aOffLen = 2;
	if(symInf->iFlags & 1)
    {
		aOffLen=4;
        data.section.insert(data.section.end(), (char*)iSymNameOffTab.data(),
                            (char*)iSymNameOffTab.data() + iSymNameOffTab.size() * aOffLen);
    }
    else
    {
        std::vector<uint16_t> d;
        d.insert(d.end(), iSymNameOffTab.begin(), iSymNameOffTab.end());
        if(d.size() % 4)
            d.push_back(0);
        data.section.insert(data.section.end(), (char*)d.data(), (char*)d.data() + d.size() * aOffLen);
    }

    symInf = (E32EpocExpSymInfoHdr*)&data.section[0];
    symInf->iStringTableOffset = data.section.size();
    data.section.insert(data.section.end(), iSymbolNames.begin(), iSymbolNames.end());

    symInf = (E32EpocExpSymInfoHdr*)&data.section[0];
    symInf->iDepDllZeroOrdTableOffset = data.section.size();

    data.section.insert(data.section.end(), symInf->iDllCount * sizeof(uint32_t), 0);
    symInf = (E32EpocExpSymInfoHdr*)&data.section[0];
    symInf->iSize = data.section.size();
    return data;
}

void SymbolLookupSection::InitHeader(E32EpocExpSymInfoHdr& s)
{
    int32_t offset = sizeof(E32EpocExpSymInfoHdr);
	assert(offset == s.iSymbolTblOffset);

	s.iSymCount = (uint16_t)iSymAddrTab.size();
	s.iStringTableSz = iSymbolNames.size();
	s.iDllCount = iDllCount;

    s.iFlags |= 1;//set the 0th bit
	if(iSymNameOffset < 0xffff) // 		iSymNameOffset = iSymbolNames.size() >> 2;
        s.iFlags &= ~1;//reset the 0th bit
}
