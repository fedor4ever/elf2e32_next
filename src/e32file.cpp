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
#include "import_section.h"
#include "relocsprocessor.h"
#include "e32header_section.h"
#include "exportbitmap_section.h"
#include "symbollookup_section.h"

E32Section ExportTable(const Symbols& s);
E32Section CodeSection(const ElfParser* parser);
E32Section DataSection(const ElfParser* parser);

bool CmpSections(E32Section first, E32Section second)
{
    return first.type < second.type;
}

E32File::E32File(const Args* args, const ElfParser* elfParser, const Symbols& s):
    iE32Opts(args), iElfSrc(elfParser), iSymbols(s){}

E32File::~E32File()
{
    delete iRelocs;
}

void UpdateOrdinalOffsetTable(E32Section& s, uint32_t offSet, const std::vector<int32_t>& iImportTabLocations)
{
    E32EpocExpSymInfoHdr* symInf = (E32EpocExpSymInfoHdr*)&s.section[0];
    uint32_t* aImportTab = (uint32_t*)(&s.section[0] + symInf->iDepDllZeroOrdTableOffset);
    offSet += symInf->iDepDllZeroOrdTableOffset; // This points to the ordinal zero offset table now
    for(auto x: iImportTabLocations)
    {
        uint32_t* aLocation = (aImportTab + x);
        *aLocation = offSet;
        offSet += sizeof(uint32_t);
    }
}

void E32File::WriteE32File()
{
    iRelocs = new RelocsProcessor(iElfSrc);
    iRelocs->Process();

    E32HeaderSection header(iE32Opts);
    iHeader = header.MakeE32Header();

    if(!iSymbols.empty())
        iHeader.pop_back(); // remove E32ImageHeaderV::iExportDesc[1]

    printf("header size: %u\n", iHeader.size());

    E32ImageHeader* hdr = (E32ImageHeader*)&iHeader[0];
    hdr->iDataSize = iElfSrc->DataSegmentSize();
    hdr->iBssSize = iElfSrc->BssSegmentSize();
    hdr->iEntryPoint = iElfSrc->EntryPointOffset();
    hdr->iCodeBase = iElfSrc->ROVirtualAddress();
    hdr->iDataBase = iElfSrc->RWVirtualAddress();
    hdr->iDllRefTableCount; // = iNumDlls;   // filling this in enables E32ROM to leave space for it
    hdr->iExportDirCount = iSymbols.size();

    const uint32_t offset = sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ);
    E32ImageHeaderV* hdrv = (E32ImageHeaderV*)&iHeader[offset];

    hdrv->iExceptionDescriptor = iElfSrc->ExceptionDescriptor();

    PrepareData();

    iE32image.sort([](auto A, auto B){return A.type < B.type;});
    for(auto x: iE32image)
    {
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
            /// TODO (#1#05/30/20): Implement initialization for field iImportTabLocations
//            UpdateOrdinalOffsetTable(x, hdr->iTextSize);
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

        printf("Added Chunks has size: %06zx for section: %s at address: %08zx\n",
               x.section.size(), x.info.c_str(), iHeader.size());
        iHeader.insert(iHeader.end(), x.section.begin(), x.section.end());
        hdr = (E32ImageHeader*)&iHeader[0];
        hdrv = (E32ImageHeaderV*)&iHeader[offset];
    }
    SetE32ImageCrc(iHeader.data());
    // see E32Rebuilder::ReCompress()
    SaveFile(iE32Opts->iOutput.c_str(), iHeader.data(), iHeader.size());
}

// Export Section consist of uint32_t array, zero element contains section's size.
// Absent symbols values set E32 image entry point, other set to their elf st_value
E32Section MakeExportSection(const Symbols& s)
{
    E32Section exports;
    if(s.empty())
        return exports;

    exports.info = "EXPORTS";
    exports.type = E32Sections::EXPORTS;
    // The export table has a header containing the number of entries
	// before the entries themselves. So add 1 to number of exports
    uint32_t sz = s.size() + 1;
    exports.section.insert(exports.section.begin(), sz * sizeof(uint32_t), 0);

    uint32_t* iTable = (uint32_t*)exports.section.data();
    iTable[0] = s.size();

    uint32_t i = 1;
    for(auto x: s)
        iTable[i++] = x->Elf_st_value();
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
    data.section.insert(data.section.begin(), parser->DataSegment(), parser->DataSegment() + parser->DataSegmentSize());
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

        ExportBitmapSection* proc =
            new ExportBitmapSection(iSymbols.size(), tmp.section, iElfSrc->EntryPoint());
        tmp = proc->CreateExportBitmap();
        if(tmp.type > E32Sections::EMPTY_SECTION)
        {
            iE32image.push_back(tmp);
            iExportDescSize = proc->ExportDescSize();
            iExportDescType = proc->ExportDescType();
        }
        delete proc;
    }

    tmp = CodeSection(iElfSrc);
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    ImportsSection* proc = new ImportsSection(iElfSrc, iRelocs, iE32Opts);
    tmp = proc->Imports();
    iImportTabLocations = proc->ImportTabLocations();
    iE32image.push_back(tmp);
    delete proc;

    if(iE32Opts->iNamedlookup)
    {
        uint32_t r = iRelocs->DllCount();
        SymbolLookupSection* proc = new SymbolLookupSection(iSymbols, r);
        tmp = proc->SymlookSection();
        if(tmp.type == E32Sections::EMPTY_SECTION)
            ReportError(ErrorCodes::BADEXPORTS);
        iE32image.push_back(tmp);
        delete proc;
    }

    tmp = DataSection(iElfSrc);
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    tmp = iRelocs->DataRelocsSection();
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    tmp = iRelocs->CodeRelocsSection();
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);
}


void BuildE32Image(const Args* args, const ElfParser* elfParser, const Symbols& s)
{
    E32File file(args, elfParser, s);
    file.WriteE32File();
}
