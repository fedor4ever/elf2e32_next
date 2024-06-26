// Copyright (c) 2020-2024 Strizhniou Fiodar
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

#include <memory>
#include <string.h>
#include <algorithm>

#include "symbol.h"
#include "deffile.h"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "symbolprocessor.h"
#include "staticlibsymbols.h"

using std::list;
using std::string;

bool IsGlobalSymbol(const Elf32_Sym* s);
bool IsExportedSymbol(const Elf32_Sym* s, const ElfParser* parser);
bool IsInvalidExport(const char* s);
bool UnCallableSymbol(const string& s);
bool UnWantedSymbol(const char* aSymbol);

uint64_t OrdinalFromString(const string& str);
SymbolType SymbolTypeCodeOrData(const Elf32_Sym* s);

SymbolProcessor::SymbolProcessor(const Args* args, const ElfParser* elfParser):
        iArgs(args), iElfParser(elfParser) {}

SymbolProcessor::~SymbolProcessor()
{
    //dtor
}

bool SortSymbolsByName(const Symbol* first, const Symbol* second){ return first->AliasName() < second->AliasName();}
bool SortSymbolsByOrdinal(const Symbol* first, const Symbol* second){ return first->Ordinal() < second->Ordinal();}

typedef Symbols::iterator Iterator;
void SymbolProcessor::ProcessPredefinedSymbols()
{
    if(iArgs->iDefinput.empty() && iArgs->iSysdef.empty())
        return;
    if(iArgs->iDefinput.empty())
    {
        iSymbols = FromSysdef();
        return;
    }

    if(iArgs->iSysdef.empty())
    {
        iSymbols = SymbolsFromDef(iArgs->iDefinput.c_str());
        return;
    }

    Symbols sysDefSym = FromSysdef();
    iSymbols = SymbolsFromDef(iArgs->iDefinput.c_str());

    uint32_t lastOrdinal = 0;
    if( (*iSymbols.crbegin())->Ordinal() > (*sysDefSym.crbegin())->Ordinal() )
        lastOrdinal = (*iSymbols.crbegin())->Ordinal();
    else
        lastOrdinal = (*sysDefSym.crbegin())->Ordinal();
    lastOrdinal++;

    Iterator nameIt = iSymbols.end(), ordIt = iSymbols.end(), lastIt = iSymbols.end();
    bool eqSymbols = false;
    for(auto x: sysDefSym)
    {
        nameIt = iSymbols.end(), ordIt = iSymbols.end(), lastIt = iSymbols.end();
        for(auto y = iSymbols.begin(); y != lastIt; y++)
        {
            if(*y == x)
            {
                eqSymbols = true;
                break;
            }
            if( (*y)->Ordinal() == x->Ordinal())
                ordIt = y;
            if( (*y)->AliasName() == x->AliasName())
                nameIt = y;
        }

        if(eqSymbols)
        {
            eqSymbols = false;
            continue;
        }

        if(*iSymbols.crbegin() == x)
            continue;

        if( (*iSymbols.crbegin())->AliasName() == x->AliasName() )
        {
            (*iSymbols.crbegin())->SetOrdinal(x->Ordinal());
            continue;
        }

        else if( (nameIt == ordIt) && (nameIt == lastIt) )
            iSymbols.push_back(x);

        else if((nameIt != ordIt) && (x != *lastIt))
        {
            if(nameIt == lastIt)
            {
                (*ordIt)->SetOrdinal(lastOrdinal);
                (*ordIt)->SetSymbolStatus(SymbolStatus::New);
                iSymbols.push_back(x);
            }
            else if(ordIt == lastIt) //simple set new ordinal
            {
                (*nameIt)->SetOrdinal(x->Ordinal());
                (*nameIt)->SetSymbolStatus(SymbolStatus::New);
            }
            else
            {
                (*nameIt)->SetOrdinal(x->Ordinal());
                (*nameIt)->SetSymbolStatus(SymbolStatus::New);

                (*ordIt)->SetOrdinal(lastOrdinal);
                (*ordIt)->SetSymbolStatus(SymbolStatus::New);
            }
            lastOrdinal++;
        }
    }

    if(iArgs->iElfinput.empty())
    {
        list<string> ls;
        for(auto s: iSymbols)
        {
            if(s->GetSymbolStatus() == SymbolStatus::New)
                ls.push_back(s->AliasName());
        }

        string name = iArgs->iDefinput;
        if(name.empty())
            name = "--sysdef";
        CheckForErrors(iArgs->iUnfrozen, ls, name);
    }
}

