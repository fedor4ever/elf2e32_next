// Copyright (c) 2018-2020 Strizhniou Fiodar
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
//  check consistency supplied E32 image
//
//

#include <cstdio>
#include <string.h>
#include <assert.h>

#include "common.hpp"
#include "e32common.h"
#include "e32parser.h"
#include "e32validator.h"
#include "e32importsprocessor.hpp"

void ThrowIfTrue(bool condition, const std::string& err)
{
    if(condition)
        ReportError(ErrorCodes::E32IMAGEVALIDATIOFAILURE, err);
}

void ValidateE32Image(const E32Parser* parser)
{
    E32Validator *v = new E32Validator(parser);
    v->ValidateE32Image();
    delete v;
}

E32Validator::E32Validator(const E32Parser* parser):
    iParser(parser)
{
	iHdr = iParser->GetE32Hdr();
	iBufSize = iParser->GetFileSize();
}

E32Validator::~E32Validator(){}

void E32Validator::ValidateE32Image()
{
    iHdrV = iParser->GetE32HdrV();

    ValidateHeader();
    ValidateExportDescription();
    ValidateRelocations(iHdr->iCodeRelocOffset,iHdr->iCodeSize);
	ValidateRelocations(iHdr->iDataRelocOffset,iHdr->iDataSize);
	ValidateImports();
}

