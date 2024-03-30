// Copyright (c) 2018-2024 Strizhniou Fiodar
// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Use some code from original elf2e32
//
// Description:
// Implementation of the Class E32Info for the elf2e32 tool
// @internalComponent
// @released
//
//

#include <cstdio>
#include <cstring>
#include <cinttypes>

#include "symbol.h"
#include "e32info.h"
#include "common.hpp"
#include "e32parser.h"
#include "e32validator.h"
#include "e32capability.h"
#include "elf2e32_opt.hpp"
#include "e32importsprocessor.hpp"

void DumpRelocs(const E32RelocSection* relocs);
void GenerateAsmFile(const Args* param);
void PrintHexData(const void* pos, size_t lenth);

E32Info::E32Info(const Args* param): iParam(param)
{
    iE32 = E32Parser::NewL(param->iE32input);
    iHdr = iE32->GetE32Hdr();
}

E32Info::E32Info(const Args* param, const std::vector<char>& e32File): iParam(param)
{
    iE32 = E32Parser::NewL(e32File);
    iHdr = iE32->GetE32Hdr();
}

E32Info::~E32Info()
{
    delete iE32;
}

void E32Info::HeaderInfo()
{
    uint32_t flags = iHdr->iFlags;
    bool isARM = false;
    uint32_t hdrfmt = HdrFmtFromFlags(flags);

    printf("Builded with tool v%d.%02d(%03d)", iHdr->iVersion.iMajor,
           iHdr->iVersion.iMinor, iHdr->iVersion.iBuild);
    printf("\tTime Stamp: %08x,%08x\n", iHdr->iTimeHi, iHdr->iTimeLo);

    char sig[5] = {};
    memcpy(sig, (const char*)&iHdr->iSignature, 4);
    printf(sig);

    if(flags & KImageDll)
        printf(" Dll for ");
    else
        printf(" Exe for ");
    CPUIdentifier(iHdr->iCpuIdentifier, isARM);

    printf("Flags:\t%08x\n", flags);

    if (!(flags & KImageDll))
    {
        printf("Priority ");
        ImagePriority((TProcessPriority)iHdr->iProcessPriority);
        if (flags & KImageFixedAddressExe)
            printf("Fixed process\n");
    }

    if (flags & KImageNoCallEntryPoint)
        printf("Entry points are not called\n");

    printf("Image header is format %u\n", hdrfmt>>24);
    uint32_t compression = iHdr->iCompressionType;

    switch (compression)
    {
    case KFormatNotCompressed:
        printf("Image is not compressed\n");
        break;
    case KUidCompressionDeflate:
        printf("Image is compressed using the DEFLATE algorithm\n");
        break;
    case KUidCompressionBytePair:
        printf("Image is compressed using the BYTEPAIR algorithm\n");
        break;
    default:
        printf("Image compression type UNKNOWN (%08x)\n", compression);
    }

    if (compression)
        printf("Uncompressed size %08x\n", iE32->UncompressedFileSize());


    uint32_t FPU = flags & KImageHWFloatMask;
    if (FPU == KImageHWFloat_None)
        printf("Image FPU support : Soft VFP\n");
    else if (FPU == KImageHWFloat_VFPv2)
        printf("Image FPU support : VFPv2\n");
    else if (FPU == KImageHWFloat_VFPv3)
        printf("Image FPU support : VFPv3\n");
    else if (FPU == KImageHWFloat_VFPv3D16)
        printf("Image FPU support : VFPv3D16\n");
    else
        printf("Image FPU support : Unknown\n");


    if (flags & KImageCodeUnpaged)
        printf("Code Paging : Unpaged\n");
    else if (flags & KImageCodePaged)
        printf("Code Paging : Paged\n");
    else
        printf("Code Paging : Default\n");


    if (flags & KImageDataUnpaged)
        printf("Data Paging : Unpaged\n");
    else if (flags & KImageDataPaged)
        printf("Data Paging : Paged\n");
    else
        printf("Data Paging : Default\n");

    if (flags & KImageDebuggable)
        printf("Debuggable : True\n");
    else
        printf("Debuggable : False\n");

    if (flags & KImageSMPSafe)
        printf("SMP Safe : True\n");
    else
        printf("SMP Safe : False\n");

    SecurityInfo();

    if (hdrfmt >= E32HdrFmt::KImageHdrFmt_V)
    {
        const E32ImageHeaderV* v = iE32->GetE32HdrV();
        uint32_t xd = v->iExceptionDescriptor;
        if ((xd & 1) && (xd != 0xffffffffu))
        {
            printf("Exception Descriptor Offset:  %08x\n", xd);
            const TExceptionDescriptor *ed = iE32->GetExceptionDescriptor();
            printf("Exception Index Table Base: %08x\n", ed->iExIdxBase);
            printf("Exception Index Table Limit: %08x\n", ed->iExIdxLimit);
            printf("RO Segment Base: %08x\n", ed->iROSegmentBase);
            printf("RO Segment Limit: %08x\n", ed->iROSegmentLimit);
        }
        else
            printf("No Exception Descriptor\n");

        printf("Export Description: Size=%03x, Type=%02x\n", v->iExportDescSize, v->iExportDescType);

        if (v->iExportDescType != KImageHdr_ExpD_NoHoles)
        {
            int32_t nb = v->iExportDescSize;
            for (int32_t i=0, j = 0; i<nb; ++i)
            {
                if (++j == 8)
                {
                    j = 0;
                    printf("\n");
                }
                printf(" %02x", v->iExportDesc[i]);
            }
            printf("\n");
        }

        int32_t r = iE32->GetExportDescription();

        if (r == KErrNone)
            printf("Export description consistent\n");
        else if (r == KErrNotSupported)
            printf("Export description type not recognised\n");
        else
            printf("!! Export description inconsistent !!\n");
    }

    uint32_t mv = iHdr->iModuleVersion;
    printf("Module Version: %u.%u\n", mv>>16, mv&0xffff);

    switch(ImpFmtFromFlags(flags))
    {
    case KImageImpFmt_PE:
        printf("Imports are PE-style\n");
        break;
    case KImageImpFmt_ELF:
        printf("Imports are ELF-style\n");
        break;
    case KImageImpFmt_PE2:
        printf("Imports are PE-style without redundant ordinal lists\n");
        break;
    default:
        break;
    }

    /// TODO (Administrator#1#09/11/18): Print right message for pre-9 binaries
    printf("ARM EABI\n");
    printf("Built against EKA2\n");
    // end todo

    printf("Uids:\t\t%08x %08x %08x (%08x)\n", iHdr->iUid1, iHdr->iUid2, iHdr->iUid3, iHdr->iUidChecksum);

    if (hdrfmt >= KImageHdrFmt_V)
        printf("Header CRC:\t%08x\n", iHdr->iHeaderCrc);

    printf("File Size:\t%08" PRIxMAX "\n", (uintmax_t)iE32->GetFileSize());
    printf("Code Size:\t%08x\n", iHdr->iCodeSize);
    printf("Data Size:\t%08x\n", iHdr->iDataSize);
    printf("Compression:\t%08x\n", iHdr->iCompressionType);
    printf("Min Heap Size:\t%08x\n", iHdr->iHeapSizeMin);
    printf("Max Heap Size:\t%08x\n", iHdr->iHeapSizeMax);
    printf("Stack Size:\t%08x\n", iHdr->iStackSize);
    printf("Code link addr:\t%08x\n", iHdr->iCodeBase);
    printf("Data link addr:\t%08x\n", iHdr->iDataBase);
    printf("Code reloc offset:\t%08x\n", iHdr->iCodeRelocOffset);
    printf("Data reloc offset:\t%08x\n", iHdr->iDataRelocOffset);
    printf("Dll ref table count: %d\n", iHdr->iDllRefTableCount);

    if (iHdr->iCodeSize || iHdr->iDataSize || iHdr->iBssSize || iHdr->iImportOffset)
        printf("        Offset  Size  Relocs #Relocs\n");

    printf("Code    %06x %06x", iHdr->iCodeOffset, iHdr->iCodeSize);

    if (iHdr->iCodeRelocOffset)
    {
        const E32RelocSection *r = iE32->GetRelocSection(iHdr->iCodeRelocOffset);
        printf(" %06x %06x", iHdr->iCodeRelocOffset, r->iNumberOfRelocs);
    }else printf("              ");

    printf("        +%06x (entry pnt)", iHdr->iEntryPoint);
    printf("\n");

    printf("Data    %06x %06x", iHdr->iDataOffset, iHdr->iDataSize);

    if (iHdr->iDataRelocOffset)
    {
        const E32RelocSection *r = iE32->GetRelocSection(iHdr->iDataRelocOffset);
        printf(" %06x %06x", iHdr->iDataRelocOffset, r->iNumberOfRelocs);
    }
    printf("\n");

    printf("Bss            %06x\n", iHdr->iBssSize);

    if (iHdr->iExportDirOffset)
        printf("Export  %06x %06x                      (%u entries)\n",
               iHdr->iExportDirOffset, iHdr->iExportDirCount*4, iHdr->iExportDirCount);

    if (iHdr->iImportOffset)
        printf("Import  %06x\n", iHdr->iImportOffset);
}

