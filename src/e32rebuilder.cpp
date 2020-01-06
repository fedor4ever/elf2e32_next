/*
 * E32Rebulder.cpp
 *
 *  Created on: 6 янв. 2020 г.
 *      Author: Artiom
 */

#include <e32common.h>
#include "common.hpp"
#include "e32parser.h"
#include "e32rebuilder.h"
#include "elf2e32_opt.hpp"
#include "elf2e32_version.hpp"

E32Rebuilder::E32Rebuilder(Args* param): iReBuildOptions(param)
{
	iFile = ReadFile(iReBuildOptions->iE32input.c_str(), iFileSize);
}

void E32Rebuilder::Run()
{
	EditHeader();
	ReCompress();
	SaveFile(iReBuildOptions->iOutput.c_str(), iFile, iFileSize);
}

E32Rebuilder::~E32Rebuilder()
{
	delete iParser;
	delete[] iFile;
}

void E32Rebuilder::EditHeader()
{
	E32ImageHeader* h = (E32ImageHeader*)iFile;
	E32ImageHeaderV* v = (E32ImageHeaderV*)iFile + sizeof(E32ImageHeader);
	if(iReBuildOptions->iUid1)
		h->iUid1 = iReBuildOptions->iUid1;
	if(iReBuildOptions->iUid2)
		h->iUid2 = iReBuildOptions->iUid2;
	if(iReBuildOptions->iUid3)
		h->iUid3 = iReBuildOptions->iUid3;

	if(iReBuildOptions->iVersion)
		h->iModuleVersion = iReBuildOptions->iVersion;

	ToolVersion tool;
	h->iVersion.iMajor = tool.iMajor;
	h->iVersion.iMinor = tool.iMinor;
	h->iVersion.iBuild = tool.iBuild;

//	h->iTimeHi =;
//	h->iTimeLo =;

	if(iReBuildOptions->iHeapMin || iReBuildOptions->iHeapMax)
	{
		h->iHeapSizeMin = iReBuildOptions->iHeapMin;
		h->iHeapSizeMax = iReBuildOptions->iHeapMax;
	}

	if(iReBuildOptions->iPriority)
		h->iProcessPriority = iReBuildOptions->iPriority;

	h->iUidChecksum = GetUidChecksum(h->iUid1, h->iUid2, h->iUid3);
	h->iHeaderCrc = KImageCrcInitialiser;
	uint32_t crc = Crc32(iFile, h->iCodeOffset);
	h->iHeaderCrc = crc;

	if(iReBuildOptions->iSid)
		v->iS.iSecureId = iReBuildOptions->iSid;
	if(iReBuildOptions->iVid)
		v->iS.iVendorId = iReBuildOptions->iVid;
}

void E32Rebuilder::ReCompress()
{
//	if(iReBuildOptions->iCompressionmethod.empty())
		return;
	iParser = new E32Parser(iFile, iFileSize);
	iFileSize = iParser->GetFileSize();
}
