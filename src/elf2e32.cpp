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
// Control workflow for the elf2e32 tool
//
//

#include "logger.h"
#include "elf2e32.h"
#include "e32info.h"
#include "e32common.h"
#include "argparser.h"
#include "elf2e32_opt.hpp"

Elf2E32::Elf2E32(int argc, char** argv)
{
    iHdr = new E32ImageHeader();
    iArgs = new ArgParser(argc, argv);
}

Elf2E32::~Elf2E32()
{
    delete iHdr;
    delete iArgs;
    delete iArg;
}

void Elf2E32::Run()
{
    iArg = iArgs->Parse();
    Logger::Instance(iArg->iLog);
    if(!iArg->iE32input.empty() && iArg->iOutput.empty())
        iTask = new E32Info(iArg);

    if(iTask)
        iTask->Run();
}
