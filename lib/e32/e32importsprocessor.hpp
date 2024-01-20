// Copyright (c) 2018-2024 Strizhniou Fiodar
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
// Process imports for the elf2e32 tool
//
//

#ifndef E32IMPORTPROCESSOR_H_INCLUDED
#define E32IMPORTPROCESSOR_H_INCLUDED

#include <cstdint>
#include <cstddef>

/**
A block of relocations for a single page (4kB) of code/data.

Immediately following this structure are an array of uint16_t values
each representing a single value in the page which is to be relocated.
The lower 12 bits of each entry is the offset, in bytes, from start of this page.
The Upper 4 bits are the relocation type to be applied to the 32-bit value located
at that offset.
 - 1 means relocate relative to code section.
 - 2 means relocate relative to data section.
 - 3 means relocate relative to code or data section; calculate which.

A value of all zeros (0x0000) is ignored. (Used for padding structure to 4 byte alignment).
*/
struct E32RelocBlock
{
	uint32_t iPageOffset; ///< Offset, in bytes, for the page being relocated; relative to the section start. Always a multiple of the page size: 4096 bytes.
	uint32_t iBlockSize;  ///< Size, in bytes, for this block structure. Always a multiple of 4.
    uint16_t iEntry[1];
};

struct E32RelocSection
{
    int32_t iSize;                 // size of this relocation section
    int32_t iNumberOfRelocs;       // number of relocations in this section
    E32RelocBlock iRelocBlock[1];
};

struct E32ImportBlockPE2
{
    uint32_t iOffsetOfDllName = 0;    ///< Offset from start of import section for a NULL terminated executable (DLL or EXE) name.
    int32_t  iNumberOfImports = 0;    ///< Number of imports from this executable.
};

struct E32ImportBlock
{
    uint32_t Size(uint32_t aImpFmt) const;
    uint32_t iOffsetOfDllName = 0;        ///< Offset from start of import section for a NULL terminated executable (DLL or EXE) name.
    int32_t  iNumberOfImports = 0;        ///< Number of imports from this executable.
//  uint32_t  iImport[iNumberOfImports];  ///< For ELF-derived executes: list of code section offsets. For PE, list of imported ordinals. Omitted in PE2 import format
    const uint32_t iImports[1];
};

struct E32ImportSection
{
    int32_t iSize;     ///< Size of E32 Import Section in bytes
    const E32ImportBlock iImportBlock[1]; //iImportBlock[iDllRefTableCount]
};

class E32ImportParser
{
public:
    E32ImportParser(uint32_t importCount, uint32_t importFormat,
    		const E32ImportSection* section);

    void NextImportBlock();
    bool HasImports() const;
    uint32_t GetOffsetOfDllName() const;
    uint32_t GetSectionSize() const;
    uint32_t GetNumberOfImports() const;
    uint32_t GetImportOrdinal() const;
    uint32_t GetImportOffset(uint32_t index) const;
private:
    uint32_t iImportCounter = 0;
    uint32_t iImportFormat = 0;
    const E32ImportSection* iSection = nullptr;
    const E32ImportBlock* iNext = nullptr;
    uint32_t iImpOrdinal = 0;
};

#endif // E32IMPORTPROCESSOR_H_INCLUDED
