// Copyright (c) 2023-2024 Strizhniou Fiodar
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

#include "common.hpp"
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
    E32ImageHeader* h = (E32ImageHeader*)&buf[0];

    uint32_t compression = h->iCompressionType;
    h->iCompressionType = KFormatNotCompressed; // E32Parser already decompress E32Image

    iFile = E32Parser::NewL(buf);
    iHeader = (E32ImageHeader*)iFile->GetE32Hdr(); //FIXME: Add explicit write access
    iHeaderV = (E32ImageHeaderV*)iFile->GetE32HdrV(); //FIXME: Add explicit write access

    iHeader->iCompressionType = compression;

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

void E32Editor::SetCaps(uint64_t caps)
{
    iHeaderV->iS.iCaps = caps;
}

void E32Editor::SetFlags(uint32_t flags)
{
    iHeader->iFlags = flags;
}

void E32Editor::SetHeaderCrc(uint32_t headercrc)
{
    iHeader->iHeaderCrc = headercrc;
}

void E32Editor::SetCompressionType(uint32_t type)
{
    if(type != (uint32_t)-1)
        iHeader->iCompressionType = type;
}

void E32Editor::SetE32Time(uint32_t timeLo, uint32_t timeHi)
{
    iHeader->iTimeLo = timeLo;
    iHeader->iTimeHi = timeHi;
}

void E32Editor::SetVersion(uint8_t major, uint8_t minor, uint16_t build)
{
    iHeader->iVersion.iMajor = major;
    iHeader->iVersion.iMinor = minor;
    iHeader->iVersion.iBuild = build;
}

void E32Editor::ReGenerateCRCs()
{
	iHeader->iUidChecksum = GetUidChecksum(iHeader->iUid1, iHeader->iUid2, iHeader->iUid3);
	iHeader->iHeaderCrc = KImageCrcInitialiser;
	iHeader->iHeaderCrc = Crc32(iE32File, iHeader->iCodeOffset);
}

void E32Editor::DumpE32Img()
{
//    SaveFile("tests/tmp/e32file.tmp", iFile->GetBufferedImage(), iFile->GetFileSize());
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

uint64_t E32Editor::Caps() const
{
    return iHeaderV->iS.iCaps;
}

uint32_t E32Editor::Flags() const
{
    return iHeader->iFlags;
}

uint32_t E32Editor::TimeLo() const
{
    return iHeader->iTimeLo;
}

uint32_t E32Editor::TimeHi() const
{
    return iHeader->iTimeHi;
}

uint32_t E32Editor::HeaderCrc() const
{
    return iHeader->iHeaderCrc;
}

uint32_t E32Editor::CompressionType() const
{
    return iHeader->iCompressionType;
}

uint8_t E32Editor::Version_Major() const
{
    return iHeader->iVersion.iMajor;
}

uint8_t E32Editor::Version_Minor() const
{
    return iHeader->iVersion.iMinor;
}

uint16_t E32Editor::Version_Build() const
{
    return iHeader->iVersion.iBuild;
}