void E32Info::ImagePriority(TProcessPriority priority) const
{
    if (priority==EPrioritySupervisor)
		printf("Supervisor");

    else if (priority>EPriorityRealTimeServer)
		printf("RealTime+%d", priority-EPriorityRealTimeServer);
    else if (priority==EPriorityRealTimeServer)
		printf("RealTime");

    else if (priority>EPriorityFileServer)
		printf("FileServer+%d", priority-EPriorityFileServer);
	else if (priority==EPriorityFileServer)
		printf("FileServer");

	else if (priority>EPriorityWindowServer)
		printf("WindowServer+%d", priority-EPriorityWindowServer);
	else if (priority==EPriorityWindowServer)
		printf("WindowServer");

    else if (priority>EPriorityHigh)
		printf("High+%d", priority-EPriorityHigh);
	else if (priority==EPriorityHigh)
		printf("High");

	else if (priority>EPriorityForeground)
		printf("Foreground+%d", priority-EPriorityForeground);
	else if (priority==EPriorityForeground)
		printf("Foreground");

	else if (priority>EPriorityBackground)
		printf("Background+%d", priority-EPriorityBackground);
	else if (priority==EPriorityBackground)
		printf("Background");

    else if (priority>EPriorityLow)
		printf("Low+%d", priority-EPriorityLow);
	else if (priority==EPriorityLow)
		printf("Low");

	else
		printf("Illegal (%d)", priority);

    printf("\n");
}

