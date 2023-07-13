// Copyright (c) 2020 Strizhniou Fiodar.
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Description:
// This class holds advanced symbol info
//
//
// This class provides 3 functions to obtain symbol name:
//   Name() - return it raw name
//   AliasName() - symbol name used in exports symbol table
//   RawAliasName() - alias for symbol name used in .def file
// Therefore use AliasName() where possible.
//
// This class provides 2 functions to set symbol name:
//   SetName() - implicitly set alias name too
//   SetAliasName() - directly set alias name
// Therefore use SetName() where possible.
//

#if !defined(SYMBOL_H)
#define SYMBOL_H

#include <string>
#include "elfdefs.h"

struct Elf32_Sym;

enum SymbolStatus {Matching, Missing, New};

enum SymbolType
{
	SymbolTypeNotDefined = STT_NOTYPE,
	SymbolTypeData = STT_OBJECT,
	SymbolTypeCode = STT_FUNC
};

class Symbol
{

public:

    Symbol(SymbolType type);
    Symbol(const std::string& symbolName, SymbolType type, const Elf32_Sym* symbol, uint32_t ordinal);
	~Symbol();

	bool operator==(const Symbol* aSym) const;
	bool operator!=(const Symbol* aSym) const;

	uint32_t Ordinal() const;
	void SetOrdinal(uint32_t ordinalNum);

	bool Absent() const;
	void SetAbsent(bool absent);

	std::string Name() const;
	void SetName(const std::string& symbolName);

	std::string AliasName() const;
	std::string RawAliasName() const;
	void SetAliasName(const std::string& symbolName);

	int GetSymbolStatus() const;
	void SetSymbolStatus(SymbolStatus symbolStatus);

	std::string DefFileComment() const;
	void SetDefFileComment(const std::string& comment);

	SymbolType CodeDataType() const;
	void SetCodeDataType(SymbolType type);

	bool R3unused() const;
	void SetR3Unused(bool R3Unused);

	uint32_t SymbolSize() const;
	void SetSymbolSize(uint32_t size);

	void SetElfSymbol(const Elf32_Sym* symbol);
	Elf32_Sym* GetElf32_Sym() const;
	uint32_t Elf_st_value() const;
private:
    Elf32_Sym* iElfSym = nullptr;

	SymbolStatus    iSymbolStatus = SymbolStatus::Missing;
	std::string		iSymbolName;
	std::string		iAliasName;
	SymbolType	    iSymbolType = SymbolTypeNotDefined;
	uint32_t	    iOrdinal  = -1;
	std::string		iComment;
	bool		    iAbsent = false;
	bool		    iR3Unused = false;
	uint32_t	    iSize = 0;
};

void SymbolInfo(const Symbol* const s);

#endif // !defined(SYMBOL_H)
