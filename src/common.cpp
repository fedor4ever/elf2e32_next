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

#include <iostream>
#include "logger.h"
#include "common.hpp"

using std::cout;

void ReportError(const ErrorCodes err, const std::string& s, const int)
{
    ReportLog("elf2e32: Error: ");
    Logger::Instance()->Log(err, s);
}

void ReportWarning(const ErrorCodes err, const std::string& s, const int)
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
