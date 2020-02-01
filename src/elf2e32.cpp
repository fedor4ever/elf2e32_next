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
#include <iostream>
#include "logger.h"
#include "elf2e32.h"
#include "e32info.h"
#include "e32common.h"
#include "argparser.h"
#include "e32rebuilder.h"
#include "elf2e32_opt.hpp"

Elf2E32::Elf2E32(int argc, char** argv)
{
    iHdr = new E32ImageHeader();
    iArgParser = new ArgParser(argc, argv);
    iCmdParam = new Args();
}

Elf2E32::~Elf2E32()
{
    delete iHdr;
    delete iArgParser;
    delete iCmdParam;
    delete iTask;
}

void Elf2E32::Run()
{
    if(!iArgParser->Parse(iCmdParam))
    	return;

    Logger::Instance(iCmdParam->iLog);
    if(!iCmdParam->iE32input.empty() && iCmdParam->iOutput.empty())
        iTask = new E32Info(iCmdParam);

    else if(!iCmdParam->iE32input.empty() && !iCmdParam->iOutput.empty())
        iTask = new E32Rebuilder(iCmdParam);

    else if(!iCmdParam->iElfinput.empty())
        iTask = new ArtifactBuilder(iCmdParam);

    if(iTask)
        iTask->Run();
}
