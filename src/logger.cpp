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
    {ErrorCodes::UNKNOWNERROR  , "Unknown error happens!\n"},
    {ErrorCodes::MISSEDARGUMENT, "option %s has missed argument\n"},
    {ErrorCodes::UNKNOWNOPTION , "option %s unrecognized\n"}
//    {, },
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
