// Copyright (c) 2019-2023 Strizhniou Fiodar
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
#include <memory>
#include <string>

class Args;
class Symbol;
class E32Parser;
class ElfParser;

typedef std::list <Symbol*>	Symbols;
/// TODO (Administrator#9#05/08/20): Use smartpointers to prevent memory leaks
//typedef std::list <std::shared_ptr<Symbol*>> Symbols;


enum ErrorCodes
{
    UNKNOWNERROR,
    MISSEDARGUMENT,
    UNKNOWNOPTION,
    FILEOPENERROR,
	FILEREADERROR,
	FILESTORERROR,
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
	E32IMAGEVALIDATIOFAILURE,
	HUFFMANINVALIDCODINGERROR,
	HUFFMANTOOMANYCODESERROR,
	HUFFMANBUFFEROVERFLOWERROR,
	HUFFMANBUFFERUNDERFLOWERROR,
	EMPTYARGUMENT,
	VALUEOVERFLOW,
    ELFMAGICERROR,
    ELFCLASSERROR,
    ELFABIVERSIONERROR,
    ELFLEERROR,
    ELFARMERROR,
    ELFEXECUTABLEERROR,
    ELFSHSTRINDEXERROR,
    SYMBOLCOUNTMISMATCHERROR,
    ABSENTSYMBOL,
    ABSENTSYMBOLINELF,
    MISSEDFROZENSYMBOLS,
    MISSEDFROZENSYMBOLSERROR,
    UNKNOWNHEADERNAME,
    ELFPIEERROR,
    DEPRECATEDTARGET,
    UNDEFINEDENTRYPOINT,
    ENTRYPOINTNOTSET,
    MISSEDEXCEPTIONDESCRIPTOR,
    OUTOFBOUNDSEXCEPTIONDESCRIPTOR,
    NOSTATICSYMBOLS,
    UNKNOWNSECTION,
    UNFROZENSYMBOLADDED,
    ELF_ST_VALUE,
    ILLEGALEXPORTFROMDATASEGMENT,
    BADFILE
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

void ReportError(const ErrorCodes err, std::list<std::string> ls,
            const std::string& str = empty, const int = 0);
void ReportError(const ErrorCodes err, const std::string& str = empty,
            const std::string& s = empty, const int = 0);

void ReportWarning(const ErrorCodes err, const int x);
void ReportWarning(const ErrorCodes err, const std::string& str = empty, const int = 0);
void ReportWarning(const ErrorCodes err, const std::string& s1, const std::string& s2);

void ReportLog(const std::string& str, int x = -1, int y = -1, int z = -1);

const char* ReadFile(const char* filename, std::streamsize& fsize);
void SaveFile(const char* filename, const char* filebuf, int fsize);
void SaveFile(const std::string& filename, const std::string& filebuf);

Symbols SymbolsFromDef(const char *defFile);

void BuildE32Image(const Args* args, const ElfParser* elfParser, const Symbols& s);

bool VerboseOut();
bool DisableLongVerbosePrint();

std::string ToLower(const std::string& s);

void CheckE32CRC(const E32Parser* parser, const Args* args);

#endif // COMMON_HPP_INCLUDED
