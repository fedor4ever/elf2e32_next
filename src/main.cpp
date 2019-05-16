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
// entry point
//
//

#include <iostream>
#include "elf2e32.h"

using namespace std;

#if __cplusplus < 201304
#error "compiler with C++14 required!"
#endif // __cplusplus

int main(int argc, char** argv)
{
    Elf2E32 task(argc, argv);
    task.Run();
    return 0;
}
