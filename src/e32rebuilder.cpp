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

#include "common.hpp"
#include "e32common.h"
#include "e32parser.h"
#include "e32validator.h"
#include "e32rebuilder.h"
#include "elf2e32_opt.hpp"
#include "elf2e32_version.hpp"

E32Rebuilder::E32Rebuilder(Args* param): iReBuildOptions(param)
{
}

void E32Rebuilder::Run()
{
	iFile = ReadFile(iReBuildOptions->iE32input.c_str(), iFileSize);
	//for decompression purpose we provide memory buffer large enough to hold uncompressed data
	if( ((E32ImageHeader*)(iFile))->iCompressionType)
    {
        uint32_t extracted = ((E32ImageHeader*)(iFile))->iCodeOffset;
        extracted += ((E32ImageHeaderJ*)(iFile + sizeof(E32ImageHeader) ))->iUncompressedSize;
        const char* newfile = new char[extracted]();
        memcpy(newfile, iFile, iFileSize);
        delete[] iFile;
        iFile = nullptr;
        iFile = newfile;
        iFileSize = extracted;
    }

	iParser = new E32Parser(iFile, iFileSize);
	iHdr = iParser->GetFileLayout();
	iFileSize = iParser->GetFileSize();
	iFile = iParser->GetBufferedImage();
//    ValidateE32Image(iParser);
	EditHeader();
	ReCompress();
	SetE32ImageCrc(iFile);
	SaveFile(iReBuildOptions->iOutput.c_str(), iFile, iFileSize);
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

//	h->iTimeHi =;
//	h->iTimeLo =;

	if(iReBuildOptions->iHeapMin || iReBuildOptions->iHeapMax)
	{
		iHdr->iHeapSizeMin = iReBuildOptions->iHeapMin;
		iHdr->iHeapSizeMax = iReBuildOptions->iHeapMax;
	}

	if(iReBuildOptions->iPriority)
		iHdr->iProcessPriority = iReBuildOptions->iPriority;

	E32ImageHeaderV* v = iParser->GetE32HdrV();
	if(iReBuildOptions->iSid)
		v->iS.iSecureId = iReBuildOptions->iSid;
	if(iReBuildOptions->iVid)
		v->iS.iVendorId = iReBuildOptions->iVid;
}

void E32Rebuilder::ReCompress()
{
    return;
//  compress E32Image if needed
    iHdr->iCompressionType = iReBuildOptions->iCompressionMethod;
}
