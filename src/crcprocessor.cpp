// Copyright (c) 2024 Strizhniou Fiodar
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
//  It creates CRC32 checksums for DSO files and store them in file with ".dcrc" extension.
//  Also verify generated DSO between different versions if precalculated file used.
//
//

#include <string>
#include <fstream>
#include <sstream>

#include "common.hpp"
#include "crcprocessor.h"
#include "elf2e32_opt.hpp"

using std::string;
using std::fstream;
using std::stringstream;


CRCProcessor::CRCProcessor(const Args* arg): iArgs(arg) {}

CRCProcessor::~CRCProcessor() {}

void CRCProcessor::Run()
{
    DeduceCRCFiles();
    ParseFile();
    CRCFromFile();
    CRCToFile();
    PrintInvalidCRCs();
}

void CRCProcessor::DeduceCRCFiles()
{
    iFileIn = iArgs->iFileCrc;
    if(iFileIn != DefaultOptionalArg)
        return;
    SetCRCFiles();
#ifdef SET_COMPILETIME_LOAD_EXISTED_FILECRC
    if(IsFileExist(iFileOut))
    {
        std::swap(iFileIn, iFileOut);
        iFileOut.clear();
    }
#endif // SET_COMPILETIME_LOAD_EXISTED_FILECRC
}

void CRCProcessor::ParseFile()
{
    if(!IsFileExist(iFileIn))
        return;

    ReportLog("Reading checksums from file: " + iFileIn + "\n\n");
    fstream file(iFileIn, fstream::in);

    string s;
    while(file.good())
    {
        getline(file, s);
        Tokenize(s);
    }

    if(!file)
        ReportError(FILEREADERROR, iFileIn);
    file.close();
}

void CRCProcessor::CRCToFile()
{
    if(iFileOut.empty())
        return;
    ReportLog("Saving checksums to file: " + iFileOut + "\n\n");
    string tmp = CRCAsStr();
    SaveFile(iFileOut, tmp);
}

/** @brief Print difference for CRCs if any.
  *
  * Print difference between predefined CRCs from crc file and generated from input file if any.
  */
void CRCProcessor::PrintInvalidCRCs()
{
    if(!IsFileExist(iFileIn))
        return;

    if(!PrintInvalidTargetCRC())
        return;
    if(iArgs->iForceE32Build)
        ReportLog("\n");
    else
        ReportError(ErrorCodes::ZEROBUFFER, "CRC32 validation failed!\n");
}

void CRCProcessor::Tokenize(const string& line)
{
    if(line.empty())
        return;

    stringstream stream(line);
    string type;
    char delim;
    uint32_t crc;
    stream >> std::hex >> type >> delim >> crc;

    if(delim != '=')
        ReportError(ErrorCodes::ZEROBUFFER, "Error while parsing crc file. Delimeter '=' not found!");

    ProcessTokens(type, crc);
}

void CRCProcessor::ReadOrCreateCRCFile(const std::string& file)
{
    iFileIn = file;
    CRCFile(iFileIn);
    if(IsFileExist(iFileIn))
        return;
    iFileOut = iFileIn;
    iFileIn.clear();
}

void PrintIfNEQ(uint32_t in, uint32_t out, const string& msg)
{
    if(in == out)
        return;
    std::stringstream s;
    s << std::hex << msg + ": 0x" << in << " - 0x" << out << "\n";
    ReportLog(s.str());
}
