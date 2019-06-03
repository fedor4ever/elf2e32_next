// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2019 Strizhniou Fiodar.
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors: Strizhniou Fiodar - fix build and runtime errors, refactoring.
//
// Description:
// Implementation of the Class Symbol for the elf2e32 tool
// @internalComponent
// @released
//
//


#if !defined(_SYMBOL_H_)
#define _SYMBOL_H_

#include <string>
#include "elfdefs.h"

enum SymbolStatus {Matching,Missing,New};

enum SymbolType
{
	SymbolTypeNotDefined = STT_NOTYPE,
	SymbolTypeData = STT_OBJECT,
	SymbolTypeCode = STT_FUNC
};

/**
 * This class is shared among all that use the symbol information.
 * To be finalized by DefFile.
 */
class Symbol
{

public:

    Symbol(SymbolType stype);
	~Symbol();

	bool operator==(const Symbol* aSym) const;
	const char* SymbolName() const;
	const char* ExportName();
	uint32_t OrdNum() const;
	SymbolType CodeDataType();

	bool R3unused();
	bool Absent();
	void SetAbsent(bool absent);
	std::string Comment();
	int GetSymbolStatus();
	void SetOrdinal(uint32_t ordinalNum);
	void SetSymbolStatus(SymbolStatus symbolStatus);
	void SetSymbolName(const std::string& symbolName);

	void Comment(const std::string& comment);
	void CodeDataType(SymbolType type);
	void R3Unused(bool R3Unused);
	void ExportName(char* exportName);
	void SetSymbolSize(uint32_t size);
	uint32_t SymbolSize();

    Elf32_Sym	*iElfSym = nullptr;
	/**
	 * The index of this symbol in the symbol table(required for the hash table while
	 * creating the dso).
	 */
	uint32_t		iSymbolIndex = 0;

private:
	SymbolStatus    iSymbolStatus = Missing;
	std::string		iSymbolName;
	std::string		iExportName;
	SymbolType	    iSymbolType = SymbolTypeNotDefined;
	uint32_t	    iOrdinalNumber  = -1;
	std::string		iComment;
	bool		    iAbsent = false;
	bool		    iR3Unused = false;
	uint32_t	    iSize = 0;
};

#endif // !defined(_SYMBOL_H_)
