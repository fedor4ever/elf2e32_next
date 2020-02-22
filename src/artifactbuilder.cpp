// Copyright (c) 2020 Strizhniou Fiodar
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
// Build E32Image and other companions
//
//

#include "task.hpp"
#include "deffile.h"
#include "dsofile.h"
#include "common.hpp"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "artifactbuilder.h"
#include "symbolprocessor.h"

class Symbol;

ArtifactBuilder::ArtifactBuilder(Args* param): iOpts(param)
{
    //ctor
}

ArtifactBuilder::~ArtifactBuilder()
{
    //dtor
}

void ArtifactBuilder::Run()
{
    PrepareBuild();
    MakeDSO();
    MakeDef();
    MakeE32();
}

void ArtifactBuilder::PrepareBuild()
{
    iElfParser = new ElfParser(iOpts->iElfinput);
    iElfParser->GetElfFileLayout();
    SymbolProcessor processor(iElfParser, iOpts);
    //TODO: process symbols from elf and --sysdef
    iSymbols = processor.Process();
}

void ArtifactBuilder::MakeDSO()
{
    if(iOpts->iDso.empty())
        return;
    DSOFile* dso = new DSOFile();
    dso->WriteDSOFile(iOpts->iDso, iOpts->iLinkas, iSymbols);
    delete dso;
}

void ArtifactBuilder::MakeDef()
{
    if(iOpts->iDefoutput.empty())
        return;
    DefFile deffile;
	deffile.WriteDefFile(iOpts->iDefoutput.c_str(), iSymbols);
}

void ArtifactBuilder::MakeE32()
{
    if(!iOpts->iOutput.empty())
        return;
}
