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
// Provide common interface for major task in elf2e32 tool
//
//

#ifndef TASK_HPP_INCLUDED
#define TASK_HPP_INCLUDED

class Task
{
public:
    virtual ~Task(){}
    virtual void Run() = 0;
};

#endif // TASK_HPP_INCLUDED
