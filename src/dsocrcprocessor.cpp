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

#include "DSOCRCProcessor.h"
#include "common.hpp"
#include "elfparser.h"
#include "elf2e32_opt.hpp"

#include <string>
#include <fstream>
#include <sstream>

using std::string;
using std::fstream;
using std::stringstream;

void CheckDSOCrc(const Args* args)
{
    if(args == nullptr)
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error in CheckDSOCrc(). Got uninitialized Args object!");

    if(args->iFileCrc.empty()) // option --filecrc not used
        return;

    if(!IsFileExist(args->iDso))
        return;

    if(args->iFileCrc.find(".crc") != std::string::npos)
        return;

    DSOCRCProcessor crc(args);
    crc.Run();
}

void DSOCRCProcessor::SetCRCFiles()
{
    if(iArgs->iDso.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error in DSOCRCProcessor::SetCRCFiles(). Got uninitialized DSO output!");
    ReadOrCreateCRCFile(iArgs->iDso);
}

DSOCRCProcessor::DSOCRCProcessor(const Args* args): CRCProcessor(args) {}

DSOCRCProcessor::~DSOCRCProcessor() {}

std::string DSOCRCProcessor::CRCAsStr()
{
    stringstream buf;
    buf << std::hex;
    buf << "DSO_checksum" << " = 0x" << iCRCOut.iDSOCrc;
    return buf.str();
}

void DSOCRCProcessor::CRCFile(string& s)
{
    s.erase(s.find_last_of("."));
    s += ".dcrc";
}

void DSOCRCProcessor::ProcessTokens(const std::string& type, uint32_t crc)
{
    iCRCIn.iDSOCrc = crc;
}

void DSOCRCProcessor::CRCFromFile()
{
    ElfParser* p = new ElfParser(iArgs->iDso);
    p->GetElfFileLayout();
    iCRCOut.iDSOCrc = Crc32(p->ElfWithFixedHashTable(), p->FileSize());
    delete p;
}

bool DSOCRCProcessor::PrintInvalidTargetCRC()
{
    ReportLog("DSOCRCProcessor: ");
    if(iCRCIn.iDSOCrc == iCRCOut.iDSOCrc)
    {
        ReportLog("All DSO CRC matches!\n");
        return false;
    }
    ReportWarning(ErrorCodes::ZEROBUFFER, "Found CRC32 mismatch(es) between in - out:\n");
    PrintIfNEQ(iCRCIn.iDSOCrc, iCRCOut.iDSOCrc, "DSO Crc");
    return true;
}
