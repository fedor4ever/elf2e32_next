// Copyright (c) 2019-2020 Strizhniou Fiodar
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
// Print various info to console screen and
// file(optional) for the elf2e32 tool
//
//

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <stdio.h>
#include "common.hpp"

class Logger
{
    public:
        static Logger* Instance(const std::string& s = empty);
        ~Logger();
        void Log(const std::string& s);
        void Log(const std::string& s, int x);
        void Log(const std::string& s, int x, int y);
        void Log(const std::string& s, int x, int y, int z);

        void Log(ErrorCodes errcode);
        void Log(ErrorCodes errcode, const std::string& s);
        void Log(ErrorCodes errcode, const std::string& s1, const std::string& s2);

        void Log(ErrorCodes errcode, const std::string& s1, int x, const std::string& s2 = empty);

        void Log(ErrorCodes errcode, int x);
        void Log(ErrorCodes errcode, int x, int y);
        void Log(ErrorCodes errcode, int x, int y, int z);
    private:
        Logger(const std::string& s);
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
    private:
        FILE* iFile = nullptr;
};

#endif // LOGGER_H
