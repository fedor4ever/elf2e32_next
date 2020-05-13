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
// Create E32Image.
//
//

#include "symbol.h"
#include "e32file.h"
#include "argparser.h"
#include "elfparser.h"
#include "e32common.h"
#include "elf2e32_opt.hpp"
#include "e32headerbuilder.h"
#include "exportbitmapprocessor.h"
#include "symbollookupprocessor.h"

E32Section ExportTable(const Symbols& s);
E32Section CodeSection(const ElfParser* parser);
E32Section DataSection(const ElfParser* parser);

bool CmpSections(E32Section first, E32Section second)
{
    return first.type < second.type;
}

E32File::E32File(const Args* args, const ElfParser* elfParser, const Symbols& s):
    iE32Opts(args), iElfSrc(elfParser), iSymbols(s) {}

E32File::~E32File()
{
    //dtor
}

void E32File::WriteE32File()
{
    E32HeaderBuilder header(iE32Opts);
    iHeader = header.MakeE32Header();

    printf("header size: %u\n", iHeader.size());

    E32ImageHeader* hdr = (E32ImageHeader*)iHeader.data();
    hdr->iDataSize = iElfSrc->DataSegmentSize();
    hdr->iBssSize = iElfSrc->BssSegmentSize();
    hdr->iEntryPoint = iElfSrc->EntryPointOffset();
    hdr->iCodeBase = iElfSrc->ROVirtualAddress();
    hdr->iDataBase = iElfSrc->RWVirtualAddress();
    hdr->iDllRefTableCount; // = iNumDlls;   // filling this in enables E32ROM to leave space for it
    hdr->iExportDirCount = iSymbols.size();

    const uint32_t offset = sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ);
//    E32ImageHeaderV* hdrv = (E32ImageHeaderV*)iHeader[offset];
    E32ImageHeaderV* hdrv = (E32ImageHeaderV*)(iHeader.data() + offset);

    hdrv->iExceptionDescriptor = iElfSrc->ExceptionDescriptor();

    PrepareData();

    iE32image.sort([](auto A, auto B){return A.type < B.type;});
    for(auto x: iE32image)
    {
        printf("section type %u has size %u\n", x.type, x.section.size());
        printf("current E32Image size: 0x%x\n", iHeader.size());
        switch(x.type)
        {
        case E32Sections::HEADER:
            break;
        case E32Sections::BITMAP:
            hdrv->iExportDescSize = this->iExportDescSize;
            hdrv->iExportDescType = this->iExportDescType;
            break;
        case E32Sections::EXPORTS:
            hdr->iExportDirOffset = iHeader.size();
        case E32Sections::CODE:
            hdr->iCodeOffset = iHeader.size();
            hdr->iTextSize = hdr->iCodeSize = iHeader.size() + x.section.size() - hdr->iCodeOffset;
            break;
        case E32Sections::SYMLOOK:
            {
            hdr->iTextSize = hdr->iCodeSize = iHeader.size() + x.section.size() - hdr->iCodeOffset;
            E32EpocExpSymInfoHdr* symInf = (E32EpocExpSymInfoHdr*)x.section.data();
            uint32_t offset = symInf->iDepDllZeroOrdTableOffset;
//            symInf->iDllCount = iNumDlls;

//            offset += iNumDlls * sizeof(uint32_t); // Dependency list - ordinal zero placeholder
            symInf->iSize = offset;
            }
            break;
        case E32Sections::DATA:
            hdr->iDataOffset = iHeader.size();
            break;
        case E32Sections::IMPORTS:
            hdr->iImportOffset = iHeader.size();
            break;
        case E32Sections::CODERELOCKS:
            hdr->iCodeRelocOffset = iHeader.size();
            break;
        case E32Sections::DATARELOCKS:
            hdr->iDataRelocOffset = iHeader.size();
            break;
        default:
            ReportError(ErrorCodes::UNKNOWNSECTION);
            break;
        }

        iHeader.insert(iHeader.end(), x.section.begin(), x.section.end());
        hdr = (E32ImageHeader*)&iHeader[0];
        hdrv = (E32ImageHeaderV*)&iHeader[offset];
    }
    SetE32ImageCrc(iHeader.data());
    // see E32Rebuilder::ReCompress()
    SaveFile(iE32Opts->iOutput.c_str(), iHeader.data(), iHeader.size());
}

E32Section MakeExportSection(const Symbols& s)
{
    E32Section exports;
    if(s.empty())
        return exports;

    exports.info = "EXPORTS";
    exports.type = E32Sections::EXPORTS;
    uint32_t sz = s.size() + 1;
    exports.section.insert(exports.section.begin(), sz * sizeof(uint32_t), 0);

    uint32_t* iTable = (uint32_t*)exports.section.data();
    iTable[0] = sz;

    uint32_t i = 1;
    for(auto x: s)
    {
//        printf("%s\n", x->AliasName().c_str());
        iTable[i++] = x->Elf_st_value();
    }
    return exports;
}

E32Section CodeSection(const ElfParser* parser)
{
    E32Section code;
    code.info = "CODE";
    code.type = E32Sections::CODE;
    code.section.insert(code.section.begin(), parser->CodeSegment(), parser->CodeSegment() + parser->CodeSegmentSize());
    return code;
}

E32Section DataSection(const ElfParser* parser)
{
    //iElfImage->GetRawRWSegment(), iElfImage->GetRWSize()
    E32Section data;
    data.info = "DATA";
    data.type = E32Sections::DATA;
    data.section.insert(data.section.begin(), parser->CodeSegment(), parser->CodeSegment() + parser->DataSegmentSize());
    return data;
}

void E32File::PrepareData()
{
    E32Section tmp;
    printf("EntryPointOffset: 0x%x\n", iElfSrc->EntryPointOffset());

    tmp = MakeExportSection(iSymbols);
    if(tmp.type > E32Sections::EMPTY_SECTION)
    {
        iE32image.push_back(tmp);

        ExportBitmapProcessor* proc =
            new ExportBitmapProcessor(iSymbols.size(), tmp.section, iElfSrc->EntryPointOffset());
        tmp = proc->CreateExportBitmap();
        if(tmp.type > E32Sections::EMPTY_SECTION)
        {
            iE32image.push_back(tmp);
            iExportDescSize = proc->ExportDescSize();
            iExportDescType = proc->ExportDescType();
        }
        delete proc;
    }

    if(iE32Opts->iNamedlookup)
    {
        SymbolLookupProcessor* proc = new SymbolLookupProcessor(iSymbols);
        tmp = proc->SymlookSection();
        if(tmp.type == E32Sections::EMPTY_SECTION)
            ReportError(ErrorCodes::BADEXPORTS);
        iE32image.push_back(tmp);
        delete proc;
    }

    tmp = CodeSection(iElfSrc);
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    tmp = DataSection(iElfSrc);
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    #if 0
    tmp2.section = ImportsSection();
    if(!tmp2.section.empty())
    {
        tmp2.type = E32Sections::IMPORTS;
        iE32image.push_back(tmp2);
    }
    tmp2.section = CodeRelocsSection();
    if(!tmp2.section.empty())
    {
        tmp2.type = E32Sections::CODERELOCKS;
        iE32image.push_back(tmp2);
    }

    tmp2.section = DataRelocsSection();
    if(!tmp2.section.empty())
    {
        tmp2.type = E32Sections::DATARELOCKS;
        iE32image.push_back(tmp2);
    }
    #endif // 0
}


void BuildE32Image(const Args* args, const ElfParser* elfParser, const Symbols& s)
{
    E32File file(args, elfParser, s);
    file.WriteE32File();
}
