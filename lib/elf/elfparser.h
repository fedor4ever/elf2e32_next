// Copyright (c) 2020 Strizhniou Fiodar
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
// Parse ELF Images
//
//

#ifndef ELFPARSER_H
#define ELFPARSER_H

#include <string>

#include "elfdefs.h"

class ElfParser
{
    public:
        ElfParser(std::string elf);
        void GetElfFileLayout();
        ~ElfParser();
        bool IsRelocationFixRequired();
        uint32_t CodeSegmentSize();
        uint32_t DataSegmentSize();
        uint32_t ImportsCount() const;
        const char* GetSymbolNameFromStringTable(uint32_t index) const;
        Elf32_Sym* GetSymbolTableEntity(uint32_t index) const;
        Elf32_Phdr* GetSegmentAtAddr(Elf32_Addr addr) const;
        ESegmentType SegmentType(Elf32_Addr addr) const;
    private:
        void ValidateElfImage();
        void ProcessSectionHeaders();
        void ProcessProgHeaders();
        void ProcessDynamicTable();
    private:
        std::string iFile;
        const char* iFileBuf = nullptr;
        std::streamsize iFileBufSize = 0;
    private:
        /** Elf header */
        Elf32_Ehdr* iElfHeader = nullptr;

        /** Section header table. */
        Elf32_Shdr* iSections = nullptr;
        /** Section header names table. */
        char* iSectionHdrStrTbl = nullptr;
    private:
        /** The static symbol table.*/
        Elf32_Sym* iSymTab = nullptr;
        char* iStrTab = nullptr;
        Elf32_Sym* iLim = nullptr;
    private:
        const char* iCommentSection = nullptr;
    private:
        /** The dynamic table*/
        Elf32_HashTable* iHashTbl = nullptr;
    private:
        /** Program headers table*/
        Elf32_Phdr* iProgHeader = nullptr;
        Elf32_Phdr* iDynSegmentHdr = nullptr;

        Elf32_Phdr* iCodeSegmentHdr = nullptr;
        uint32_t iCodeSegmentIdx = 0;
        const char* iCodeSegment = nullptr;

        Elf32_Phdr* iDataSegmentHdr = nullptr;
        uint32_t iDataSegmentIdx = 0;
        const char* iDataSegment = nullptr;
        	/** The dynamic symbol array.*/
        Elf32_Sym* iElfDynSym = nullptr;//The ELF symbol

        //others
        char* iStringTable = nullptr;
};



#endif // ELFPARSER_H
