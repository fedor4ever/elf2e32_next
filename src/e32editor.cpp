// Copyright (c) 2023 Strizhniou Fiodar
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
// Generate checksums for the E32Image sections
//
//

#include <string.h>

#include "e32common.h"
#include "e32editor.h"
#include "e32parser.h"
#include "e32importsprocessor.hpp"

E32Editor::E32Editor(const E32Parser* const file): iFile(file)
{
    //ctor
}

void E32Editor::ConstructL()
{
    std::vector<char> buf(iFile->GetBufferedImage(), iFile->GetBufferedImage() + iFile->GetFileSize());
    iFile = E32Parser::NewL(buf);
    iHeader = (E32ImageHeader*)iFile->GetE32Hdr(); //FIXME: Add explicit write access
    iE32File = iFile->GetBufferedImage();
}

E32Editor* E32Editor::NewL(const E32Parser* const file)
{
    E32Editor* self = new E32Editor(file);
    self->ConstructL();
    return self;
}

E32Editor::~E32Editor()
{
    delete iFile;
}

void E32Editor::SetE32Time(uint32_t timeLo, uint32_t timeHi)
{
    iHeader->iTimeLo = timeLo;
    iHeader->iTimeHi = timeHi;
}

uint32_t E32Editor::FullImage() const
{
    return Crc32(iFile->GetBufferedImage(), iFile->GetFileSize());
}

uint32_t E32Editor::Header() const
{
    return Crc32(iFile->GetBufferedImage(), sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ) + sizeof(E32ImageHeaderV));
}

uint32_t E32Editor::ExportBitMap() const
{
    const E32ImageHeaderV* h = iFile->GetE32HdrV();
    if(h->iExportDescSize == 0)
        return -1;
    return Crc32(h->iExportDesc, h->iExportDescSize);
}

uint32_t E32Editor::Code() const
{
    return Crc32(iFile->GetBufferedImage() + iHeader->iCodeOffset, iHeader->iCodeSize);
}

uint32_t E32Editor::Data() const
{
    if(iHeader->iDataSize == 0)
        return -1;
    return Crc32(iFile->GetBufferedImage() + iHeader->iDataOffset, iHeader->iDataSize);
}

uint32_t E32Editor::Exports() const
{
    if(iHeader->iExportDirOffset == 0)
        return -1;
    return Crc32(iFile->GetBufferedImage() + iHeader->iExportDirOffset, iHeader->iExportDirCount * 4);
}

uint32_t E32Editor::Symlook() const
{
    if(!(iHeader->iFlags & KImageNmdExpData))
        return -1;
    const E32EpocExpSymInfoHdr* h = iFile->GetEpocExpSymInfoHdr();
    return Crc32(h, h->iSize);
}

uint32_t E32Editor::Imports() const
{
    const E32ImportSection* h = iFile->GetImportSection();
    return Crc32(h, h->iSize);
}

uint32_t E32Editor::CodeRelocs() const
{
    uint32_t length = iFile->GetFileSize() - iHeader->iCodeRelocOffset;
    if(iHeader->iDataRelocOffset > 0)
        length = iHeader->iDataRelocOffset - iHeader->iCodeRelocOffset;
    return Crc32(iFile->GetRelocSection(iHeader->iCodeRelocOffset), length);
}

uint32_t E32Editor::DataRelocs() const
{
    if(iHeader->iDataRelocOffset == 0)
        return -1;
    uint32_t length = iFile->GetFileSize() - iHeader->iDataRelocOffset;
    return Crc32(iFile->GetRelocSection(iHeader->iDataRelocOffset), length);
}

uint32_t E32Editor::TimeHi() const
{
    return iHeader->iTimeHi;
}

uint32_t E32Editor::TimeLo() const
{
    return iHeader->iTimeLo;
}

