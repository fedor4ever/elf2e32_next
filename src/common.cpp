// Copyright (c) 2019-2024 Strizhniou Fiodar
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

#include <string>
#include <fstream>
#include <unistd.h>
#include <algorithm>

using std::string;

#include "logger.h"
#include "common.hpp"
#include "e32common.h"

const char e32Error[] = "elf2e32: Error: ";
const char e32Warning[] = "elf2e32: Warning: ";
void ReportError(const ErrorCodes err, const std::string& str,
                 void (*f)())
{
    ReportLog(e32Error);
    Logger::Instance()->Log(err, str);
    (*f)();
    throw err;
}

void ReportError(const ErrorCodes err, int x, int y)
{
    ReportLog(e32Error);
    if(y)
        Logger::Instance()->Log(err, x, y);
    else
        Logger::Instance()->Log(err, x);
    throw err;
}

void ReportError(const ErrorCodes err, const std::list<string>& ls,
                const std::string& str, int i)
{
    std::string missedSymbols;
    for(auto x: ls)
    {
        missedSymbols+='\t';
        missedSymbols+=x;
        missedSymbols+='\n';
    }
    ReportLog(e32Error);
    if(!str.empty())
        Logger::Instance()->Log(err, str, i, missedSymbols);
    else
        Logger::Instance()->Log(err, missedSymbols);
    throw err;
}

void ReportError(const ErrorCodes err, const std::string& str,
                 const std::string& s, int x)
{
    ReportLog(e32Error);
    if(s.empty())
        Logger::Instance()->Log(err, str);
    else if(x == 0)
        Logger::Instance()->Log(err, str, s);
    else
        Logger::Instance()->Log(err, str, x, s);
    throw err;
}

void ReportWarning(const ErrorCodes err, int x)
{
    ReportLog(e32Warning);
    Logger::Instance()->Log(err, x);
}

void ReportWarning(const ErrorCodes err, const std::string& s, int x)
{
    ReportLog(e32Warning);
    if(s.empty())
        Logger::Instance()->Log(err);
    else if(!x)
        Logger::Instance()->Log(err, s);
    else
        Logger::Instance()->Log(err, s, x);
}

void ReportWarning(const ErrorCodes err, const std::string& s1, const std::string& s2)
{
    ReportLog(e32Warning);
    Logger::Instance()->Log(err, s1, s2);
}

void ReportLog(const std::string& str, int x, int y, int z)
{
    Logger::Instance()->Log(str, x, y, z);
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

void SaveFile(const string& filename, const string& filebuf)
{
    SaveFile(filename.c_str(), filebuf.c_str(), filebuf.size());
}

void SaveFile(const char* filename, const char* filebuf, int fsize)
{
    std::fstream fs(filename, std::fstream::binary | std::fstream::out | std::fstream::trunc);
    if(!fs)
        ReportError(FILEOPENERROR, filename);
    fs.write(filebuf, fsize);

    if(!fs)
        ReportError(FILESTORERROR, filename);
    fs.close();
}

bool IsFileExist(const std::string& s)
{
    return access(s.c_str(), 0) == 0;
}

string FileNameFromPath(const string& s)
{
    std::size_t found = s.find_last_of("/\\");
    return s.substr(found+1);
}

string ToLower(const string& s)
{
    string data = s;
    std::transform(data.begin(), data.end(), data.begin(),
    [](unsigned char c){ return std::tolower(c); });
    return data;
}
