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

#ifndef ELFPARSER_H
#define ELFPARSER_H

#include <string>

class ElfParser
{
    public:
        ElfParser(std::string elf);
        void GetElfFileLayout();
        ~ElfParser();
    private:
        std::string iFile;
        const char* iFileBuf = nullptr;
        std::streamsize iFileBufSize = 0;
};

#endif // ELFPARSER_H
