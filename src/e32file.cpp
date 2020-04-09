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

#include "e32file.h"
#include "argparser.h"
#include "elfparser.h"
#include "e32common.h"
#include "elf2e32_opt.hpp"
#include "e32headerbuilder.h"

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

    iE32image.sort([](auto A, auto B){return A.type < B.type;});
    for(auto x: iE32image)
    {
        switch(x.type)
        {
        case E32Sections::HEADER:
            break;
        case E32Sections::BITMAP:
            hdrv->iExportDescSize;    // size of bitmap section // init after export bitmap creation
            hdrv->iExportDescType;    // type of description of holes in export table // init after export bitmap creation
            break;
        case E32Sections::EXPORTS:
            hdr->iExportDirOffset = iHeader.size();
        case E32Sections::CODE:
            hdr->iCodeOffset = iHeader.size();
        case E32Sections::SYMLOOK: //falltru
            hdr->iTextSize = hdr->iCodeSize = iHeader.size() + x.section.size() - hdr->iCodeOffset;
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
        hdr = (E32ImageHeader*)iHeader[0];
        hdrv = (E32ImageHeaderV*)iHeader[offset];
    }
    SetE32ImageCrc(iHeader.data());
    // see E32Rebuilder::ReCompress()
    SaveFile(iE32Opts->iOutput.c_str(), iHeader.data(), iHeader.size());
}

void BuildE32Image(const Args* args, const ElfParser* elfParser, const Symbols& s)
{
    E32File file(args, elfParser, s);
    file.WriteE32File();
}
