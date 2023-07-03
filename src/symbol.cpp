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
#include <iostream>
#include "symbol.h"
#include "common.hpp"

using namespace std;

Elf32_Sym* Dup(const Elf32_Sym* symbol)
{
    Elf32_Sym* tmp = new Elf32_Sym();
    tmp->st_name = symbol->st_name;
    tmp->st_value = symbol->st_value;
    tmp->st_size = symbol->st_size;
    tmp->st_info = symbol->st_info;
    tmp->st_other = symbol->st_other;
    tmp->st_shndx = symbol->st_shndx;
    return tmp;
}

Symbol::Symbol(SymbolType stype): iSymbolType(stype) {}

Symbol::Symbol(const std::string& symbolName, SymbolType type,
   const Elf32_Sym* symbol, uint32_t ordinal):
   iSymbolName(symbolName), iSymbolType(type), iOrdinal(ordinal)
{
    iElfSym = Dup(symbol);
}

Symbol::~Symbol()
{
    delete iElfSym;
}

void Symbol::SetElfSymbol(const Elf32_Sym* symbol)
{
    if(iElfSym)
    {
        delete iElfSym;
        iElfSym = nullptr;
    }
    iElfSym = Dup(symbol);
}

uint32_t Symbol::Elf_st_value() const
{
    return iElfSym->st_value;
}

Elf32_Sym* Symbol::GetElf32_Sym() const
{
    return iElfSym;
}

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
    ReportLog("autor: I never see such file ");
    ReportLog("which has alias name. Please\n");
    ReportLog("open issue at https://github.com/fedor4ever/elf2e32_next\n");
    ReportLog("and give me that file. Thanks.");
	iAliasName = alias;
}

uint32_t Symbol::SymbolSize() const {
	return iSize;
}

void Symbol::SetSymbolSize(uint32_t s) {
	iSize = s;
}

void ReportSymbolType(const Symbol* const s)
{
    ReportLog("with type: ");
    SymbolType t = s->CodeDataType();
    switch(t)
    {
        case SymbolType::SymbolTypeNotDefined:
            ReportLog("SymbolTypeNotDefined");
            break;
        case SymbolType::SymbolTypeCode:
            ReportLog("SymbolTypeCode");
            break;
        case SymbolType::SymbolTypeData:
            ReportLog("SymbolTypeData");
            break;
        default:
            ReportLog("Wrong SymbolType has value: %d", t);
            break;
    }
}

void SymbolInfo(const Symbol* const s)
{
    ReportLog("\n\nSymbol: ");
    ReportLog(s->AliasName());
    ReportLog(" has size: %d and st_value: %d ", s->SymbolSize(), s->Elf_st_value());
    ReportSymbolType(s);
    ReportLog("\n**********************\n");
}
