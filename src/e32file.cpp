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


#include <string.h>

#include "symbol.h"
#include "e32file.h"
#include "e32info.h"
#include "argparser.h"
#include "elfparser.h"
#include "e32common.h"
#include "e32parser.h"
#include "e32rebuilder.h"
#include "e32validator.h"
#include "e32compressor.h"
#include "elf2e32_opt.hpp"
#include "import_section.h"
#include "relocsprocessor.h"
#include "e32header_section.h"
#include "exportbitmap_section.h"
#include "symbollookup_section.h"

E32Section CodeSection(const ElfParser* parser);
E32Section DataSection(const ElfParser* parser);
bool IsSimpleEXE(TargetType type);

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

void UpdateImportTable(const char* s, size_t bufsz, const std::vector<int32_t>& iImportTabLocations)
{
    E32Parser* p = new E32Parser(s, bufsz);
    const E32ImageHeader* h = p->GetFileLayout();
    const E32EpocExpSymInfoHdr* symInf = p->GetEpocExpSymInfoHdr();
    size_t offSet = p->ExpSymInfoTableOffset();
    E32EpocExpSymInfoHdr* sInf = (E32EpocExpSymInfoHdr*)(p->GetBufferedImage() + offSet);
    offSet += sInf->iDepDllZeroOrdTableOffset; // This points to the ordinal zero offset table now
    offSet -= h->iCodeOffset;

    uint32_t* aImportTab = (uint32_t*)p->GetImportSection();
    for(auto x: iImportTabLocations)
    {
        aImportTab[x] = offSet;
        offSet += sizeof(uint32_t);
    }
    delete p;
}

void E32File::SetFixedAddress(E32ImageHeader* hdr)
{
    if(iE32Opts->iFixedaddress)
    {
        if(iElfSrc->ImageIsDll())
            ReportLog("Warning: Cannot set fixed address for DLL.\n");
        else
            hdr->iFlags|=KImageFixedAddressExe;
    }
    else
        hdr->iFlags&=~KImageFixedAddressExe;
}

void E32File::WriteE32File()
{
    /**< SDK versions ignore exported symbols for EXE */
    if(IsSimpleEXE(iE32Opts->iTargettype)) {
        const char t[] = "has exported symbol(s)\n";
        ReportWarning(ErrorCodes::BADFILE, iE32Opts->iOutput.c_str(), t);
        if(iE32Opts->iVerbose)
        {
            ReportLog("*********************\n");
            for(auto x: iSymbols)
                ReportLog(x->AliasName() + "\n");
            ReportLog("*********************\n");
        }
        Symbols s;
        iSymbols = s;
    }

    iRelocs = new RelocsProcessor(iElfSrc, iSymbols, iE32Opts->iNamedlookup);
    iRelocs->Process();

    E32HeaderSection header(iE32Opts);
    iHeader = header.MakeE32Header();

    E32ImageHeader* hdr = (E32ImageHeader*)&iHeader[0];
    hdr->iDataSize = iElfSrc->DataSegmentSize();
    hdr->iBssSize = iElfSrc->BssSegmentSize();
    hdr->iEntryPoint = iElfSrc->EntryPointOffset();
    hdr->iCodeBase = iElfSrc->ROVirtualAddress();
    hdr->iDataBase = iElfSrc->RWVirtualAddress();
    hdr->iDllRefTableCount = iRelocs->DllCount();   // filling this in enables E32ROM to leave space for it
    hdr->iExportDirCount = iSymbols.size();
    hdr->iCompressionType = 0;

    SetFixedAddress(hdr);

    if(iElfSrc->ImageIsDll())
        hdr->iFlags |= KImageDll;

    const uint32_t offset = sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ);
    E32ImageHeaderV* hdrv = (E32ImageHeaderV*)&iHeader[offset];

    hdrv->iExceptionDescriptor = iElfSrc->ExceptionDescriptor();

    PrepareData();

    for(auto x: iE32image)
    {
// we set this field outside switch because Symbian Post Linker, Elf2E32 V2.0
// set this field for exes as KImageHdr_ExpD_FullBitmap.
        hdrv->iExportDescType = this->iExportDescType;
        switch(x.type)
        {
        case E32Sections::HEADER:
            break;
        case E32Sections::BITMAP:
            hdrv->iExportDescSize = this->iExportDescSize;
            hdr->iCodeOffset = iHeader.size() + x.section.size();
            break;
        case E32Sections::EXPORTS: //falltru
            hdr->iExportDirOffset = iHeader.size() + sizeof(uint32_t); // point directly to exports
        case E32Sections::SYMLOOK: //falltru
        case E32Sections::CODE: //falltru
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

        if(iE32Opts->iVerbose) {
            printf("Added Chunks has size: %06zx for section:"
               " %s at address: %08zx\n",
               x.section.size(), x.info.c_str(), iHeader.size());
        }

        iHeader.insert(iHeader.end(), x.section.begin(), x.section.end());
        hdr = (E32ImageHeader*)&iHeader[0];
        hdrv = (E32ImageHeaderV*)&iHeader[offset];
    }
    UpdateImportTable(iHeader.data(), iHeader.size(), iImportTabLocations);
    E32ImageHeaderJ* j = (E32ImageHeaderJ*)&iHeader[sizeof(E32ImageHeader)];
    j->iUncompressedSize = iHeader.size() - hdr->iCodeOffset;

//    Compress(iE32Opts->iCompressionMethod);
    hdr = (E32ImageHeader*)&iHeader[0];
    hdr->iCompressionType = iE32Opts->iCompressionMethod;

    SetE32ImageCrc(iHeader.data());

// Compression doesn't work so we use E32Rebuilder to compress.
//    E32SectionUnit s(iHeader);
//    E32Parser* p = new E32Parser(s.data(), s.size());
//    p->GetFileLayout();
//    ValidateE32Image(p);

    Args arg;
    arg.iCompressionMethod = iE32Opts->iCompressionMethod;
    arg.iOutput = iE32Opts->iOutput;
    //does that crap because E32Rebuilder free filebuf itself
    char* t = new char[iHeader.size()]();
    memcpy(t, &iHeader[0], iHeader.size());

    //call E32Info::HeaderInfo() for verbose output
    if(iE32Opts->iVerbose)
    {
        hdr = (E32ImageHeader*)&t[0];
        hdr->iCompressionType = KFormatNotCompressed;

        auto sz = iHeader.size();
        E32Info* info = new E32Info(t, sz);
        info->HeaderInfo();
        delete info;

        hdr->iCompressionType = iE32Opts->iCompressionMethod;
    }

    E32Rebuilder* rb = new E32Rebuilder(&arg, t, iHeader.size());
    rb->Compress();
    delete rb;
    iHeader.clear();
//    SaveFile(iE32Opts->iOutput.c_str(), iHeader.data(), iHeader.size());
}

