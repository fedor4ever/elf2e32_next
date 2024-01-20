// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implementation of the Inflater for e32 image dump for the elf2e32 tool
// @internalComponent
// @released
//
//

#include <memory.h>
#include "common.hpp"
#include "inflate.h"

/*
Inline constructor for CInflater
@param aInput
@internalComponent
@released
*/
inline CInflater::CInflater(TBitInput& aInput):iBits(&aInput),iEncoding(nullptr),iOut(nullptr)
{
}

/*
Function for the 2nd phase construction.
@param
@internalComponent
@released
*/
void CInflater::ConstructL()
{
	iEncoding=new TEncoding;
	InitL();
	iLen=0;
	iOut=new uint8_t[KDeflateMaxDistance];
	iAvail=iLimit=iOut;
}

/*
Function NewLC
@Leave OutOfMemory
@param aInput
@return pointer to self
@internalComponent
@released
*/
CInflater* CInflater::NewLC(TBitInput& aInput)
{
	CInflater* self=new CInflater(aInput);
	self->ConstructL();

	return self;
}

/*
Destructor for CInflater
@internalComponent
@released
*/
CInflater::~CInflater()
{
	delete iEncoding;
	delete [] iOut;
}

/*
Function ReadL
@Leave
@param aBuffer
@param aLength
@internalComponent
@released
*/
int32_t CInflater::ReadL(uint8_t* aBuffer,int32_t aLength)
{
	int32_t tfr=0;
	for (;;)
	{
		int32_t len;
		if(aLength > (iLimit-iAvail))
			len=iLimit-iAvail;
		else
			len=aLength;

		if (len && aBuffer)
		{
			memcpy((void * const)aBuffer,(const void * const)iAvail,(size_t)len);
			aBuffer+=len;
		}
		aLength-=len;
		iAvail+=len;
		tfr+=len;
		if (aLength==0)
			return tfr;
		len=InflateL();
		if (len==0)
			return tfr;
		iAvail=iOut;
		iLimit=iAvail+len;
	}
}

/*
Function InitL
@Leave
@internalComponent
@released
*/
void CInflater::InitL()
{
	// read the encoding
	Huffman::InternalizeL(*iBits,iEncoding->iLitLen,KDeflationCodes);
	// validate the encoding
	if (!Huffman::IsValid(iEncoding->iLitLen,TEncoding::ELitLens) ||
		!Huffman::IsValid(iEncoding->iDistance,TEncoding::EDistances))
	{
		ReportError(HUFFMANINVALIDCODINGERROR);
	}

	// convert the length tables into huffman decoding trees
	Huffman::Decoding(iEncoding->iLitLen,TEncoding::ELitLens,iEncoding->iLitLen);
	Huffman::Decoding(iEncoding->iDistance,TEncoding::EDistances,iEncoding->iDistance,KDeflateDistCodeBase);
}

/*
Consume all data lag in the history buffer, then decode to fill up the output buffer
Return the number of available bytes in the output buffer. This is only ever less than the
buffer size if the end of stream marker has been read.
@internalComponent
@released
*/
int32_t CInflater::InflateL()
{
	// empty the history buffer into the output
	uint8_t* out=iOut;
	uint8_t* const end=out+KDeflateMaxDistance;
	const uint32_t* tree=iEncoding->iLitLen;
	if (iLen<0)	// EOF
		return 0;
	if (iLen>0)
		goto useHistory;

	while (out<end)
	{
		// get a huffman code
		{
			int32_t val=iBits->HuffmanL(tree)-TEncoding::ELiterals;
			if (val<0)
			{
				*out++=uint8_t(val);
				continue;			// another literal/length combo
			}
			if (val==TEncoding::EEos-TEncoding::ELiterals)
			{	// eos marker. we're done
				iLen=-1;
				break;
			}

			// get the extra bits for the code
			int32_t code=val&0xff;
			if (code>=8)
			{	// xtra bits
				int32_t xtra=(code>>2)-1;
				code-=xtra<<2;
				code<<=xtra;
				code|=iBits->ReadL(xtra);
			}
			if (val<KDeflateDistCodeBase-TEncoding::ELiterals)
			{	// length code... get the code
				iLen=code+KDeflateMinLength;
				tree=iEncoding->iDistance;
				continue;			// read the huffman code
			}
			// distance code
			iRptr=out-(code+1);
			if(iRptr+KDeflateMaxDistance<end)
                iRptr+=KDeflateMaxDistance;
		}
		useHistory:
			int32_t tfr;
			if(iLen > (end-out))
				tfr=end-out;
			else
				tfr=iLen;

			iLen-=tfr;
			const uint8_t* from=iRptr;
			do
			{
				*out++=*from++;
				if (from==end)
					from-=KDeflateMaxDistance;
			}while (--tfr!=0);
			iRptr=from;
			tree=iEncoding->iLitLen;
	};

	return out-iOut;
}

/*
TFileInput Constructor
@param source
@param size
@internalComponent
@released
*/
TFileInput::TFileInput(unsigned char* source,int size):iReadBuf(source),iSize(size)
{
	Set(source,iSize*8);
}

/*
Function UnderFlowL
@Leave CommonError
@internalComponent
@released
*/
void TFileInput::UnderflowL()
{
	ReportError(HUFFMANBUFFERUNDERFLOWERROR);
}

/*
Function InflateUncompress
@param source
@param sourcesize
@param dest
@param destsize
@internalComponent
@released
*/
void DeCompressInflate(unsigned char* source, int sourcesize,unsigned char* dest, int destsize)
{
	TFileInput* input = new TFileInput(source, sourcesize);
	CInflater* inflater=CInflater::NewLC(*input);
	inflater->ReadL(dest,destsize);
	delete input;
	delete inflater;
}