const uint32_t KMaxDesSize = 0x0fffffffu; // maximum size of descriptor
void E32Validator::ValidateHeader()
{
    // file size unknown, set to maximum valid so rest of validation works...
	if(iBufSize == 0)
		iBufSize = KMaxDesSize;

    ThrowIfTrue(iBufSize > KMaxDesSize, "file size not negative or too big");

	// check file is big enough to contain this header...
	size_t hdrSize = sizeof(E32ImageHeader);
	ThrowIfTrue(iBufSize < hdrSize,
        "file is big enough to contain E32ImageHeader header");

	uint32_t hdrfmt = HdrFmtFromFlags(iHdr->iFlags);
    if( hdrfmt == KImageHdrFmt_J)
        hdrSize += sizeof(E32ImageHeaderJ);

    if( hdrfmt == KImageHdrFmt_V)
        hdrSize += sizeof(E32ImageHeaderV) + sizeof(E32ImageHeaderJ);

    ThrowIfTrue(iBufSize < hdrSize,
        "file is big enough to contain E32ImageHeaderV header");

	// check header format version...
	// KImageHdrFmt_J, KImageHdrFmt_Original, KImageHdrFmt_V
	if( (hdrfmt != KImageHdrFmt_V) && (hdrfmt != KImageHdrFmt_J) &&
        (hdrfmt != KImageHdrFmt_Original) )
    {
        ThrowIfTrue(true, "header format version");
    }

	// check header size...
	ThrowIfTrue(iHdr->iCodeOffset > iBufSize, "header size. Doesn't fit in file.");

	// check iCpuIdentifier...
	Cpu cpu = (Cpu)iHdr->iCpuIdentifier;
	bool isARM = (cpu==ArmV4Cpu || cpu==ArmV5Cpu || cpu==ArmV6Cpu);
	iPointerAlignMask = isARM ? 3 : 0; // mask of bits which must be zero for aligned pointers/offsets

	uint32_t checksum = GetUidChecksum(iHdr->iUid1, iHdr->iUid2, iHdr->iUid3);
	ThrowIfTrue(checksum != iHdr->iUidChecksum, "UID checksum");

    // check iSignature...
	if( *(uint32_t*)(iHdr->iSignature) != 0x434f5045) // 'EPOC'
        ThrowIfTrue(true, "E32 image signature for that file");

	// check iHeaderCrc...
	char* buf = new char[iHdr->iCodeOffset]();
	memcpy(buf, iHdr, iHdr->iCodeOffset);
	((E32ImageHeader*)buf)->iHeaderCrc = KImageCrcInitialiser;
	uint32_t crc = Crc32(buf, iHdr->iCodeOffset);
	delete[] buf;
	ThrowIfTrue(crc != iHdr->iHeaderCrc, "E32Image header crc");

	// check iModuleVersion...
	uint32_t mv = iHdr->iModuleVersion;
	ThrowIfTrue(mv>=0x80000000u || (mv&0x0000ffffu)>0x8000u, "iModuleVersion");

    // check iCompressionType and get uncompressed size...
    uint32_t compression = iHdr->iCompressionType;
    uint32_t uncompressedSize = iBufSize;

    if(compression!=KFormatNotCompressed)
    {
        uncompressedSize = iParser->UncompressedFileSize();
        ThrowIfTrue( (compression != KUidCompressionDeflate) &&
            (compression != KUidCompressionBytePair), "compression method");
        ThrowIfTrue(uncompressedSize < iHdr->iCodeOffset,
                    "compression size not overflowed 32 bits");
    }

	// check sizes won't overflow the limit for a descriptor (many Loader uses won't like that).
    ThrowIfTrue(uncompressedSize > KMaxDesSize,
                "uncompressed size doesn't overflow the limit for a descriptor");

	// check KImageDll in iFlags...
	if(iHdr->iFlags&KImageDll)
        ThrowIfTrue(iHdr->iUid1 != KDynamicLibraryUidValue, "UID1 in DLL");
	else
        ThrowIfTrue(iHdr->iUid1 != KExecutableImageUidValue, "UID1 in EXE");

	// check iFlags for ABI and entry point types...
	if(isARM)
    {
        ThrowIfTrue((iHdr->iFlags&KImageEptMask) != KImageEpt_Eka2,
            "Eka2 entry point type");
    /// TODO (Administrator#1#11/03/18): Enable checks for other arm targets too!
        #if defined(__EABI__)
        ThrowIfTrue((iFlags&KImageABIMask)!=KImageABI_EABI,
            "EABI ABI");
        #elif defined(__GCC32__)
        ThrowIfTrue((iFlags&KImageABIMask)!=KImageABI_GCC98r2,
            "GCC98r2 ABI");
        #endif
    }
	else{
        ThrowIfTrue(iHdr->iFlags & KImageEptMask,
            "no special entry point type allowed on non-ARM targets");
        ThrowIfTrue(iHdr->iFlags & KImageABIMask,
            "no usage ARM EABI on non-ARM targets");
    }

	// check iFlags for import format...
	ThrowIfTrue((iHdr->iFlags&KImageImpFmtMask) > KImageImpFmt_PE2,
        "check iFlags for import format");

	// check iHeapSizeMin...
	ThrowIfTrue(iHdr->iHeapSizeMin < 0, "check iHeapSizeMin");

	// check iHeapSizeMax...
	ThrowIfTrue(iHdr->iHeapSizeMax < iHdr->iHeapSizeMin, "check iHeapSizeMax");

	// check iStackSize...
	ThrowIfTrue(iHdr->iStackSize < 0, "check iStackSize");

	// check iBssSize...
	ThrowIfTrue(iHdr->iBssSize < 0, "check iBssSize");

	// check iEntryPoint...
	ThrowIfTrue(iHdr->iEntryPoint >= iHdr->iCodeSize,
        "offset to entry point. It has value less or equal to code section size");
    ThrowIfTrue(iHdr->iEntryPoint+KCodeSegIdOffset+sizeof(uint32_t)>iHdr->iCodeSize,
        "iEntryPoint+KCodeSegIdOffset+sizeof(uint32_t) > iHdr->iCodeSize"); //todo: what this check does?
    ThrowIfTrue(iHdr->iEntryPoint & iPointerAlignMask,
        "aligned entry point"); // not aligned

	// check iCodeBase...
	ThrowIfTrue(iHdr->iCodeBase & 3, "aligned iCodeBase"); // not aligned

	// check iDataBase...
	ThrowIfTrue(iHdr->iDataBase & 3, "aligned iDataBase"); // not aligned

	// check iDllRefTableCount...
	ThrowIfTrue(iHdr->iDllRefTableCount < 0, "iDllRefTableCount > 0");
	ThrowIfTrue(iHdr->iDllRefTableCount && !iHdr->iImportOffset,
         "we link to DLLs but have no import data");

	// check iCodeOffset and iCodeSize specify region in file...
	uint32_t codeStart = iHdr->iCodeOffset;
	uint32_t codeEnd = iHdr->iCodeSize+codeStart;
	ThrowIfTrue(codeEnd < codeStart,
        "code section layout. Section can't ends before its start");

    // can't happen because headerSize is defined as iCodeOffset (codeStart)
    // ThrowIfTrue(codeStart<headerSize, "valid code section after E32ImageHeader.");

    ThrowIfTrue(codeEnd > uncompressedSize,
        "code section inside uncompressed file. Section out of range!");

	// check iDataOffset and iDataSize specify region in file...
	uint32_t dataStart = iHdr->iDataOffset;
	uint32_t dataEnd = iHdr->iDataSize + dataStart;
	ThrowIfTrue(dataEnd < dataStart,
        "data section layout. Section can't ends before its start");
    // no data...
    ThrowIfTrue(!dataStart && dataEnd,
        "specified offset for data section in file");

	if(dataStart){
        ThrowIfTrue(!dataEnd, "specified offset for data section end");
        ThrowIfTrue(dataStart < codeEnd,
            "data section begins after code section");
        ThrowIfTrue(dataEnd > uncompressedSize,
            "data section inside uncompressed file. Section out of range!");
        ThrowIfTrue((dataStart-codeStart) & iPointerAlignMask,
            "data aligned with respect to code");
    }

	// check total data size isn't too big...
	uint32_t totalDataSize = iHdr->iDataSize + iHdr->iBssSize;
	ThrowIfTrue(totalDataSize > 0x7fff0000, "total data size isn't too big");

	// check iExportDirOffset and iExportDirCount specify region in code part...
	ThrowIfTrue(uint32_t(iHdr->iExportDirCount) > 65535,
         "E32ImageHeader::iExportDirCount specify too many"
         " exports. Limit(65535) exceeded.");

	if(iHdr->iExportDirCount)
    {
		uint32_t exportsStart = iHdr->iExportDirOffset;
		uint32_t exportsEnd = exportsStart + iHdr->iExportDirCount*sizeof(uint32_t);
		if(iHdr->iFlags&KImageNmdExpData)
			exportsStart -= sizeof(uint32_t); // allow for 0th ordinal
        ThrowIfTrue(exportsEnd < exportsStart,
            "exports section layout. Section can't ends before its start");
        ThrowIfTrue(exportsStart < codeStart,
            "exports section begins before code section");
        ThrowIfTrue(exportsEnd > codeEnd,
            "exports section ends after code section ends");
        ThrowIfTrue((exportsStart-codeStart) & iPointerAlignMask,
            "exports aligned within code section");

        uint32_t* exports = iParser->GetExportTable();
        ThrowIfTrue(exports[0] != iHdr->iExportDirCount,
            "exports size consistency. Export's header and iExportDirOffset size differs.");
    }

	// check iTextSize...
	ThrowIfTrue(iHdr->iTextSize > iHdr->iCodeSize,
        "E32ImageHeader::iTextSize > E32ImageHeader::iCodeSize");

	// check iImportOffset...
	uint32_t start = iHdr->iImportOffset;
	if(start)
    {
		uint32_t end = start+sizeof(E32ImportSection); // minimum valid size
		ThrowIfTrue(end < start,
            "imports section layout. Section can't ends before its start");
        ThrowIfTrue(start < codeEnd,
            "imports section begins before code section ends");
        ThrowIfTrue(end > uncompressedSize,
            "imports section inside uncompressed file. Section out of range!");
        ThrowIfTrue((start-codeEnd)&iPointerAlignMask,
            "alignment import offset within 'rest of data'");
    }

	// check iCodeRelocOffset...
	start = iHdr->iCodeRelocOffset;
	if(start)
    {
		uint32_t end = start+sizeof(E32RelocSection); // minimum valid size
		ThrowIfTrue(end < start,
            "code relocations section layout. Section can't ends before its start");
        ThrowIfTrue(start < codeEnd,
            "code relocations section begins before code section ends");
        ThrowIfTrue(end > uncompressedSize,
            "code relocations section inside uncompressed file. Section out of range!");
        ThrowIfTrue((start-codeEnd)&iPointerAlignMask,
            "alignment code relocation offset within 'rest of data'");
    }

	// check iDataRelocOffset...
	start = iHdr->iDataRelocOffset;
	if(start)
    {
		uint32_t end = start+sizeof(E32RelocSection); // minimum valid size
		ThrowIfTrue(end < start,
            "data relocations section layout. Section can't ends before its start");
        ThrowIfTrue(start < codeEnd,
            "data relocations section begins before code section ends");
        ThrowIfTrue(end > uncompressedSize,
            "data relocations section inside uncompressed file. Section out of range!");
        ThrowIfTrue((start-codeEnd)&iPointerAlignMask,
            "alignment data relocation offset within 'rest of data'");
    }

	// check exception descriptor...
	uint32_t excDesc = iParser->GetE32HdrV()->iExceptionDescriptor;
	if(excDesc&1) // if valid...
        ThrowIfTrue(excDesc >= iHdr->iCodeSize,
            "exception descriptor. Must be not less than E32ImageHeader::iCodeSize");
}