// Export Section consist of uint32_t array, first element contains section's size.
// Absent symbols values set E32 image entry point, other set to their elf st_value.
E32Section ExportSection(const Symbols& s, uintptr_t iExportTableAddress,
                             bool symlook, bool HasNoDefIn)
{
    E32Section exports;
    if(s.empty())
        return exports;

    exports.info = "EXPORTS";
    exports.type = E32Sections::EXPORTS;
    // The export table has a header containing the number of entries
	// before the entries themselves. So add 1 to number of exports
    uint32_t sz = (s.size() + 1) * sizeof(uint32_t);
    exports.section.insert(exports.section.begin(), sz, 0);

    uint32_t* iTable = (uint32_t*)exports.section.data();
    iTable[0] = s.size();

    /// TODO (Administrator#1#07/19/20): Check how OS loader handle export section if --namedlookup used
    if(HasNoDefIn && symlook) // original algorithm works as no symbols provided
        sz = 4;
    if(symlook)
        iTable[0] = sz + iExportTableAddress;

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
    E32Section data;
    if(parser->DataSegmentSize() == 0)
        return data;
    data.info = "DATA";
    data.type = E32Sections::DATA;
    data.section.insert(data.section.begin(), parser->DataSegment(), parser->DataSegment() + parser->DataSegmentSize());
    return data;
}

bool IsEXE(TargetType type)
{
    if( (type == TargetType::EExe) || (type == TargetType::EExexp) ||
        (type == TargetType::EStdExe) )
        return true;
    return false;
}

bool IsSimpleEXE(TargetType type)
{
    if( (type == TargetType::EExe) || (type == TargetType::EStdExe) )
        return true;
    return false;
}

void E32File::PrepareData()
{
    E32Section tmp;
    tmp = ExportSection(iSymbols, iRelocs->ExportTableAddress(),
                    iE32Opts->iNamedlookup, iE32Opts->iDefinput.empty());

    if(IsEXE(iE32Opts->iTargettype))
        iExportDescType = KImageHdr_ExpD_FullBitmap;

    if(tmp.type > E32Sections::EMPTY_SECTION)
    {
        if(iE32Opts->iVerbose || IsSimpleEXE(iE32Opts->iTargettype)) {
            ReportLog("***************\n");
            ReportLog("Exported symbols:\n");
            for(auto x: iSymbols) {
                ReportLog(x->Name());
                ReportLog("\n");
            }
            ReportLog("***************\n");
        }

        iE32image.push_back(tmp);

        ExportBitmapSection* proc =
            new ExportBitmapSection(iSymbols.size(), tmp.section, iElfSrc->EntryPoint());
        tmp = proc->CreateExportBitmap();
        if(tmp.type > E32Sections::EMPTY_SECTION)
        {
            iHeader.pop_back(); // remove E32ImageHeaderV::iExportDesc[1]
            iE32image.push_back(tmp);
            iExportDescSize = proc->ExportDescSize();
            iExportDescType = proc->ExportDescType();
        }
        delete proc;
    }

// While import section builds their relocs implicitly apply
// for code and data sections. Therefore should run first
    ImportsSection* proc = new ImportsSection(iElfSrc, iRelocs, iE32Opts);
    tmp = proc->Imports();
    iImportTabLocations = proc->ImportTabLocations();
    iE32image.push_back(tmp);
    delete proc;

    tmp = CodeSection(iElfSrc);
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

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

    tmp = iRelocs->CodeRelocsSection();
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);

    tmp = iRelocs->DataRelocsSection();
    if(tmp.type > E32Sections::EMPTY_SECTION)
        iE32image.push_back(tmp);
    iE32image.sort([](auto A, auto B){return A.type < B.type;});
}

void BuildE32Image(const Args* args, const ElfParser* elfParser, const Symbols& s)
{
    E32File file(args, elfParser, s);
    file.WriteE32File();
}

void E32File::Compress(uint32_t compression)
{
    if(!compression)
        return;
    if(compression == KUidCompressionDeflate)
        iHeader = CompressDeflate(iHeader);
    else if(compression == KUidCompressionBytePair)
        iHeader = CompressBPE(iHeader);
    else
        ReportError(INVALIDARGUMENT, "--compressionmethod", std::to_string(compression));
}
