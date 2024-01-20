// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2024 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors: Strizhniou Fiodar - fix build and runtime errors.
//
// Description:
//

#ifndef BYTE_PAIR_H
#define BYTE_PAIR_H

#include <cstdint>

int32_t BytePairCompress(uint8_t* dst, uint8_t* src, int32_t size);
int32_t Pak(uint8_t* dst, uint8_t* src, int32_t size);
int32_t Unpak(uint8_t* dst, uint8_t* src, int32_t srcSize, uint8_t*& srcNext);

#endif