bool SymbolProcessor::SimpleSymbolsProcessing()
{
    if(iArgs->iDefinput.empty() && iArgs->iSysdef.empty())
    {
        iSymbols = GetElfExports();
        iSymbols.sort(SortSymbolsByName);
        uint32_t ord = 1;
        for(auto x: iSymbols)
        {
            x->SetOrdinal(ord++);
        }
        return true;
    }
    if(iArgs->iDefinput.empty() && iArgs->iElfinput.empty())
    {
        iSymbols = FromSysdef();
        return true;
    }
    if(iArgs->iSysdef.empty() && iArgs->iElfinput.empty())
    {
        iSymbols = SymbolsFromDef(iArgs->iDefinput.c_str());
        return true;
    }
    return false;
}


void SymbolProcessor::ProcessElfSymbols()
{
    if(iArgs->iElfinput.empty())
        return;

    Symbols elfSym = GetElfExports();
    //look for absent symbols and add them
    Symbols absentSymbols, newsymbols;
    iSymbols.sort(SortSymbolsByOrdinal);
    uint32_t lastOrdinal = (*iSymbols.crbegin())->Ordinal();
    lastOrdinal++;

    iSymbols.sort(SortSymbolsByName);

    std::set_difference(iSymbols.begin(), iSymbols.end(), elfSym.begin(), elfSym.end(),
            std::inserter(absentSymbols, absentSymbols.begin()), SortSymbolsByName);

    std::set_difference(elfSym.begin(), elfSym.end(), iSymbols.begin(), iSymbols.end(),
            std::inserter(newsymbols, newsymbols.begin()), SortSymbolsByName);

    // dealing with new symbols
    std::list<string> ls;
    Symbols filtered;
    for(auto x: newsymbols)
    {
        if(UnCallableSymbol(x->AliasName()) && iArgs->iIgnorenoncallable)
            continue;

        if( (iArgs->iCustomdlltarget || iArgs->iExcludeunwantedexports) &&
           UnWantedSymbol(x->AliasName().c_str()) )
            continue;

        x->SetOrdinal(lastOrdinal++);
        if(!DisableLongVerbosePrint())
            ReportWarning(ErrorCodes::UNFROZENSYMBOLADDED, x->AliasName());
        filtered.push_back(x);
    }

    // dealing with absent symbols
    auto m = std::make_unique<Elf32_Sym>();
    Elf32_Sym* dummy = m.get();
    dummy->st_value = iElfParser->EntryPoint();

    if(!absentSymbols.empty())
        iAbsent = true;

    auto it = iSymbols.begin();
    for(auto x: absentSymbols)
    {
        while(it != iSymbols.end())
        {
            if(x == *it)
            {
                (*it)->SetElfSymbol(dummy);
                if((*it)->Absent())
                    break;

// This code leads to different DEF file if used --definput="tests/libcryptou_openssl.def"
// This file doesn't know about removed functions aka "missing symbols".
#if 0
// Creates the same DEF file output as SDK elf2e32. Should processed by SDK tools and result file stores in project folder
                (*it)->SetSymbolStatus(SymbolStatus::Missing);
#else
// Creates DEF file as it already processed by SDK tools. Useful for other build systems
                (*it)->SetAbsent(true);
#endif // 0

                ls.push_back((*it)->AliasName()); // new absent symbols
                break;
            }
            it++;
        }
    }

    MapAbsentWithElfSymbols(elfSym);
    CheckForErrors(iArgs->iUnfrozen, ls, iArgs->iElfinput);
    ls.clear();
    iSymbols.merge(filtered);
}

void SymbolProcessor::CheckForErrors(bool unfrozen, list<string> missedSymbols, const string& src)
{
    if(!iSymbols.empty() && VerboseOut() && !DisableLongVerbosePrint())
    {
        ReportLog("*********************\n");
        ReportLog("Exported symbols:\n");
        for(auto x: iSymbols)
            ReportLog(x->AliasName() + "\n");
        ReportLog("*********************\n");

    }

    if(!missedSymbols.empty())
    {
        if(unfrozen)
            ReportWarning(ErrorCodes::MISSEDFROZENSYMBOLS, missedSymbols.size());
        else
        {
            if(!iArgs->iDefoutput.empty())
            {
                iSymbols.sort(SortSymbolsByOrdinal);
                DefFile def;
                def.WriteDefFile(iArgs->iDefoutput.c_str(), iSymbols);
            }
            ReportError(ErrorCodes::MISSEDFROZENSYMBOLSERROR, missedSymbols, src, missedSymbols.size());
        }
    }
}