void E32Validator::ValidateExportDescription() const
{
    // check export description...
    uint32_t edSize = iHdrV->iExportDescSize + sizeof(iHdrV->iExportDescSize) +
            sizeof(iHdrV->iExportDescType);
    edSize = (edSize+3)&~3;
    uint32_t edEnd = offsetof(E32ImageHeaderV,iExportDescSize) + edSize +
            sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ);

    if(edEnd != iHdr->iCodeOffset)
    {
        ThrowIfTrue(edEnd > iHdr->iCodeOffset,
            "E32Image. It has gaps between export description and code sections");
        ThrowIfTrue(edEnd < iHdr->iCodeOffset,
            "E32Image. Code section begins in export description");
    }

    // size of bitmap of exports...
    uint32_t bitmapSize = (iHdr->iExportDirCount+7) >> 3;

    // check export description bitmap...
    switch(iHdrV->iExportDescType)
    {
        case KImageHdr_ExpD_NoHoles:
            // no bitmap to check...
            break;

        case KImageHdr_ExpD_FullBitmap:
            // full bitmap present...
            ThrowIfTrue(bitmapSize != iHdrV->iExportDescSize,
                "ExportDescription FullBitmap");
            break;

        case KImageHdr_ExpD_SparseBitmap8: {
            // sparse bitmap present...

            // get size of meta-bitmap...
            uint32_t metaBitmapSize = (bitmapSize+7) >> 3;
            ThrowIfTrue(metaBitmapSize > iHdrV->iExportDescSize,
                "size of meta-bitmap. It  doesn't fit");

            uint32_t totalSize = metaBitmapSize;

            // scan meta-bitmap counting extra bytes which should be present...
            const uint8_t* metaBitmap = iHdrV->iExportDesc;
            const uint8_t* metaBitmapEnd = metaBitmap + metaBitmapSize;
            while(metaBitmap<metaBitmapEnd)
            {
                uint32_t bits = *metaBitmap++;
                do {
                    if(bits&1)
                        ++totalSize; // another byte is present in bitmap
                }
                while(bits>>=1);
            }

            if(totalSize != iHdrV->iExportDescSize)
            {
                ThrowIfTrue(totalSize > iHdrV->iExportDescSize,
                    "sparse bitmap. Some byte(s) exceseed");
                ThrowIfTrue(totalSize < iHdrV->iExportDescSize,
                    "sparse bitmap. Some byte(s) missed");
            }
            break;
            }

        default:
            ThrowIfTrue(true, "ExportDescription. It has unknown value");;
    }
}

