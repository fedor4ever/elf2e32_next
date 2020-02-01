// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017 Strizhniou Fiodar
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
// Class for inflater
// @internalComponent
// @released
//
//

#ifndef __INFLATE_H__
#define __INFLATE_H__

#include "huffman.h"


/**
Class for inflater
@internalComponent
@released
*/
class CInflater
{
	public:
		enum {EBufSize = 0x800, ESafetyZone=8};
	public:
		static CInflater* NewLC(TBitInput& aInput);
		~CInflater();
		int32_t ReadL(uint8_t* aBuffer,int32_t aLength);
		int32_t SkipL(int32_t aLength);
	private:
		CInflater(TBitInput& aInput);
		void ConstructL();
		void InitL();
		int32_t InflateL();
	private:
		TBitInput* iBits;
		const uint8_t* iRptr;			// partial segment
		int32_t iLen;
		const uint8_t* iAvail;			// available data
		const uint8_t* iLimit;
		TEncoding* iEncoding;
		uint8_t* iOut;					// circular buffer for distance matches
		uint8_t iHuff[EBufSize+ESafetyZone];	// huffman data
};


#endif