void E32Info::SecurityInfo(bool aCapNames)
{
    uint32_t fmt = HdrFmtFromFlags(iHdr->iFlags);
    if (fmt < KImageHdrFmt_V)
        return;

    //
    // Important. Don't change output format of following security info
    // because this is relied on by used by "Symbian Signed".
    //
    const E32ImageHeaderV* v = iE32->GetE32HdrV();
    uint64_t caps = v->iS.iCaps;
    printf("Secure ID: %08x\n", v->iS.iSecureId);
    printf("Vendor ID: %08x\n", v->iS.iVendorId);
    printf("Capabilities: %08x %08x\n", (uint32_t)(caps>>32), (uint32_t)caps);

    if(!aCapNames)
        return;

    const Property* p = capabilities;
    while(p->name != nullptr)
    {
        if(caps&p->flag)
            printf("\t\t%s\n", p->name);
        p++;
    }

    printf("\n");
}

void E32Info::CodeSection()
{
    printf("\nCode (text size=%08x)\n", iHdr->iTextSize);
    PrintHexData(iE32->GetImportTable(), iHdr->iCodeSize);

    if (iHdr->iCodeRelocOffset)
    {
        const E32RelocSection *a = iE32->GetRelocSection(iHdr->iCodeRelocOffset);
        DumpRelocs(a);
    }
}

