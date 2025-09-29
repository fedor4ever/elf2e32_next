// Copyright (c) 2019-2024 Strizhniou Fiodar
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

#include <vector>
#include <string>

struct Args;
struct Opts;

class ArgParser
{
    public:
        ArgParser(int argc, char** argv);
        ArgParser(std::vector<std::string> argv, const struct Opts* opt = nullptr);
        ~ArgParser();
        bool Parse(Args* args) const;
    private:
        void ArgInfo(const Opts& opt) const;
    private:
        int iArgc;
        std::vector<std::string> iArgv;
        const struct Opts* iTests = nullptr;
};

#endif // ARGPARSER_H
