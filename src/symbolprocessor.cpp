// Copyright (c) 2020 Strizhniou Fiodar
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
// Got Symbols from ELF file, Def file and --sysdef option.
//
//

#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <algorithm>

#include "symbol.h"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "symbolprocessor.h"
#include "staticlibsymbols.h"

using std::string;
using std::cout;

bool IsGlobalSymbol(const Elf32_Sym* s);
bool IsExportedSymbol(const Elf32_Sym* s, const ElfParser* parser);
bool IsInvalidExport(const char* s);
bool UnCallableSymbol(const string& s);

uint64_t OrdinalFromString(const string& str);
SymbolType SymbolTypeCodeOrData(const Elf32_Sym* s);

SymbolProcessor::SymbolProcessor(const ElfParser* elfParser, const Args* args):
        iElfParser(elfParser), iArgs(args) {}

SymbolProcessor::~SymbolProcessor()
{
    //dtor
}

bool SortSymbolsByName(Symbol* first, Symbol* second){ return first->AliasName() < second->AliasName();}

std::pair<std::string, Symbol*> pairifyByName(Symbol* a) { return std::make_pair(a->AliasName(), a); }
std::pair<uint32_t, Symbol*> pairifyByOrdinal(Symbol* a) { return std::make_pair(a->Ordinal()  , a); }

typedef std::map<string, Symbol*> StringMap;
typedef std::map<uint32_t, Symbol*> OrdinalMap;
Symbols SymbolProcessor::GetExports()
{
    if(iArgs->iDSODump)
        return GetDSOSymbols();

    StringMap result;
    OrdinalMap ordinals;

    Symbols sysDefSym = FromSysdef();
    Symbols defSym = SymbolsFromDef(iArgs->iDefinput.c_str());

    std::transform(defSym.begin(), defSym.end(), std::inserter(result,   result.end())  , pairifyByName);
    std::transform(defSym.begin(), defSym.end(), std::inserter(ordinals, ordinals.end()), pairifyByOrdinal);

    auto rt = result.crbegin();
    uint32_t nextOrdinal = rt->second->Ordinal();
    nextOrdinal++;

    for(auto x: sysDefSym)
    {
        auto defSym = result.find(x->AliasName());
        auto ordSym = ordinals.find(x->Ordinal());

        bool notFoundByName = (defSym == result.end());
        bool notFoundByOrdinal = (ordSym == ordinals.end());

        bool foundByName = (defSym != result.end());
        bool foundByOrdinal = (ordSym != ordinals.end());
//        Symbol may set symbol with new ordinal
        if(notFoundByOrdinal)
        {
            //adding completely new symbol
            if(notFoundByName)
            {
                x->SetSymbolStatus(New);
                result.insert(std::pair<string, Symbol*>(x->AliasName(), x) );
                ordinals.insert(std::pair<uint32_t, Symbol*>(x->Ordinal(), x) );
            }
            //simple set new ordinal
            else if(foundByName)
            {
                ordinals.insert(std::pair<uint32_t, Symbol*>(x->Ordinal(), x) );
                ordinals.erase(defSym->second->Ordinal());
                defSym->second->SetOrdinal(x->Ordinal());
            }
        }

//      new symbol added as is, symbol with ordinal we found take new ordinal as next after last ordinal
        else if(foundByOrdinal && notFoundByName)
        {
// add new symbol to sorted by name std::map
            result.insert(std::pair<string, Symbol*>(x->AliasName(), x) );
// look for symbol with ordinal specified in --sysdef and update sorted by name std::map
            auto currentSymbol = result.find(ordSym->second->AliasName());
            currentSymbol->second->SetOrdinal(nextOrdinal);

// update sorted by ordinals std::map
            string oldName = ordSym->second->Name();
            string oldAliasName = ordSym->second->AliasName();
            ordSym->second->SetName(x->Name());

            ordSym->second->SetOrdinal(x->Ordinal());
            x->SetOrdinal(nextOrdinal);
            x->SetName(oldName);
            if(oldAliasName != oldName)
                x->SetAliasName(oldAliasName);
            ordinals.insert(std::pair<uint32_t, Symbol*>(nextOrdinal++, x) );
        }
        else if(foundByOrdinal && foundByName)
        {
            if(ordSym->second->Absent())
            {
                ReportWarning(ErrorCodes::ABSENTSYMBOL, ordSym->second->Name());
                ordSym->second->SetAbsent(false);
            }
            if(defSym->second->Absent())
            {
                ReportWarning(ErrorCodes::ABSENTSYMBOL, defSym->second->Name());
                defSym->second->SetAbsent(false);
            }
            if(defSym->second == ordSym->second)
                continue;

            //update ordinals
            uint32_t srcOrdinal = ordSym->second->Ordinal();
            defSym->second->SetOrdinal(x->Ordinal());
            ordSym->second->SetOrdinal(nextOrdinal++);

            //update names
            ordinals.insert(std::pair<uint32_t, Symbol*>(nextOrdinal, ordSym->second) );
            ordinals.erase(srcOrdinal);
        }
    }

    Symbols elfSym = FromElf();

    //look for absent symbols and add them
    Symbols toCompare, missedSymbols, absentSymbols;
    toCompare.insert(toCompare.end(), sysDefSym.begin(), sysDefSym.end());
    toCompare.insert(toCompare.end(), defSym.begin(), defSym.end());
    toCompare.sort(SortSymbolsByName);
    auto itDef = std::set_difference(toCompare.begin(), toCompare.end(),
        elfSym.begin(), elfSym.end(), missedSymbols.begin(), SortSymbolsByName);

    std::list<string> ls;
    for(auto x: missedSymbols)
    {
        if(x->Absent())
            continue;
        auto it = result.find(x->AliasName());
        it->second->SetAbsent(true);
        absentSymbols.push_back(x);
        ls.push_back(x->AliasName());
    }
    if(!absentSymbols.empty())
    {
        if(iArgs->iUnfrozen)
            ReportWarning(ErrorCodes::MISSEDFROZENSYMBOLS ,absentSymbols.size());
        else
            ReportError(ErrorCodes::MISSEDFROZENSYMBOLSERROR, ls, iArgs->iElfinput, absentSymbols.size());
    }

    for(auto x: absentSymbols)
    {
        auto it = result.find(x->AliasName());
        it->second->SetAbsent(true);
    }

    //ordinals in elf symbols not set so we do
    for(auto x: elfSym)
    {
        if( (iArgs->iExcludeunwantedexports || iArgs->iCustomdlltarget) && UnCallableSymbol(x->AliasName()))
            continue;

        auto it = result.find(x->AliasName());
        bool foundByName = (it != result.end());

        if(foundByName && it->second->Absent())
        {
            it->second->SetAbsent(false);
            ReportWarning(ErrorCodes::ABSENTSYMBOL, it->second->AliasName());
            continue;
        }

        x->SetOrdinal(nextOrdinal++);
        result[x->AliasName()] = x;
    }

    Symbols out;
    std::transform(
    ordinals.begin(),
    ordinals.end(),
    std::back_inserter(out),
    [](auto &kv){ return kv.second;}
    );

    return out;
}