void E32Info::DataSection()
{
    printf("\nData\n");
    PrintHexData(iE32->GetBufferedImage() + iHdr->iDataOffset, iHdr->iDataSize);

    if (iHdr->iDataRelocOffset)
    {
        const E32RelocSection *a = iE32->GetRelocSection(iHdr->iDataRelocOffset);
        DumpRelocs(a);
    }
}

void E32Info::ExportTable()
{
    printf("\nNumber of exports = %u\n", iHdr->iExportDirCount);
    uint32_t* exports = iE32->GetExportTable();
    uint32_t absentVal = iE32->EntryPoint();
    for (uint32_t i = 1; i < iHdr->iExportDirCount; i++)
    {
        if(exports[i] == absentVal)
            printf("\tOrdinal %5u:\tABSENT\n", i);
        else
            printf("\tOrdinal %5u:\t%08x\n", i, exports[i]);
    }
}

void E32Info::ImportTableInfo()
{
    if(!iHdr->iImportOffset)
        return;

    uint32_t impfmt = ImpFmtFromFlags(iHdr->iFlags);
    const E32ImportSection* section = iE32->GetImportSection();
    E32ImportParser* parser = new E32ImportParser(iHdr->iDllRefTableCount, impfmt, section);
    const char* impTable = iE32->GetImportTable();
    const uint32_t* impAddrTable = iE32->GetImportAddressTable();

    printf("\nIdata\tSize=%08x\n", parser->GetSectionSize());
    printf("Offset of import address table (relative to code section): %08x\n", iHdr->iTextSize);

    while(parser->HasImports())
    {
        uint32_t offset = parser->GetOffsetOfDllName();
        const char* dllname = iE32->GetDLLName(offset);
        uint32_t importsCount = parser->GetNumberOfImports();
        printf("%d imports from %s\n", importsCount, dllname);

        if (impfmt == KImageImpFmt_ELF)
        {
            for(uint32_t i = 0; i < importsCount; i++)
            {
                uint32_t impd = *(uint32_t*)(impTable + parser->GetImportOffset(i));
                uint32_t ordinal = impd & 0xffff;
                offset = impd >> 16;

                if (offset)
                    printf("%10u offset by %u\n", ordinal, offset);
                else
                    printf("%10u\n", ordinal);
            }
        }
        else
        {
            while (importsCount--)
                printf("\t%u\n", *impAddrTable++);
        }
        parser->NextImportBlock();
    }
}

