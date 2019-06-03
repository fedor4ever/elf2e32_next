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

#include <cstring>
#include "symbol.h"

Symbol::Symbol(SymbolType stype): iSymbolType(stype) {}

Symbol::~Symbol() {}

bool Symbol::operator==(const Symbol* aSym) const {
	if(iSymbolName.compare(aSym->iSymbolName) != 0)
		return false;
	if( iSymbolType != aSym->iSymbolType )
		return false;

	return true;
}

void Symbol::SetSymbolName(const std::string& symbolName){
    iSymbolName = symbolName;
}

const char* Symbol::SymbolName() const {
	return iSymbolName.c_str();
}

///This function returns the aliased symbol name.
const char* Symbol::ExportName() {
	 return iExportName.c_str();
}

///This function returns the ordinal number of the symbol.
uint32_t Symbol::OrdNum() const {
	 return iOrdinalNumber;
}

SymbolType Symbol::CodeDataType() {
	return iSymbolType;
}

///This function returns if r3unused is true.
bool Symbol::R3unused() {
	return iR3Unused;
}

///This function returns if the symbol is marked absent in the def file.
bool Symbol::Absent() {
	return iAbsent;
}

///This function sets the symbol to be absent.
void Symbol::SetAbsent(bool aAbsent) {
	iAbsent = aAbsent;
}

///This function returns the comment against this def file.
std::string Symbol::Comment() {
	return iComment;
}

///This function returns the symbol is a matching/missing/new symbol in the def file.
int Symbol::GetSymbolStatus() {
	return  iSymbolStatus;
}

///This function sets the ordinal number for this symbol.
void Symbol::SetOrdinal(uint32_t aOrdinalNum) {
	iOrdinalNumber=aOrdinalNum;
}

/**
This function sets the status of the symbol i.e., whether it is
a matching/missing/new symbol.
*/
void Symbol::SetSymbolStatus(SymbolStatus status) {
	iSymbolStatus = status;
}

///This function sets the export name of the symbol.
void Symbol::ExportName(char *exportName)
{
	iExportName = exportName;
}

///This function sets the comment against the symbol.
void Symbol::Comment(const std::string &comment)
{
	iComment = comment;
}

///This function sets the symbol type if it is Code or Data symbol.
void Symbol::CodeDataType(SymbolType aType)
{
	iSymbolType = aType;
}

///This function sets if R3Unused is true for this symbol.
void Symbol::R3Unused(bool aR3Unused)
{
	iR3Unused = aR3Unused;
}

///This function sets the size of this symbol.
void Symbol::SetSymbolSize(uint32_t aSize){
	iSize = aSize;
}

///This function gets the size of this symbol.
uint32_t Symbol::SymbolSize(){
	return iSize;
}
