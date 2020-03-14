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

#include <cstring>
#include "symbol.h"

Symbol::Symbol(SymbolType stype): iSymbolType(stype) {}

Symbol::Symbol(const std::string& symbolName, SymbolType type,
   const Elf32_Sym* symbol, uint32_t ordinal): iElfSym(symbol),
   iSymbolName(symbolName), iSymbolType(type), iOrdinal(ordinal) {}


Symbol::~Symbol() {}

bool Symbol::operator==(const Symbol* s) const {
	if(this->iSymbolName.compare(s->iSymbolName) != 0)
		return false;
	if(this->iSymbolType != s->iSymbolType)
		return false;

	return true;
}

bool Symbol::operator!=(const Symbol* s) const
{
    return !operator==(s);
}


void Symbol::SetName(const std::string& s){
    iSymbolName = s;
}

std::string Symbol::Name() const {
	return iSymbolName;
}


uint32_t Symbol::Ordinal() const {
	 return iOrdinal;
}

void Symbol::SetOrdinal(uint32_t ord) {
	iOrdinal = ord;
}


SymbolType Symbol::CodeDataType() const {
	return iSymbolType;
}

void Symbol::SetCodeDataType(SymbolType type) {
	iSymbolType = type;
}


bool Symbol::R3unused() const {
	return iR3Unused;
}

void Symbol::SetR3Unused(bool aR3Unused) {
	iR3Unused = aR3Unused;
}


bool Symbol::Absent() const {
	return iAbsent;
}

void Symbol::SetAbsent(bool aAbsent) {
	iAbsent = aAbsent;
}


///This function returns the comment against this def file.
std::string Symbol::DefFileComment() const {
	return iComment;
}

///This function sets the comment in .def file against the symbol.
void Symbol::SetDefFileComment(const std::string& s) {
	iComment = s;
}


///This function returns the symbol is a matching/missing/new symbol in the def file.
int Symbol::GetSymbolStatus() const {
	return  iSymbolStatus;
}

/**
This function sets the status of the symbol i.e., whether it is
a matching/missing/new symbol.
*/
void Symbol::SetSymbolStatus(SymbolStatus status) {
	iSymbolStatus = status;
}

// use this function
std::string Symbol::AliasName() const {
    if(iAliasName.empty())
        return iSymbolName;
    return iAliasName;
}

///This function sets the export name of the symbol found in .def file.
void Symbol::SetAliasName(const std::string& alias) {
	iAliasName = alias;
}


uint32_t Symbol::SymbolSize() const {
	return iSize;
}

void Symbol::SetSymbolSize(uint32_t s) {
	iSize = s;
}