void E32Info::SymbolInfo()
{
    if(!(iHdr->iFlags & KImageNmdExpData))
        return;
    const E32EpocExpSymInfoHdr* symInfoHdr = iE32->GetEpocExpSymInfoHdr();
    if(!symInfoHdr)
		return;

    char* symTblBase = (char*)symInfoHdr;
    uint32_t* symAddrTbl = (uint32_t*)(symTblBase + symInfoHdr->iSymbolTblOffset);
    char *symNameTbl = (char*)(symAddrTbl + symInfoHdr->iSymCount);

	printf("\n\n\n\t\tSymbol Info\n");
	if(symInfoHdr->iSymCount)
	{
        char* strTable = symTblBase + symInfoHdr->iStringTableOffset;
        printf("%d Symbols exported\n", symInfoHdr->iSymCount);
        printf("  Addr\t\tName\n");
        printf("----------------------------------------\n");
        size_t nameOffset = 0;
        for(int i = 0; i < symInfoHdr->iSymCount; i++)
		{
			if(symInfoHdr->iFlags & 1)
			{
				uint32_t* offset = ((uint32_t*)symNameTbl+i);
				nameOffset = (*offset << 2);
			}
			else
			{
				uint16_t* offset = ((uint16_t*)symNameTbl+i);
				nameOffset = (*offset << 2);
			}
			printf("0x%08x \t%s\t\n",symAddrTbl[i], strTable + nameOffset);
		}
	}
    else
		printf("No Symbol exported\n");
    printf("\n\n");

	if(!symInfoHdr->iDllCount)
        return;

    const char *e32Buf = iE32->GetBufferedImage();
    // The import table orders the dependencies alphabetically...
    // We need to list out in the link order...
    printf("%d Static dependencies found\n", symInfoHdr->iDllCount);
    uint32_t* depTbl = (uint32_t*)((char*)symInfoHdr + symInfoHdr->iDepDllZeroOrdTableOffset);
    uint32_t* depOffset =  (uint32_t*)((char*)depTbl - e32Buf);

    const E32ImportSection* section = iE32->GetImportSection();
    uint32_t impfmt = ImpFmtFromFlags(iHdr->iFlags);
    E32ImportParser* parser = new E32ImportParser(iHdr->iDllRefTableCount, impfmt, section);

    /* The import table has offsets to the location (in code section) where the
     * import is required. For dependencies pointed by 0th ordinal, this offset
     * must be same as the offset of the dependency table entry (relative to
     * the code section).
     */
    for(int i = 0; i < symInfoHdr->iDllCount; i++)
    {
        bool zerothFound = false;
        while(parser->HasImports())
        {
            const char* dllname = iE32->GetDLLName(parser->GetOffsetOfDllName());
            uint32_t importsCount = parser->GetNumberOfImports();

            if(impfmt == KImageImpFmt_ELF)
            {
                while(importsCount--)//start from the end of the import table
                {
                    uint32_t impd_offset = parser->GetImportOffset(importsCount);
                    uint32_t impd = *(uint32_t*)(iE32->GetImportTable() + impd_offset);
                    uint32_t ordinal = impd & 0xffff;

                    if(ordinal == 0)
                    {
                        if(impd_offset == (uint32_t)((char*)depOffset - iHdr->iCodeOffset))
                        {
                            /* The offset in import table is same as the offset of this
                             * dependency entry
                             */
                            printf("\t%s\n", dllname);
                            zerothFound = true;
                        }
                        break;
                    }
                }
            }
            if(zerothFound)
                break;

            parser->NextImportBlock();
        }
        if(!zerothFound)
            printf("!!Invalid dependency listed at %d\n", i);

        depOffset++;
    }
}

/** \brief This function prints in hex
 *
 * \param *pos - pointer to first element in memory
 * \param length - size of memory block to print
 * \return void
 * This function prints bytes in usual order like ordinal hex viewer.
 * That behavior differ from original algorithm (example with 4-byte block):
 * original - b5102802
 * usual    - 022810b5
 */

void PrintHexData(const void *pos, size_t length)
{
    printf("Block length: %lu\n", (unsigned long)length);
    const unsigned LINE_MAX = 32;
    char *p = (char *)pos;
    char str[LINE_MAX + 1] = {"\n"};

    size_t i = 0;
    for(; (i + LINE_MAX) < length; i += LINE_MAX)
    {
        printf("%06" PRIxMAX ": ", (uintmax_t)i);
        memcpy(str, p + i, LINE_MAX);
        for(size_t j = 0; j < LINE_MAX; j++)
        {
            printf("%02x", (unsigned char)str[j]);

            if( (j+1)%4 == 0)
                printf(" ");

            if( ((size_t)str[j] < 32) || ((size_t)str[j] > 128))
                str[j] = '.';
        }
        printf("%s\n", str);
    }

    if(i==length)
        return;

    size_t diff = length-i;
    memset(str, ' ', LINE_MAX);
    memcpy(str, (p + i), diff);

    printf("%06" PRIxMAX ": ", (uintmax_t)i);
    for(uint32_t j = 0; j < diff; j++)
    {
        printf("%02x", (unsigned char)str[j]);

        if( (i+1)%4 == 0)
            printf(" ");

        if( ((size_t)str[j] < 32) || ((size_t)str[j] > 128))
            str[j] = '.';
    }
    printf("%s\n", str);
}

