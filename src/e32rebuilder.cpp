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
// Edit E32Image header fields and compression type
//
//

#include <vector>
#include <string.h>

#include "common.hpp"
#include "e32common.h"
#include "e32parser.h"
#include "symbiantime.h"
#include "e32rebuilder.h"
#include "e32validator.h"
#include "e32compressor.h"
#include "elf2e32_opt.hpp"
#include "elf2e32_version.hpp"

E32Rebuilder::E32Rebuilder(Args* param): iReBuildOptions(param) {}

void E32Rebuilder::Run()
{
    iParser = E32Parser::NewL(iReBuildOptions->iE32input);
    iHdr = (E32ImageHeader*)iParser->GetE32Hdr();
    iFileSize = iParser->GetFileSize();
    iFile = (char*)iParser->GetBufferedImage();

    EditHeader();
    E32Buf file = ReCompress();
    SetE32ImageCrc(&file[0]);
    SaveAndValidate(file);
}

E32Rebuilder::~E32Rebuilder()
{
    delete iParser;
}

void E32Rebuilder::EditHeader()
{
    if(iReBuildOptions->iUid1)
        iHdr->iUid1 = iReBuildOptions->iUid1;
    if(iReBuildOptions->iUid2)
        iHdr->iUid2 = iReBuildOptions->iUid2;
    if(iReBuildOptions->iUid3)
        iHdr->iUid3 = iReBuildOptions->iUid3;

    if(iReBuildOptions->iVersion)
        iHdr->iModuleVersion = iReBuildOptions->iVersion;

    ToolVersion tool;
    iHdr->iVersion.iMajor = tool.iMajor;
    iHdr->iVersion.iMinor = tool.iMinor;
    iHdr->iVersion.iBuild = tool.iBuild;

    SymbianTime t;
    iHdr->iTimeLo = t.TimeLo();
    iHdr->iTimeHi = t.TimeHi();

    if(iReBuildOptions->iHeapMin || iReBuildOptions->iHeapMax)
    {
        iHdr->iHeapSizeMin = iReBuildOptions->iHeapMin;
        iHdr->iHeapSizeMax = iReBuildOptions->iHeapMax;
    }

    if(iReBuildOptions->iPriority)
        iHdr->iProcessPriority = iReBuildOptions->iPriority;

    E32ImageHeaderV* v = (E32ImageHeaderV*)iParser->GetE32HdrV();
    if(iReBuildOptions->iSid)
        v->iS.iSecureId = iReBuildOptions->iSid;
    if(iReBuildOptions->iVid)
        v->iS.iVendorId = iReBuildOptions->iVid;
    if(!iReBuildOptions->iCapability.empty())
        v->iS.iCaps = ProcessCapabilities(iReBuildOptions->iCapability);
}

E32Buf E32Rebuilder::ReCompress()
{
    if(!iHdr)
        ReportError(ErrorCodes::ZEROBUFFER, __func__);

    iHdr->iCompressionType = iReBuildOptions->iCompressionMethod;
    if(!iReBuildOptions->iCompressionMethod)
        return E32Buf(iFile, iFile + iFileSize);

    char* compressed = new char[iFileSize * 2]();
    uint32_t compressedSize = 0;

    uint32_t offset = iHdr->iCodeOffset;
    memcpy(compressed, iFile, offset);
    if(iHdr->iCompressionType == KUidCompressionBytePair)
    {
        int32_t BPECodeSize = CompressBPE(iFile + offset, iHdr->iCodeSize, compressed + offset, iFileSize - offset);

        int32_t srcOffset = offset + iHdr->iCodeSize;

        int32_t BPEDataSize = CompressBPE(nullptr, iFileSize - srcOffset,
                                          nullptr, iFileSize - BPECodeSize);

        compressedSize = BPECodeSize + BPEDataSize;
    }else if(iHdr->iCompressionType == KUidCompressionDeflate)
    {
        compressedSize = CompressDeflate(iFile + offset, iFileSize - offset, compressed + offset, iFileSize * 2 - offset);
    }

    iFileSize = offset + compressedSize;
    return E32Buf(compressed, compressed + iFileSize);
}

void E32Rebuilder::Compress(const E32Buf& e32File)
{
    iParser = E32Parser::NewL(e32File);
    iHdr = (E32ImageHeader*)iParser->GetE32Hdr();
    iFileSize = e32File.size();
    iFile = (char*)iParser->GetBufferedImage();
    SaveAndValidate(ReCompress());
}

void E32Rebuilder::SaveAndValidate(const E32Buf& e32File)
{
    delete iParser;
    iParser = nullptr;
    iParser = E32Parser::NewL(e32File);

    if(iReBuildOptions->iForceE32Build == false) // can't build invalid E32Image while validate on
        ValidateE32Image(iParser);
    CheckE32CRC(iParser, iReBuildOptions);
    SaveFile(iReBuildOptions->iOutput.c_str(), e32File.data(), iFileSize);
}
