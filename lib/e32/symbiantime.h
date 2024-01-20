// Copyright (c) 2020-2024 Strizhniou Fiodar
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
// Convert Unix time to Symbian time
//
//

#ifndef SYMBIANTIME_H
#define SYMBIANTIME_H

#include <time.h>
#include <stdint.h>

class SymbianTime
{
    public:
        SymbianTime(time_t t = time(nullptr));
        uint32_t TimeLo() const;
        uint32_t TimeHi() const;
    private:
        uint32_t iTimeLo = 0;
        uint32_t iTimeHi = 0;
};

#endif // SYMBIANTIME_H
