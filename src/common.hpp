// Copyright (c) 2019 Strizhniou Fiodar
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
// Hold together small helper functions for the elf2e32 tool
//
//

#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <list>
#include <string>

class Symbol;
typedef std::list <Symbol*>	Symbols;

enum ErrorCodes
{
    UNKNOWNERROR,
    MISSEDARGUMENT,
    UNKNOWNOPTION,
    FILEOPENERROR,
    NOTELFFILE,
    UNKNOWNCOMPRESSION,
    BYTEPAIRINCONSISTENTSIZE,
    HUFFMANINCONSISTENTSIZE,
    ELFFILEEXPECTEDE32,
    WRONGFILESIZEFORDECOMPRESSION,
    ZEROBUFFER,
    BADEXPORTS,
    NOREQUIREDOPTION,
    INVALIDARGUMENT,
    EMPTYFILEREADING,
    ORDINALSEQUENCE,
    ARGUMENTNAME,
    UNRECOGNIZEDTOKEN,
    EMPTYDATAWRITING,
	E32IMAGEVALIDATIOFAILURE
};

// handy macro for tracing
// ex: E32IMAGEHEADER_TRACE(("E32RelocSection block offset: 0x%x\t size: 0x%x\n",pageOffset,blockSize));
#if USE_E32IMAGE_TRACE
	#define E32IMAGEHEADER_TRACE(_t) printf _t
#else
	#define E32IMAGEHEADER_TRACE(_t)
#endif

const std::string empty = std::string();

void ReportError(const ErrorCodes err, const std::string& str,
                 void (*f)());
void ReportError(const ErrorCodes err, const int x, const int = 0);
void ReportError(const ErrorCodes err, const std::string& str = empty,
                 const std::string& s = empty, const int = 0);

void ReportWarning(const ErrorCodes err, const std::string& str = empty, const int = 0);
void ReportLog(const std::string& str, int x = -1, int y = -1, int z = -1);

const char* ReadFile(const char* filename, int& fsize);

Symbols SymbolsFromDef(const char *defFile);

#endif // COMMON_HPP_INCLUDED
