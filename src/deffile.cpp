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
// Implementation of the Class DefFile for the elf2e32 tool
// @internalComponent
// @released
//
//

//
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "symbol.h"
#include "deffile.h"
#include "common.hpp"

using std::string;
using std::fstream;

void WriteDefString(Symbol *sym, std::fstream &fstr);
void TokensChecker(const std::vector<std::string> &tokens);

/**
Function to Read def file and get the internal representation in structure.
@param defFile - DEF File name
*/
Symbols DefFile::GetSymbols(const char *defFile)
{
	ReadDefFile(defFile);
	ParseDefFile();

	if(iSymbols.empty())
        ReportError(EMPTYFILEREADING, defFile);
	return iSymbols;
}

/**
Function to read def file line by line to std::vector.
@param defFile - DEF File name
*/
void DefFile::ReadDefFile(const char *aDefFile)
{
    iFileName=aDefFile;

    fstream file(aDefFile, fstream::in);

    if(!file)
        ReportError(FILEOPENERROR, aDefFile);

    string s;
    while(file.good())
    {
        getline(file, s);
        iDefFile.push_back(s);
    }
}

const std::string trim_chars = " \t\n\v\f\r\0";
void LTrim(std::string& str)
{
    str.erase(0, str.find_first_not_of(trim_chars));
}

void RTrim(std::string& str)
{
    str.erase(str.find_last_not_of(trim_chars) + 1);
}

void Trim(std::string& str)
{
    RTrim(str);
    LTrim(str);
}

/**
Function to Parse Def File which has been read in buffer.
@internalComponent
@released
*/
void DefFile::ParseDefFile()
{
	size_t PreviousOrdinal=0;
	size_t LineNum = 0;

	auto t = iDefFile[0];
	if(string::npos != t.find("DSONAME"))
	{
        iDsoNames.push_back(t.erase(0, 1)); // trim comment
        t = iDefFile[1];
	    if(string::npos == t.find("LINKAS"))
            ReportError(ErrorCodes::BADFILE, iFileName,
                        "has DSONAME and missed LINKAS definition");
        iDsoNames.push_back(t.erase(0, 1)); // trim comment
	}

	for(auto str: iDefFile)
    {
        if(str.find("NONAME") != string::npos)
        {
            Trim(str);
            Tokenizer(str, LineNum);
            size_t ordinalNo = iSymbol->Ordinal();
            if (ordinalNo != PreviousOrdinal+1)
            {
                ReportError(ORDINALSEQUENCE, iFileName,
                                   iSymbol->AliasName(), LineNum);
            }

            PreviousOrdinal = ordinalNo;
        }
        LineNum++;
    }
}

void TokensChecker(const std::vector<std::string> &tokens)
{
    if(tokens[1] != "@")
        ReportError(ARGUMENTNAME, tokens[1]);

    for(auto x: tokens)
    {
        if(x.find_first_of(trim_chars) != string::npos)
            ReportError(ARGUMENTNAME, x);
    }
}

/** @brief Analyze line from .def file
  *
  * It split every line to tokens and initialize Symbol class with them
  * Example string for tokenize:
  *  "BIGNUM_it @ 2717 NONAME R3UNUSED ABSENT; some comment"
  *  "BIGNUM_it @ 2717 NONAME DATA 28; some comment"
  */
void DefFile::Tokenizer(std::string aLine, size_t aIndex)
{
    iSymbol = new Symbol(SymbolTypeCode);
    iSymbol->SetSymbolStatus(SymbolStatus::Matching);

//    take comments
    std::size_t pos = aLine.find_first_of(';');
    if(pos < string::npos)
    {
        std::string comment = aLine.substr(pos);
        iSymbol->SetDefFileComment(comment);
        aLine.erase(pos);
    }

//    check optional arguments
    if(aLine.find(" DATA ") < string::npos)
        iSymbol->SetCodeDataType(SymbolTypeData);
    if(aLine.find(" R3UNUSED") < string::npos)
        iSymbol->SetR3Unused(true);
    if(aLine.find(" ABSENT") < string::npos)
        iSymbol->SetAbsent(true);
    if(aLine.find(" MISSING") < string::npos)
        iSymbol->SetSymbolStatus(Missing);

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(aLine);

    while (std::getline(tokenStream, token, ' '))
    {
        tokens.push_back(token);
    }

    if(tokens.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Bad input deffile found!\n");

    if((tokens.size() > 4) && (iSymbol->CodeDataType() == SymbolTypeData))
    {
        for(auto z: tokens[5]) // size of variable in elf
        {
            if(!isdigit(z))
                ReportError(UNRECOGNIZEDTOKEN, iFileName, tokens[5], aIndex);
        }
        uint32_t lenth = atol( tokens[5].c_str() );
        iSymbol->SetSymbolSize(lenth);
    }

    /**< Take SymbolName and maybe AliasName  */
    if(tokens[0].find('=') == string::npos)
        iSymbol->SetName(tokens[0]);
    else
    {
        /**< Symbol name may have alias like SymbolName=AliasName */
        if(tokens[0].find('=') != tokens[0].rfind('='))
            ReportError(UNRECOGNIZEDTOKEN, iFileName, tokens[0],
                    aIndex); /**< Not allowed like SomeName=OtherName=AnotherName */

        std::string sName = tokens[0].substr(0, tokens[0].find('='));
        std::string eName = tokens[0].substr(tokens[0].find('='));
        iSymbol->SetName(sName);
        iSymbol->SetAliasName(eName);
    }

    iSymbol->SetOrdinal( atol( tokens[2].c_str() ) );

    iSymbols.push_back(iSymbol);
}

void DefFile::WriteDefFile(const char *fileName, const Symbols& s)
{
    bool isNewSymFound = true;
    if(s.empty())
        ReportError(EMPTYDATAWRITING, fileName);

    std::fstream fs;
    fs.open(fileName, std::fstream::out | std::fstream::trunc);
    if(!fs.is_open())
        ReportError(FILEOPENERROR,fileName);

    for(auto x: iDsoNames) {
        fs << x;
    }

    fs << "EXPORTS\n";
    for(auto x: s)
    {
        if((x->GetSymbolStatus()==New) && isNewSymFound)
        {
            isNewSymFound = false;
            fs << "; NEW:\n";
        }

        if(x->GetSymbolStatus()==Missing)
            fs << "; MISSING:";
        WriteDefString(x, fs);
    }

    fs << "\n";
    fs.close();
}

void DefFile::SetDsoImpLibName(std::vector<std::string> names) {
    iDsoNames = names;
}

std::vector<std::string> DefFile::GetDsoImpLibName() {
    return iDsoNames;
}

void WriteDefString(Symbol *sym, std::fstream &fstr)
{
    fstr << "\t";
    fstr << sym->Name();
    if(!sym->AliasName().empty() && (sym->AliasName() != sym->Name()) )
        fstr << "=" << sym->AliasName();

    fstr << " @ ";
    fstr << sym->Ordinal();
    fstr << " NONAME";

    if(sym->CodeDataType()==SymbolTypeData)
    {
        fstr << " DATA ";
        fstr << sym->SymbolSize();
    }

    if(sym->R3unused())
        fstr << " R3UNUSED";
    if(sym->Absent())
        fstr << " ABSENT";

    if(!sym->DefFileComment().empty())
    {
        fstr << " ; ";
        fstr << sym->DefFileComment();
    }

    fstr << "\n";
}

Symbols SymbolsFromDef(const char *defFile)
{
	DefFile def;
	return def.GetSymbols(defFile);
}
