// Copyright (c) 2018-2019 Strizhniou Fiodar
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
//  for the elf2e32 tool
//
//

#include "e32target.h"

struct TargetDesc
{
    E32Sections index;
    const char* description = nullptr;
};

struct TargetDesc TargetInfo[8]=
{
    {E32Sections::HEADER,      "Image Header"},
    {E32Sections::CODE,        "Code Section"},
    {E32Sections::EXPORTS,     "Export Table"},
    {E32Sections::SYMLOOK,     "Symbol Info"},
    {E32Sections::DATA,        "Data Section"},
    {E32Sections::IMPORTS,     "Import Section"},
    {E32Sections::CODERELOCKS, "Code Relocs"},
    {E32Sections::DATARELOCKS, "Data Relocs"}
};


bool CmpSections(E32Section first, E32Section second)
{
    return first.type < second.type;
}

E32Target::~E32Target()
{
    //dtor
}

E32Target::E32Target(Args* param): iBuildOptions(param){}

void E32Target::Run()
{
	;
}
void E32Target::Add(const E32Section& s)
{
    iE32Image.push_back(s);
}

void E32Target::Sort()
{
    iE32Image.sort(CmpSections);
}
