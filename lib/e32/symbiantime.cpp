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
// Edit E32Image header fields and compression type
//
//

#include <time.h>
#include "symbiantime.h"

// Symbian time in seconds starts since midnight Jan 1st, 2000
constexpr uint64_t KSymbianTimeUnixOffsetInSeconds = (30*365*24*60*60) + (7*24*60*60);
constexpr uint64_t KBritishSummerTime = 60 * 60;
constexpr uint64_t KSecondsTo2000AD=730497 * 24 * 3600 + KBritishSummerTime;

SymbianTime::SymbianTime()
{
    time_t t = time(nullptr);
    t -= KSymbianTimeUnixOffsetInSeconds;
    t += KSecondsTo2000AD;
    t *= 1000000;       // milliseconds
    iTimeLo=(uint32_t)t;
    iTimeHi = (uint32_t)t>>32;
}

uint32_t SymbianTime::TimeLo() const
{
    return iTimeLo;
}

uint32_t SymbianTime::TimeHi() const
{
    return iTimeHi;
}
