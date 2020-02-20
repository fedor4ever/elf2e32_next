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
#include "elf2e32_opt.hpp"
#include "artifactbuilder.h"

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
    iSymbols = SymbolsFromDef(iOpts->iDefinput.c_str());
    //TODO: process symbols from elf and DSO too
}

void ArtifactBuilder::MakeDSO()
{
    if(iOpts->iDso.empty())
        return;
    DSOFile* dso = new DSOFile();
    std::size_t found = iOpts->iDso.find_last_of("/\\");
    dso->WriteDSOFile(iOpts->iDso.c_str(), iOpts->iDso.substr(found+1).c_str(),
              iOpts->iLinkas.c_str(), iSymbols);
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