void E32Info::Run()
{
    auto flags = iParam->iDump;
    if(flags.empty())
        HeaderInfo();

    printf("E32ImageFile \'%s\'\n", iParam->iE32input.c_str());

    // We ignore validate E32 Image when E32Info used for logging on freshly created E32 Image
    if(iParam->iForceE32Build == false)
    {
        ValidateE32Image(iE32);
        CheckE32CRC(iE32, iParam);
    }

    for(auto x: flags)
    {
        switch(x)
        {
            case 'a':
                GenerateAsmFile(iParam);
                break;
            case 'h':
                HeaderInfo();
                break;
            case 's':
                SecurityInfo(true);
                break;
            case 'c':
                CodeSection();
                break;
            case 'd':
                DataSection();
                break;
            case 'e':
                ExportTable();
                break;
            case 'i':
                ImportTableInfo();
                break;
            case 't':
                SymbolInfo();
                break;
            default:
                ReportError(INVALIDARGUMENT, "--dump", flags);
        }
    }
}

void E32Info::CPUIdentifier(uint16_t aCPUType, bool &isARM)
{
    switch (aCPUType)
    {
    case X86Cpu:
        printf("X86 CPU\n");
        break;
    case ArmV4Cpu:
        printf("ARMV4 CPU\n");
        isARM = true;
        break;
    case ArmV5Cpu:
        printf("ARMV5 CPU\n");
        isARM = true;
        break;
    case ArmV6Cpu:
        printf("ARMV6 CPU\n");
        isARM = true;
        break;
    case MCoreCpu:
        printf("M*Core CPU\n");
        break;
    case UnknownCpu: //fallthru
    default:
        printf("Unknown CPU\n");
        break;
    }
}

void DumpRelocs(const E32RelocSection *reloc)
{
    int32_t cnt=reloc->iNumberOfRelocs;
	printf("%d relocs\n", cnt);
	const E32RelocBlock* relocs=reloc->iRelocBlock;
	int32_t printed=0;
	while(cnt > 0)
    {
        int32_t page=relocs->iPageOffset;
		int32_t size=relocs->iBlockSize;
		size-=8;
		const uint16_t *p = relocs->iEntry;
		while (size>0)
		{
			uint32_t a=*p++;
			uint32_t relocType = (a & 0x3000) >> 12;
			if ((relocType == 1) || (relocType == 3)) //only relocation type1 and type3
			{
				printf("%08x(%u) ", page + (a&0x0fff), relocType);
				printed++;
				if (printed>3)
				{
				    printf("\n");
					printed=0;
				}
			}
			size-=2;
			cnt--;
		}
		relocs = (const E32RelocBlock*)p;
    }
    printf("\n");
}

void GenerateAsmFile(const Args *param)
{
    if(param->iDefoutput.empty())
        ReportError(NOREQUIREDOPTION, "--definput");

    const char *defin = param->iDefoutput.c_str();
	Symbols syms = SymbolsFromDef(defin);

	FILE *fptr = nullptr;
	if(!param->iOutput.empty())
        fptr = freopen(param->iOutput.c_str(), "w", stdout);
	if(!fptr)
        printf("Can't store ASM in file! Print to screen.\n");

    for(auto x: syms)
    {
        if(x->Absent())
            continue;
        //Set the visibility of the symbols as default."DYNAMIC" option is
        //added to remove STV_HIDDEN visibility warnings generated by every
        //export during kernel build
        printf("\tIMPORT %s [DYNAMIC]\n", x->AliasName().c_str());
    }

    // Create a directive section that instructs the linker to make all listed
    // symbols visible.
    printf("\n AREA |.directive|, READONLY, NOALLOC\n\n");
    printf("\tDCB \"#<SYMEDIT>#\\n\"\n");

    for(auto x: syms)
    {
        if(x->Absent())
            continue;
        // Example:
        //  DCB "EXPORT __ARM_ll_mlass\n"
        printf("\tDCB \"EXPORT %s\\n\"\n", x->AliasName().c_str());
    }

    printf("\n END\n");
	if(fptr)
        fclose(fptr);
}