/** Symbols marked as absent in the def file but present in the
 elf file are mapped to the symbols present in the elf file and reset the absent flag.
 Also warn for that symbol(s) */
void SymbolProcessor::MapAbsentWithElfSymbols(const Symbols& fromElf)
{
    auto it1 = iSymbols.begin();
    auto it2 = fromElf.begin();

    if(it2 == fromElf.end())
        ReportError(ErrorCodes::ZEROBUFFER, "DLL Elf file has no exports! Check symbol(s) visibility!");

    while((it1 != iSymbols.end()) && (it2 != fromElf.end()))
    {
        if((*it1)->AliasName() > (*it2)->AliasName())
            it2++;
        else if((*it1)->AliasName() < (*it2)->AliasName())
            it1++;
        else if((*it1)->AliasName() == (*it2)->AliasName())
        {
            (*it1)->SetElfSymbol( (*it2)->GetElf32_Sym());
            if( (*it1)->Absent() )
            {
                ReportWarning(ErrorCodes::ABSENTSYMBOLINELF, (*it1)->AliasName());
                (*it1)->SetAbsent(false);
            }
            it1++, it2++;
        }
    }

    while(it1 != iSymbols.end())
    {
        if(!(*it1)->Absent())
            break;
        it1++;
    }

    if(it1 != iSymbols.end())
        ReportError(ErrorCodes::ELF_ST_VALUE);
}

bool IsNoExportEXE(TargetType type)
{
    return type == TargetType::EExe;
}

Symbols SymbolProcessor::GetExports()
{
    /**< SDK versions ignore exported symbols for EXE */
    if(IsNoExportEXE(iArgs->iTargettype))
        return iSymbols;

    if(iArgs->iDSODump)
        return GetDSOSymbols();

    if(SimpleSymbolsProcessing())
        return iSymbols;

    ProcessPredefinedSymbols();
    ProcessElfSymbols();
    iSymbols.sort(SortSymbolsByOrdinal);
    return iSymbols;
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
        sym->SetSymbolStatus(SymbolStatus::Matching);
        if(sym->AliasName().find("_._.absent_export_") != string::npos)
            sym->SetAbsent(true);
        result.push_back(sym);
    }
    return result;
}

Symbols SymbolProcessor::GetElfExports()
{
    Symbols elf;
    if(!iElfParser || IsNoExportEXE(iArgs->iTargettype))
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

    if(elf.empty() && (iArgs->iTargettype != TargetType::EStdExe))
        ReportError(ErrorCodes::ZEROBUFFER, "DLL Elf file has no exports! Check symbol(s) visibility!");
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
        string ord(line.substr(argpos, endpos-argpos + 1));
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
        if(line.find(ord) != string::npos)
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
            msg += ". Ignored because symbols ordinals should starts from 1\n";
            ReportWarning(ErrorCodes::INVALIDARGUMENT, "--sysdef", iArgs->iSysdef);
            ReportLog(msg);
            continue;
        }
        Symbol* s = new Symbol(SymbolType::SymbolTypeCode);
        s->SetName(funcname);
        s->SetOrdinal(ordinalnum);
        s->SetSymbolStatus(SymbolStatus::New);
        sysdef.push_back(s);
    }
    sysdef.sort(SortSymbolsByOrdinal);
    return sysdef;
}

bool SymbolProcessor::IsAbsentFound()
{
    return iAbsent;
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

bool IsImportedSymbol(uint32_t symIdx, const ElfParser* parser)
{
    Elf32_Sym* sym = parser->GetSymbolTableEntity(symIdx);
    if(!IsDefinedSymbol(sym, parser) && IsGlobalSymbol(sym) && HasSymbolDefaultVisibility(sym))
		return true;
	return false;
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
        {
            ReportLog("Unwantedruntimesymbol: ");
            ReportLog(aSymbol);
			return true;
        }
	}
	return false;
}