void E32Validator::ValidateRelocations(uint32_t offset, uint32_t sectionSize)
{
    if(!offset)
		return; // no relocations

    // read section header (ValidateHeader has alread checked this is OK)...
	const E32RelocSection* sectionHeader = iParser->GetRelocSection(offset);
	int32_t size = sectionHeader->iSize;
	int32_t relocsRemaining = sectionHeader->iNumberOfRelocs;
	ThrowIfTrue(size & 3, "reloc section. It has not multiple of word size");

	// calculate buffer range for block data...
	uint8_t* p = (uint8_t*)(sectionHeader->iRelocBlock);
	uint8_t* sectionEnd = p+size;
	uint8_t* bufferEnd = p + (iBufSize - iHdr->iCodeSize) - 1;

	ThrowIfTrue(sectionEnd < p,
        "buffer range for block data hasn't math overflow"); // math overflow

    ThrowIfTrue(sectionEnd > bufferEnd,
        "reloc section hasn't overflows buffer range for block data"); // overflows buffer

    // process each block...
	while(p!=sectionEnd)
    {
		E32RelocBlock* block = (E32RelocBlock*)p;

		// get address of first entry in this block...
		uint16_t* entryPtr = block->iEntry;
		if((uint8_t*)entryPtr<(uint8_t*)block || (uint8_t*)entryPtr>sectionEnd)
            ThrowIfTrue(true,
                "first entry in relocation block not overflows relocation section");

		// read block header...
		uint32_t pageOffset = block->iPageOffset;
		uint32_t blockSize = block->iBlockSize;
		ThrowIfTrue(pageOffset & 0xfff, "relocation page align"); // not page aligned
		ThrowIfTrue(blockSize < sizeof(E32RelocBlock),
            "relocation block size less than sizeof(E32RelocBlock)"); // blockSize must be at least that of the header just read
        ThrowIfTrue(blockSize & 3, "relocation block size is word aligned");

		// caculate end of entries in this block...
		uint16_t* entryEnd = (uint16_t*)(p + blockSize);
		ThrowIfTrue(entryEnd < entryPtr,
            "pointer address end of this relocation block"
            " isn't less than start due math overflow");
        ThrowIfTrue(entryEnd > (uint16_t*)sectionEnd,
            "pointer address end of this relocation block"
            "doesn't overflow address entire relocation section"); // overflows relocation section

		// process each entry in this block...
		while(entryPtr < entryEnd)
        {
			uint32_t entry = *entryPtr++;
			if(!entry)
				continue;

			// check relocation type...
			uint32_t entryType = entry & 0xf000;
			if(entryType!=KTextRelocType && entryType!=KDataRelocType && entryType!=KInferredRelocType)
				ThrowIfTrue(true, "relocation type");

			// check relocation is within section being relocated...
			uint32_t offset = pageOffset + (entry&0x0fff);
			ThrowIfTrue(offset>=sectionSize || offset+4>sectionSize,
                "relocation is within section"); // not within section
            ThrowIfTrue(offset & iPointerAlignMask,
                "relocation aligned correctly"); // not aligned correctly

			// count each relocation processed...
			--relocsRemaining;
        }

		// next sub block...
		p = (uint8_t*)entryEnd;
    }

	// check number of relocations in section header is correct...
	ThrowIfTrue(relocsRemaining,
         "number of relocations in section header is correct");
}