Symbols SymbolProcessor::GetDSOSymbols()
{
    Symbols result;
    uint32_t numOfImports = iElfParser->ImportsCount();
    uint32_t* ordinals =  iElfParser->GetDSOImportsOrdinals();
    for(uint32_t i = 0; i < numOfImports; i++)
    {
        const char* symName = iElfParser->GetSymbolNameFromStringTable(i);
        if(IsInvalidExport(symName))
            continue;

        Elf32_Sym* symTableEntity = iElfParser->GetSymbolTableEntity(i);
        if(!IsExportedSymbol(symTableEntity, iElfParser))
            continue;

        SymbolType type = SymbolTypeCodeOrData(symTableEntity);
        Symbol* sym = new Symbol(symName, type, symTableEntity, ordinals[i - 1]);
        sym->SetSymbolSize(symTableEntity->st_size);
        result.push_back(sym);
    }
    return result;
}

Symbols SymbolProcessor::FromElf()
{
    Symbols elf;
    if(!iElfParser)
        return elf;

    uint32_t numOfImports = iElfParser->ImportsCount();
    for(uint32_t i = 0; i < numOfImports; i++)
    {
        const char* symName = iElfParser->GetSymbolNameFromStringTable(i);
        if(IsInvalidExport(symName))
            continue;

        Elf32_Sym* symTableEntity = iElfParser->GetSymbolTableEntity(i);
        if(!IsExportedSymbol(symTableEntity, iElfParser))
            continue;

        SymbolType type = SymbolTypeCodeOrData(symTableEntity);
        Symbol* sym = new Symbol(symName, type, symTableEntity, i);
        sym->SetSymbolSize(symTableEntity->st_size);
        sym->SetSymbolStatus(SymbolStatus::New);
        elf.push_back(sym);
    }
    elf.sort(SortSymbolsByName);
    return elf;
}

