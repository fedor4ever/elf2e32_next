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
// Hold together small helper functions for the elf2e32 tool
//
//

#include <fstream>
#include <string>

using  std::string;

#include "logger.h"
#include "common.hpp"
#include "e32common.h"

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

void ReportError(const ErrorCodes err, std::list<string> ls,
                const std::string& str, const int x)
{
    std::string missedSymbols;
    for(auto x: ls)
    {
        missedSymbols+='\t';
        missedSymbols+=x;
        missedSymbols+='\n';
    }
    ReportLog("elf2e32: Error: ");
    Logger::Instance()->Log(err, str, x, missedSymbols);
}

void ReportError(const ErrorCodes err, const std::string& str,
                 const std::string& s, const int x)
{
    ReportLog("elf2e32: Error: ");
    if(s.empty())
        Logger::Instance()->Log(err, str);
    else
        Logger::Instance()->Log(err, str, x, s);
    throw err;
}

void ReportWarning(const ErrorCodes err, const int x)
{
    ReportLog("elf2e32: Warning: ");
    Logger::Instance()->Log(err, x);
}

void ReportWarning(const ErrorCodes err, const std::string& s, const int x)
{
    ReportLog("elf2e32: Warning: ");
    Logger::Instance()->Log(err, s);
}

void ReportWarning(const ErrorCodes err, const std::string& s1, const std::string& s2)
{
    ReportLog("elf2e32: Warning: ");
    Logger::Instance()->Log(err, s1, s2);
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

uint16_t ProcessPriority(const std::string& str)
{
	if(str == "low")
		return (uint16_t)TProcessPriority::EPriorityLow;
	if(str == "background")
		return (uint16_t)TProcessPriority::EPriorityBackground;
	if(str == "foreground")
		return (uint16_t)TProcessPriority::EPriorityForeground;
	if(str == "high")
		return (uint16_t)TProcessPriority::EPriorityHigh;
	if(str == "windowserver")
		return (uint16_t)TProcessPriority::EPriorityWindowServer;
	if(str == "fileserver")
		return (uint16_t)TProcessPriority::EPriorityFileServer;
	if(str == "realtimeserver")
		return (uint16_t)TProcessPriority::EPriorityRealTimeServer;
	if(str == "supervisor")
		return (uint16_t)TProcessPriority::EPrioritySupervisor;
	return 0;
}
