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

#include <fstream>

#include "logger.h"
#include "common.hpp"
#define INCLUDE_CAPABILITY_NAMES
#include "e32capability.h"

void ReportError(const ErrorCodes err, const std::string& str,
                 void (*f)())
{
    ReportLog("elf2e32: Error: ");
    Logger::Instance()->Log(err, str);
    (*f)();
    throw err;
}

void ReportError(const ErrorCodes err, const int x, const int y)
{
    ReportLog("elf2e32: Error: ");
    if(y)
        Logger::Instance()->Log(err, x, y);
    else
        Logger::Instance()->Log(err, x);
    throw err;
}

void ReportError(const ErrorCodes err, const std::string& str,
                 const std::string& s, const int x)
{
    ReportLog("elf2e32: Error: ");
    if(s.empty())
        Logger::Instance()->Log(err, str);
    else
        Logger::Instance()->Log(err, str, s);
    throw err;
}

void ReportWarning(const ErrorCodes err, const std::string& s, const int x)
{
    ReportLog("elf2e32: Warning: ");
    Logger::Instance()->Log(err, s);
}

void ReportLog(const std::string& str, int x, int y, int z)
{
    if(z > -1)
        Logger::Instance()->Log(str, x, y, z);
    else if(y > -1)
        Logger::Instance()->Log(str, x, y);
    else if(x > -1)
        Logger::Instance()->Log(str, x);
    else
        Logger::Instance()->Log(str);
}

const char* ReadFile(const char* filename, std::streamsize& fsize)
{
    std::fstream fs(filename, std::fstream::binary | std::fstream::in);
    if(!fs)
        ReportError(FILEOPENERROR, filename);

    fs.seekg(0, fs.end);
    fsize = fs.tellg();
    fs.seekg(0, fs.beg);

    char* bufferedFile = new char[fsize]();
    fs.read(bufferedFile, fsize);
    if(!fs)
    {
    	delete[] bufferedFile;
    	ReportError(FILEREADERROR, filename);
    }
    fs.close();
    return bufferedFile;
}