Symbols SymbolProcessor::FromSysdef()
{
    Symbols sysdef;
    uint64_t ordinalnum = 0;
    string line(iArgs->iSysdef);

    while(!line.empty())
    {
        size_t argpos = line.find_first_of(",");
        if(argpos == string::npos) ReportError(INVALIDARGUMENT, "--sysdef", iArgs->iSysdef);
        size_t endpos = line.find_first_of(";");
        if(endpos == string::npos) ReportError(INVALIDARGUMENT, "--sysdef", iArgs->iSysdef);

        string funcname(line.substr(0, argpos));
        string ordnum(line.substr(argpos+1, endpos-argpos-1));
        line.erase(0, endpos+1);

        if(line.find(funcname) != string::npos)
        {
            string tmp("Find duplicate function name: ");
            tmp += funcname;
            tmp += " with ordinal: ";
            tmp += ordnum;
            tmp += "\n";
            ReportLog(tmp);
            continue;
        }
        if(line.find(ordnum) != string::npos)
        {
            string tmp("Find duplicate function ordinal: ");
            tmp += ordnum;
            tmp += "with name: ";
            tmp += funcname;
            tmp += "\n";
            ReportLog(tmp);
            continue;
        }

        ordinalnum = OrdinalFromString(ordnum);
        if(ordinalnum == 0)
        {
            string msg("Found symbol: ");
            msg += funcname;
            msg += ": ";
            msg += ordnum;
            msg += ". Ignored because symbols ordinals starts from 1\n";
            ReportWarning(ErrorCodes::INVALIDARGUMENT, "--sysdef", iArgs->iSysdef);
            ReportLog(msg);
            continue;
        }
        Symbol* s = new Symbol(SymbolType::SymbolTypeCode);
        s->SetName(funcname);
        s->SetOrdinal(ordinalnum);
        sysdef.push_back(s);
    }
    return sysdef;
}

uint64_t OrdinalFromString(const string& str)
{
    char* p;
    uint64_t res = strtol(str.c_str(), &p, 10);
    if(*p != 0) ReportError(INVALIDARGUMENT, "--sysdef", str);
    return res;
}

bool IsInvalidExport(const char* s)
{
	return (strncmp(s, "_ZTS", strlen("_ZTS")) == 0);
}

bool IsGlobalSymbol(const Elf32_Sym* s)
{
	return (ELF32_ST_BIND(s->st_info) == STB_GLOBAL);
}

bool IsDefinedSymbol(const Elf32_Sym* s, const ElfParser* parser)
{
	if(s->st_shndx == SHN_UNDEF)
		return false;
	ESegmentType aType = parser->SegmentType(s->st_value);
	return ((aType == ESegmentRO) || (aType == ESegmentRW));
}

bool HasSymbolDefaultVisibility(const Elf32_Sym* s)
{
	return (STV_DEFAULT == ELF32_ST_VISIBILITY(s->st_other) ||
         STV_PROTECTED == ELF32_ST_VISIBILITY(s->st_other));
}

bool IsFunctionSymbol(const Elf32_Sym* s)
{
	return (STT_FUNC == ELF32_ST_TYPE(s->st_info));
}

bool IsDataSymbol(const Elf32_Sym* s)
{
	return (STT_OBJECT == ELF32_ST_TYPE(s->st_info));
}

bool IsExportedSymbol(const Elf32_Sym* sym, const ElfParser* parser)
{
    uint32_t index = sym->st_shndx;
    if( (index < SHN_ABS) && (index != SHN_UNDEF) &&
       IsGlobalSymbol(sym) && HasSymbolDefaultVisibility(sym) && IsDefinedSymbol(sym, parser) &&
       (IsFunctionSymbol(sym) || IsDataSymbol(sym) ) )
    {
        return true;
    }
    return false;
}

SymbolType SymbolTypeCodeOrData(const Elf32_Sym* s)
{
    if(STT_FUNC == ELF32_ST_TYPE(s->st_info))
        return SymbolType::SymbolTypeCode;
    else
        return SymbolType::SymbolTypeData;
}

bool UnCallableSymbol(const string& s)
{
    return ((s.find("_ZTI") != string::npos) || (s.find("_ZTV") != string::npos));
}

/**
Function checks if new symbols are present in the static library list
*/
bool UnWantedSymbol(const char* aSymbol)
{
	constexpr size_t symbollistsize = sizeof(Unwantedruntimesymbols) / sizeof(Unwantedruntimesymbols[0]);
	for (size_t i = 0; i<symbollistsize; i++)
	{
		if (strstr(Unwantedruntimesymbols[i], aSymbol))
			return true;
	}
	return false;
}
