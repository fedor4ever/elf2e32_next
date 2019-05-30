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

#include <string>

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
    BADEXPORTS
};

const std::string empty = std::string();

void ReportError(  const ErrorCodes err, const int x, const int = 0);
void ReportError(  const ErrorCodes err, const std::string& str = empty, const int = 0);
void ReportWarning(const ErrorCodes err, const std::string& str = empty, const int = 0);
void ReportLog(const std::string& str, int x = -1, int y = -1, int z = -1);

#endif // COMMON_HPP_INCLUDED
