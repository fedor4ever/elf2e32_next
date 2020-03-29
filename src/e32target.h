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

#ifndef E32TARGET_H
#define E32TARGET_H

#include <list>
#include <vector>
#include <string>

#include "task.hpp"

struct Args;
struct E32ImageHeader;
typedef std::vector<char> E32SectionUnit;

/// Sections for E32Image chunks in sorted order
enum class E32Sections
{
    HEADER,
    CODE,
    EXPORTS,
    SYMLOOK,
    DATA,
    IMPORTS,
    CODERELOCKS,
    DATARELOCKS
};

struct E32Section
{
    E32Sections type;
    E32SectionUnit section;
    std::string description;
};

typedef std::list<E32Section> E32img;

class E32Target: public Task
{
    public:
        E32Target(Args* param);
        virtual void Run() final;
        virtual ~E32Target();
        void Add(const E32Section& s);
    private:
        void Sort();
    private:
        Args* iBuildOptions = nullptr;
        E32img iE32Image;
};

#endif // E32TARGET_H
