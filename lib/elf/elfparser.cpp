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
// Parse ELF Images
//
//

#include "elfparser.h"
#include "common.hpp"

using std::string;

ElfParser::ElfParser(string elf): iFile(elf) {}

ElfParser::~ElfParser()
{
    delete[] iFileBuf;
}

void ElfParser::GetElfFileLayout()
{
    iFileBuf = ReadFile(iFile.c_str(), iFileBufSize);
}
