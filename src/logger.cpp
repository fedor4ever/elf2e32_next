// Copyright (c) 2019-2020 Strizhniou Fiodar
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
// Print various info to console screen and
// file(optional) for the elf2e32 tool
//
//

#include <stdio.h>
#include "logger.h"

static Logger* _self = nullptr;

struct Message
{
    int index = 0;
    const char* str = nullptr;
};

struct Message Messages[]=
{
    {ErrorCodes::UNKNOWNERROR,  "Unknown error happens!\n"},
    {ErrorCodes::MISSEDARGUMENT, "Option %s has missed argument\n"},
    {ErrorCodes::UNKNOWNOPTION, "Option %s unrecognized\n"},
    {ErrorCodes::FILEOPENERROR, "Can't open file: %s!\n"},
	{ErrorCodes::FILEREADERROR, "Can't read file: %s.\n"},
	{ErrorCodes::FILESTORERROR, "Can't save file: %s.\n"},
    {ErrorCodes::NOTELFFILE,    "This file not ELF: %s!\n"},
    {ErrorCodes::UNKNOWNCOMPRESSION, "Unknown compression algorythm.\n"},
    {ErrorCodes::BYTEPAIRINCONSISTENTSIZE,
            "Inconsistent sizes discovered during Byte pair uncompression.\n"},
    {ErrorCodes::HUFFMANINCONSISTENTSIZE,
            "Inconsistent sizes discovered during Huffman uncompression.\n"},
    {ErrorCodes::ELFFILEEXPECTEDE32, "Expected E32 image but got ELF file.\n"},
    {ErrorCodes::WRONGFILESIZEFORDECOMPRESSION, "Set wrong size for "
            "decompression. Expected: %d but got: %d\n"},
    {ErrorCodes::ZEROBUFFER, "%s"},
    {ErrorCodes::BADEXPORTS, "Bad exports.\n"},
    {ErrorCodes::NOREQUIREDOPTION, "Expected option %s not set.\n"},
    {ErrorCodes::INVALIDARGUMENT, "Option :%s has invalid arg: %s.\n"},
    {ErrorCodes::EMPTYFILEREADING, "Def file: %s has no symbols definitions.\n"},
    {ErrorCodes::ORDINALSEQUENCE, "Ordinal number is not in sequence:"
            " %s[Line No=%d][%s].\n"},
    {ErrorCodes::ARGUMENTNAME, "Argument %s is not correct.\n"},
    {ErrorCodes::UNRECOGNIZEDTOKEN, "Unrecognized Token : %s[Line No=%d][%s].\n"},
    {ErrorCodes::EMPTYDATAWRITING, "Got empty data for store in file: %s.\n"},
    {ErrorCodes::E32IMAGEVALIDATIOFAILURE, "Failed to validate E32 image for"
            " valid %s.\n"},
    {ErrorCodes::HUFFMANINVALIDCODINGERROR, "Invalid Huffman coding during E32Image compression.\n"},
    {ErrorCodes::HUFFMANTOOMANYCODESERROR,  "Too many Huffman codes during E32Image compression.\n"},
    {ErrorCodes::HUFFMANBUFFEROVERFLOWERROR, "Huffman buffer overflow during E32Image compression.\n"},
    {ErrorCodes::HUFFMANBUFFERUNDERFLOWERROR, "Huffman buffer underflow on deflate.\n"},
    {ErrorCodes::EMPTYARGUMENT, "Function %s got empty argument for: %s.\n"},
    {ErrorCodes::VALUEOVERFLOW, "Overflow happens for: %s.\n"},
    {ErrorCodes::ELFMAGICERROR, "Invalid ELF magic in file : %s."},
	{ErrorCodes::ELFCLASSERROR, "ELF file %s is not 32 bit."},
	{ErrorCodes::ELFABIVERSIONERROR, "ELF file %s is not BPABI conformant."},
	{ErrorCodes::ELFLEERROR, "ELF file %s is not Little Endian."},
	{ErrorCodes::ELFARMERROR, "ELF file %s does not target ARM."},
	{ErrorCodes::ELFEXECUTABLEERROR, "ELF file %s is neither executable (ET_EXEC) or shared (ET_DYN)."},
	{ErrorCodes::ELFSHSTRINDEXERROR, "Error in ELF Section Header String Index : %s."},
	{ErrorCodes::SYMBOLCOUNTMISMATCHERROR, "Symbol count provided by DT_ARM_SYMTABSZ is not same as "
            "that in the Hash Table in %s"},
    {ErrorCodes::ABSENTSYMBOL, "Attempt to export absent symbol %s found. This can break library ABI.\n"},
    {ErrorCodes::ABSENTSYMBOLINELF, "%s absent in the DEF file and --sysdef, but present in the ELF file.\n"},
    {ErrorCodes::MISSEDFROZENSYMBOLS, "Frozen Export(s): %u missing from the ELF file.\n"},
    {ErrorCodes::MISSEDFROZENSYMBOLSERROR, "ELF File %s with frozen export(s) have %u missed symbol(s):\n%s.\n"},
    {ErrorCodes::SYSDEFMERGE, "Failure when merge symbols from .def file and option --sysdef for symbol(s): %s.\n"},
    {ErrorCodes::UNKNOWNHEADERNAME, "Can't deduce name for header file with exports.\n"},
    {ErrorCodes::ELFPIEERROR, "Found elf file %s builded with -pie option. Symbian has no support for it.\n"},
    {ErrorCodes::DEPRECATEDTARGET, "This target deprecated at v9.0. Application must be converted to EXE or PLUGIN (ECOM).\n"},
    {ErrorCodes::UNDEFINEDENTRYPOINT, "Entry point and Text segment base both 0, can't tell if entry point set for elf.\n"},
    {ErrorCodes::ENTRYPOINTNOTSET, "Entry point is not set for elf\n"},
    {ErrorCodes::MISSEDEXCEPTIONDESCRIPTOR, "Input ELF File has not found(or missed) exception descriptor.\n"},
    {ErrorCodes::OUTOFBOUNDSEXCEPTIONDESCRIPTOR, "Input ELF File has invalid exception descriptor.\n"},
    {ErrorCodes::NOSTATICSYMBOLS, "Input ELF File contains no static symbols.\n"},
    {ErrorCodes::UNKNOWNSECTION, "Detected unknown section while building E32Image.\n"}//,
//    {ErrorCodes::, ".\n"}//,
};

