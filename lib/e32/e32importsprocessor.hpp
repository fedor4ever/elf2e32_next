// Copyright (c) 2018 Strizhniou Fiodar
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

struct E32RelocSection
{
    int32_t iSize;                 // size of this relocation section
    int32_t iNumberOfRelocs;       // number of relocations in this section
};

/**
Header for the Import Section in an image, as referenced by E32ImageHeader::iImportOffset.
Immediately following this structure are an array of E32ImportBlock structures.
The number of these is given by E32ImageHeader::iDllRefTableCount.
*/
struct E32ImportSection
{
    int32_t iSize;     ///< Size of this section excluding 'this' structure
//  E32ImportBlock iImportBlock[iDllRefTableCount];
};

/**
A block of imports from a single executable.
These structures are conatined in a images Import Section (E32ImportSection).
*/
struct E32ImportBlock
{
    const E32ImportBlock* NextBlock(uint32_t aImpFmt) const;
    uint32_t Size(uint32_t aImpFmt) const;
    const uint32_t* Imports() const;    // import list if present

    uint32_t iOffsetOfDllName;           ///< Offset from start of import section for a NUL terminated executable (DLL or EXE) name.
    int32_t    iNumberOfImports;           ///< Number of imports from this executable.
//  uint32_t   iImport[iNumberOfImports];  ///< For ELF-derived executes: list of code section offsets. For PE, list of imported ordinals. Omitted in PE2 import format
};


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
// uint16_t iEntry[]
};

#endif // E32IMPORTPROCESSOR_H_INCLUDED
