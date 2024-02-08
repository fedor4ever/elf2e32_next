 /*
 Copyright (c) 2024 Strizhniou Fiodar
 All rights reserved.
 This component and the accompanying materials are made available
 under the terms of "Eclipse Public License v1.0"
 which accompanies this distribution, and is available
 at the URL "http://www.eclipse.org/legal/epl-v10.html".

 Initial Contributors:
 Strizhniou Fiodar - initial contribution.

 Contributors:

 Description:
 Creates CRC32 checksums from DSO.

 Validate DSO:   --dso=tests\libcrypto{000a0000}.dso --filecrc=tests\libcrypto-2.4.5.SDK.dcrc
 CRC32 from DSO: --dso=tests\libcrypto{000a0000}.dso --filecrc
 */

#include "common.hpp"
#include "dsocrcfile.h"

DSOCrcFile::DSOCrcFile(const Args* args): iArgs(args) {}

void DSOCrcFile::Run()
{
    CheckDSOCrc(iArgs);
}