Logger::Logger(const std::string& s)
{
    if(s.empty())
        return;

    iFile = fopen(s.c_str(), "w");
    if(!iFile)
    {
        printf("Cannot open file %s.\n", s.c_str());
        throw ErrorCodes::FILEOPENERROR;
    }

}

Logger::~Logger()
{
    if(iFile)
        fclose(iFile);
}

Logger* Logger::Instance(const std::string& s)
{
    if(!_self)
        _self = new Logger(s);
    return _self;
}

void Logger::Log(const std::string& s)
{
    if(iFile)
        fprintf(iFile, "%s", s.c_str());
    printf("%s", s.c_str());
}

void Logger::Log(const std::string& s, int x)
{
    if(iFile)
        fprintf(iFile, s.c_str(), x);
    printf(s.c_str(), x);
}

void Logger::Log(const std::string& s, int x, int y)
{
    if(iFile)
        fprintf(iFile, s.c_str(), x, y);
    printf(s.c_str(), x, y);
}

void Logger::Log(const std::string& s, int x, int y, int z)
{
    if(iFile)
        fprintf(iFile, s.c_str(), x, y, z);
    printf(s.c_str(), x, y, z);
}

void Logger::Log(ErrorCodes errcode, const std::string& s)
{
    if(iFile)
        fprintf(iFile, Messages[errcode].str, s.c_str());
    printf(Messages[errcode].str, s.c_str());
}


void Logger::Log(ErrorCodes errcode, const std::string& s1, const std::string& s)
{
    if(iFile)
        fprintf(iFile, Messages[errcode].str, s1.c_str(), s.c_str());
    printf(Messages[errcode].str, s1.c_str(), s.c_str());
}

void Logger::Log(ErrorCodes errcode, const std::string& s1, int x, const std::string& s2)
{
    if(iFile)
        fprintf(iFile, Messages[errcode].str, s1.c_str(), x, s2.c_str());
    printf(Messages[errcode].str, s1.c_str(), x, s2.c_str());
}


void Logger::Log(ErrorCodes errcode)
{
    if(iFile)
        fprintf(iFile, "%s", Messages[errcode].str);
    printf("%s", Messages[errcode].str);
}

void Logger::Log(ErrorCodes errcode, const int x, const int y)
{
    if(iFile)
        fprintf(iFile, Messages[errcode].str, x, y);
    printf(Messages[errcode].str, x, y);
}

void Logger::Log(ErrorCodes errcode, const int x)
{
    if(iFile)
        fprintf(iFile, Messages[errcode].str, x);
    printf(Messages[errcode].str, x);
}
