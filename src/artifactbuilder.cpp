// Copyright (c) 2020 - 2023 Strizhniou Fiodar
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

#include <vector>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "task.hpp"
#include "symbol.h"
#include "deffile.h"
#include "dsofile.h"
#include "common.hpp"
#include "e32common.h"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "artifactbuilder.h"
#include "symbolprocessor.h"
#include "elf2e32_version.hpp"

void MakeImportHeader(Symbols symbols, std::string dllName);
void ValidateOptions(Args* arg);

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
    MakeImportHeader(iSymbols, iOpts->iHeader);
}

void ArtifactBuilder::PrepareBuild()
{
    ValidateOptions(iOpts);
    if(!iOpts->iElfinput.empty())
    {
        iElfParser = new ElfParser(iOpts->iElfinput);
        iElfParser->GetElfFileLayout();
    }
    SymbolProcessor processor(iOpts, iElfParser);
    iSymbols = processor.GetExports();
    DsoImpLibName();
}

void ArtifactBuilder::DsoImpLibName()
{
    if(iOpts->iDefinput.empty())
        return;
    DefFile d;
    d.GetSymbols(iOpts->iDefinput.c_str());
    iDsoImpLibName = d.GetDsoImpLibName();
    if(iOpts->iDSODump)
        iDsoImpLibName = iElfParser->DsoImpLibName();
}

void ArtifactBuilder::MakeDSO()
{
    if(iOpts->iDso.empty())
        return;
    DSOFile* dso = new DSOFile();
    dso->WriteDSOFile(iOpts, iSymbols, iDsoImpLibName);
    delete dso;
}

void ArtifactBuilder::MakeDef()
{
    if(iOpts->iDefoutput.empty())
        return;
    DefFile deffile;

    std::vector<std::string> tmp;
    if(iOpts->iDSODump)
        deffile.SetDsoImpLibName(iDsoImpLibName);
    deffile.WriteDefFile(iOpts->iDefoutput.c_str(), iSymbols);
}

void ArtifactBuilder::MakeE32()
{
    if(iOpts->iOutput.empty())
        return;
    BuildE32Image(iOpts, iElfParser, iSymbols);
}

void MakeImportHeader(Symbols symbols, std::string dllName)
{
    if(dllName.empty())
        return;

    std::fstream fstr(dllName, std::fstream::out | std::fstream::trunc);
    ToolVersion ver;
    auto it = symbols.begin();

    fstr << "//This file generated by elf2e32 ";
    fstr << (uint32_t)ver.iMajor << "." << (uint32_t)ver.iMinor << "." << ver.iBuild;
    fstr << " to easy dynamic linking.\n";
    fstr << "//Usage example:\n// #include <e32std.h>\n// RLibrary library;\n";
    fstr << "// User::LeaveIfError(library.Load(" << dllName << "));\n";
    fstr << "// TLibraryFunction entry=library.Lookup(" << (*it)->AliasName() << ");\n";
    fstr << "// Call the function to create new CMessenger\n"
        "// CMessenger* messenger=(CMessenger*) entry();\n";
    fstr << "// library.Close();\n";

    fstr << "#ifndef ELF2E32_" << dllName << "\n";
    fstr << "#define ELF2E32_" << dllName << "\n";
    for(auto x: symbols)
    {
        fstr << "#define " << x->AliasName() << "\t" << x->Ordinal() << "\n";
    }

    fstr << "#endif // ELF2E32_" << dllName << "\n";
    fstr.close();
}
