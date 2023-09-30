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
    ReCompress();
    SetE32ImageCrc(iFile);
// We create copy of file object because ValidateE32Image(iParser) break it's consistency.
    std::vector<char> file;
    file.assign(iFile, iFile + iFileSize);
// We reinit E32Parser object because it's consistency broken.
    delete iParser;
    iParser = E32Parser::NewL(file);

    ValidateE32Image(iParser);
    CheckE32CRC(iParser, iReBuildOptions);
    SaveFile(iReBuildOptions->iOutput.c_str(), file.data(), iFileSize);
}

E32Rebuilder::~E32Rebuilder()
{
    delete iParser;
    delete[] iFile;
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

void E32Rebuilder::ReCompress()
{
    if(!iHdr)
        ReportError(ErrorCodes::ZEROBUFFER, __func__);
    if(!iReBuildOptions->iCompressionMethod)
        return;
    iHdr->iCompressionType = iReBuildOptions->iCompressionMethod;

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

    delete[] iFile;
    iFile = nullptr;
    iFile = compressed;
    iFileSize = offset + compressedSize;
}

void E32Rebuilder::Compress(const std::vector<char>& e32File)
{
    iFileSize = e32File.size();
    iFile = new char[iFileSize]();
    std::copy(e32File.begin(), e32File.end(), iFile);
    iHdr = (E32ImageHeader*)iFile;
    ReCompress();

    // We create copy of file object because ValidateE32Image(iParser) break it's consistency.
    std::vector<char> file;
    file.assign(iFile, iFile + iFileSize);
    iParser = E32Parser::NewL(file);

    if(iReBuildOptions->iForceE32Build == false) // can't build invalid E32Image while validate on
        ValidateE32Image(iParser);
    CheckE32CRC(iParser, iReBuildOptions);
    SaveFile(iReBuildOptions->iOutput.c_str(), file.data(), iFileSize);
}
