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
// Parse cmdline arguments for the elf2e32 tool
//
//

#ifndef ARGPARSER_H
#define ARGPARSER_H

struct Args;

class ArgParser
{
    public:
        ArgParser(int argc, char** argv);
        ~ArgParser();
        bool Parse(Args* args) const;
    private:
        int iArgc;
        char** iArgv;
};

#endif // ARGPARSER_H