void E32Validator::ValidateImports() const
{
    if(!iHdr->iImportOffset)
		return; // no imports

    // buffer pointer to read relocation from...
    uint8_t* buf = (uint8_t*)iParser->GetBufferedImage();
	uint8_t* bufferEnd = buf + iBufSize; //last byte of E32Image

    // read section header (ValidateHeader has alread checked this is OK)...
	const E32ImportSection* sectionHeader = iParser->GetImportSection();

	// check section lies within buffer...
	uint8_t* sectionStart = (uint8_t*)sectionHeader; // start of first import block
	uint8_t* sectionEnd = sectionStart + sectionHeader->iSize;
	ThrowIfTrue(sectionEnd < sectionStart,
        "import section hasn't math overflow or big enough to contain header");
    ThrowIfTrue(sectionEnd > bufferEnd, "import section hasn't overflows buffer");

	// process each import block...
	uint32_t numDeps = iHdr->iDllRefTableCount;
	uint32_t totalImports = 0;
	uint32_t importFormat = iHdr->iFlags&KImageImpFmtMask;

	uint8_t* p = (uint8_t*)sectionHeader->iImportBlock;
    while(numDeps--)
    {
		// get block header...
		const E32ImportBlock* block = (E32ImportBlock*)p;
		totalImports += block->iNumberOfImports;

		p = (uint8_t*)block->iImports;
		ThrowIfTrue(p<(uint8_t*)block || p>sectionEnd,
            "buffer without overflows in E32ImportBlock");

		// check import dll name is within section...
		uint8_t* name = sectionStart+block->iOffsetOfDllName;
		ThrowIfTrue(name<sectionStart || name>=sectionEnd,
              "import dll name within import section");

        while(*name++ && name<sectionEnd){}

        ThrowIfTrue(name[-1], "import dll name(s) doesn't overflows import section");

		// process import count...
		ThrowIfTrue(block->iNumberOfImports >= 0x80000000u/sizeof(uint32_t),
            "number of imports fit into a signed integer");

		// process import data...

		// PE2 doesn't have any more data...
		if(importFormat==KImageImpFmt_PE2)
			continue;

        // get import data range...
		const uint32_t* imports = block->iImports;
		const uint32_t* importsEnd = imports + block->iNumberOfImports;
		ThrowIfTrue(importsEnd < imports,
            "math hasn't overflow in import data range");

        ThrowIfTrue(importsEnd > (uint32_t*)sectionEnd,
            "buffer doesn't overflows in import data range");

		if(importFormat==KImageImpFmt_ELF)
        {
			// check imports are in code section...
			uint32_t limit = iHdr->iCodeSize-sizeof(uint32_t);
			while(imports < importsEnd)
            {
				uint32_t i = *imports++;
				ThrowIfTrue(i > limit, "imports are in code section");
				ThrowIfTrue(i&iPointerAlignMask, "imports are word aligned");
            }
        }
		else if(importFormat == KImageImpFmt_PE)
        {} // import data is not used, so don't bother checking it
		else
            ThrowIfTrue(true, "import format");

		// move pointer on to next block...
		p = (uint8_t*)imports;
    }

	// done processing imports; for PE derived files now check import address table (IAT)...
	if(importFormat==KImageImpFmt_PE || importFormat==KImageImpFmt_PE2)
    {
        ThrowIfTrue(totalImports >= 0x80000000u/sizeof(uint32_t),
            "imports count fit into a signed integer");
		uint32_t iat = iHdr->iTextSize; // import address table
        // Fuzzer can't trigger this because PE imports are for
        // X86 which doesn't have alignment restrictions
		ThrowIfTrue(iat & iPointerAlignMask,
            "PE imports are for X86 which doesn't have alignment restrictions");

		uint32_t iatEnd = iat+sizeof(uint32_t)*totalImports;
		ThrowIfTrue(iatEnd<iat || iatEnd>iHdr->iCodeSize,
            "import address table doesn't overflows code part of file");
    }
}
