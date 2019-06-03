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
    {ErrorCodes::NOTELFFILE,    "This file not ELF: %s!\n"},
    {ErrorCodes::UNKNOWNCOMPRESSION, "Unknown compression algorythm.\n"},
    {ErrorCodes::BYTEPAIRINCONSISTENTSIZE,
            "Inconsistent sizes discovered during Byte pair uncompression.\n"},
    {ErrorCodes::HUFFMANINCONSISTENTSIZE,
            "Inconsistent sizes discovered during Huffman uncompression.\n"},
    {ErrorCodes::ELFFILEEXPECTEDE32, "Expected E32 image but got ELF file.\n"},
    {ErrorCodes::WRONGFILESIZEFORDECOMPRESSION, "Set wrong size for "
            "decompression. Expected: %d but got: %d\n"},
    {ErrorCodes::ZEROBUFFER, "Got zero sized buffer for parse!"
            "May be internal error.\n"},
    {ErrorCodes::BADEXPORTS, "Bad exports.\n"},
    {ErrorCodes::NOREQUIREDOPTION, "Expected option %s not set.\n"},
    {ErrorCodes::INVALIDARGUMENT, "Option :%s has invalid arg: %s.\n"},
    {ErrorCodes::EMPTYFILEREADING, "Def file: %s has no symbols definitions.\n"},
    {ErrorCodes::ORDINALSEQUENCE, "Ordinal number is not in sequence:"
            " %s[Line No=%d][%s].\n"},
    {ErrorCodes::ARGUMENTNAME, "Argument %s is not correct.\n"},
    {ErrorCodes::UNRECOGNIZEDTOKEN, "Unrecognized Token : %s[Line No=%d][%s].\n"},
    {ErrorCodes::EMPTYDATAWRITING, "Got empty data for store in file: %s.\n"}//,
//    {ErrorCodes::, ".\n"}//,
};

Logger::Logger(const std::string& s)
{
    iFile = fopen(s.c_str(), "w");
    if(!iFile && !s.empty())
        printf("Cannot open file %s.\n", s.c_str());
}

Logger::~Logger()
{
    if(iFile)
        fclose(iFile);
    delete _self;
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
        fprintf(iFile, s.c_str());
    printf(s.c_str());
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

void Logger::Log(ErrorCodes errcode)
{
    if(iFile)
        fprintf(iFile, Messages[errcode].str);
    printf(Messages[errcode].str);
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
