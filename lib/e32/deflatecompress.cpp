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
//

#include <cassert>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <type_traits>

#include "farray.h"
#include "huffman.h"

using std::min;


void DeflateL(const uint8_t* aBuf, int32_t aLength, TBitOutput& aOutput);

/**
Class HDeflateHash
@internalComponent
@released
*/
class HDeflateHash
{
	public:
		inline static HDeflateHash* NewLC(int32_t aLinks);
//
		inline int32_t First(const uint8_t* aPtr,int32_t aPos);
		inline int32_t Next(int32_t aPos,int32_t aOffset) const;
	private:
		inline HDeflateHash();
		inline static int32_t Hash(const uint8_t* aPtr);
	private:
		typedef uint16_t TOffset;
	private:
		int32_t iHash[256];
		TOffset iOffset[1];	// or more
};

/**
Class MDeflater
@internalComponent
@released
*/
class MDeflater
{
	public:
		void DeflateL(const uint8_t* aBase,int32_t aLength);
	private:
		const uint8_t* DoDeflateL(const uint8_t* aBase,const uint8_t* aEnd,HDeflateHash& aHash);
		static int32_t Match(const uint8_t* aPtr,const uint8_t* aEnd,int32_t aPos,HDeflateHash& aHas);
		void SegmentL(int32_t aLength,int32_t aDistance);
		virtual void LitLenL(int32_t aCode) =0;
		virtual void OffsetL(int32_t aCode) =0;
		virtual void ExtraL(int32_t aLen, uint32_t aBits) =0;
};

/**
Class TDeflateStats
@internalComponent
@released
*/
class TDeflateStats : public MDeflater
{
	public:
		explicit inline TDeflateStats(TEncoding& aEncoding);
	private:
		// from MDeflater
		void LitLenL(int32_t aCode);
		void OffsetL(int32_t aCode);
		void ExtraL(int32_t aLen,uint32_t aBits){}
	private:
		TEncoding& iEncoding;
};

/**
Class TDeflater
@internalComponent
@released
*/
class TDeflater : public MDeflater
{
	public:
		inline TDeflater(TBitOutput& aOutput,const TEncoding& aEncoding);
	private:
		// from MDeflater
		void LitLenL(int32_t aCode);
		void OffsetL(int32_t aCode);
		void ExtraL(int32_t aLen,uint32_t aBits);
	private:
		TBitOutput& iOutput;
		const TEncoding& iEncoding;
};


/**
Constructor for class HDeflateHash
@internalComponent
@released
*/
inline HDeflateHash::HDeflateHash()
{int32_t* p=iHash+256;do *--p=-KDeflateMaxDistance-1; while (p>iHash);}

/**
@Leave - OutOfMemory
This function allocates memory for HDeflateHash
@param aLinks
@return pointer to allocated memory
@internalComponent
@released
*/
inline HDeflateHash* HDeflateHash::NewLC(int32_t aLinks)
{
#if __GNUC__ >= 4 || _MSC_VER
	// Try to detect if the class' layout has changed.
	static_assert( sizeof(HDeflateHash) == 1028, "sizeof(HDeflateHash) != 1028" );
	static_assert( sizeof(TOffset) == 2, "sizeof(TOffset) != 2" );
	static_assert( offsetof(HDeflateHash, iHash) < offsetof(HDeflateHash, iOffset),
               "offsetof(HDeflateHash, iHash) !< offsetof(HDeflateHash, iOffset)" );

	// Compute the size of the class, including rounding it up to a multiple of 4
	// bytes.

	unsigned n = sizeof(int32_t) * 256 + sizeof(TOffset) * (min)(aLinks, KDeflateMaxDistance);

	while (n & 0x1f)
	{
		n++;
	}

	// Allocate the raw memory ...
	void* p = ::operator new(n);

	// ... And create the object in that memory.
	return new(p) HDeflateHash;
#else
	return new(new char[offsetof(HDeflateHash,iOffset[(min)(aLinks,KDeflateMaxDistance)])]) HDeflateHash;
#endif
}

/**
Hash function for HDeflateHash
@param aPtr
@return Hash value
@internalComponent
@released
*/
inline int32_t HDeflateHash::Hash(const uint8_t* aPtr)
{
	uint32_t x=aPtr[0]|(aPtr[1]<<8)|(aPtr[2]<<16);
	return (x*KDeflateHashMultiplier)>>KDeflateHashShift;
}

/**
Function First
@param aPtr
@param aPos
@internalComponent
@released
*/
inline int32_t HDeflateHash::First(const uint8_t* aPtr,int32_t aPos)
{
	int32_t h=Hash(aPtr);
	int32_t offset=(min)(aPos-iHash[h],KDeflateMaxDistance<<1);
	iHash[h]=aPos;
	iOffset[aPos&(KDeflateMaxDistance-1)]=TOffset(offset);
	return offset;
}

/**
Function Next
@param aPtr
@param aPos
@internalComponent
@released
*/
inline int32_t HDeflateHash::Next(int32_t aPos,int32_t aOffset) const
{return aOffset+iOffset[(aPos-aOffset)&(KDeflateMaxDistance-1)];}


// Class TDeflater
//
// generic deflation algorithm, can do either statistics and the encoder

/**
Function Match
@param aPtr
@param aEnd
@param aPos
@param aHash
@internalComponent
@released
*/
int32_t MDeflater::Match(const uint8_t* aPtr,const uint8_t* aEnd,int32_t aPos,HDeflateHash& aHash)
{
	int32_t offset=aHash.First(aPtr,aPos);
	if (offset>KDeflateMaxDistance)
		return 0;
	int32_t match=0;
	aEnd=(min)(aEnd,aPtr+KDeflateMaxLength);
	uint8_t c=*aPtr;
	do
	{
		const uint8_t* p=aPtr-offset;
		if (p[match>>16]==c)
		{	// might be a better match
			const uint8_t* m=aPtr;
			for (;;)
			{
				if (*p++!=*m++)
					break;
				if (m<aEnd)
					continue;
				return ((m-aPtr)<<16)|offset;
			}
			int32_t left=m-aPtr-1;
			if (left>match>>16)
			{
				match=(left<<16)|offset;
				c=m[-1];
			}
		}
		offset=aHash.Next(aPos,offset);
	} while (offset<=KDeflateMaxDistance);
	return match;
}

/*
Apply the deflation algorithm to the data [aBase,aEnd)
Return a pointer after the last byte that was deflated (which may not be aEnd)
@param aBase
@param aEnd
@param aHash
@internalComponent
@released
*/
const uint8_t* MDeflater::DoDeflateL(const uint8_t* aBase,const uint8_t* aEnd,HDeflateHash& aHash)
{
	const uint8_t* ptr=aBase;
	int32_t prev=0;		// the previous deflation match
	do
	{
		int32_t match=Match(ptr,aEnd,ptr-aBase,aHash);
// Extra deflation applies two optimisations which double the time taken
// 1. If we have a match at p, then test for a better match at p+1 before using it
// 2. When we have a match, add the hash links for all the data which will be skipped
		if (match>>16 < prev>>16)
		{	// use the previous match--it was better
			int32_t len=prev>>16;
			SegmentL(len,prev-(len<<16));
			// fill in missing hash entries for better compression
			const uint8_t* e=ptr+len-2;
			do
			{
				++ptr;
				if (ptr + 2 < aEnd)
				  aHash.First(ptr,ptr-aBase);
			} while (ptr<e);
			prev=0;
		}
		else if (match<=(KDeflateMinLength<<16))
			LitLenL(*ptr);			// no deflation match here
		else
		{	// save this match and test the next position
			if (prev)	// we had a match at ptr-1, but this is better
				LitLenL(ptr[-1]);
			prev=match;
		}
		++ptr;
	} while (ptr+KDeflateMinLength-1<aEnd);
	if (prev)
	{		// emit the stored match
		int32_t len=prev>>16;
		SegmentL(len,prev-(len<<16));
		ptr+=len-1;
	}
	return ptr;
}

/*
The generic deflation algorithm
@param aBase
@param aLength
@internalComponent
@released
*/
void MDeflater::DeflateL(const uint8_t* aBase,int32_t aLength)
{
	const uint8_t* end=aBase+aLength;
	if (aLength>KDeflateMinLength)
	{	// deflation kicks in if there is enough data
		HDeflateHash* hash=HDeflateHash::NewLC(aLength);

		aBase=DoDeflateL(aBase,end,*hash);
		delete hash;
	}
	while (aBase<end)					// emit remaining bytes
		LitLenL(*aBase++);
	LitLenL(TEncoding::EEos);	// eos marker
}

/*
Turn a (length,offset) pair into the deflation codes+extra bits before calling the specific
LitLen(), Offset() and Extra() functions.
@param aLength
@param aDistance
@internalComponent
@released
*/
void MDeflater::SegmentL(int32_t aLength,int32_t aDistance)
{
	aLength-=KDeflateMinLength;
	int32_t extralen=0;
	uint32_t len=aLength;
	while (len>=8)
	{
		++extralen;
		len>>=1;
	}
	LitLenL((extralen<<2)+len+TEncoding::ELiterals);
	if (extralen)
		ExtraL(extralen,aLength);
//
	aDistance--;
	extralen=0;
	uint32_t dist=aDistance;
	while (dist>=8)
	{
		++extralen;
		dist>>=1;
	}
	OffsetL((extralen<<2)+dist);
	if (extralen)
		ExtraL(extralen,aDistance);
}

/**
Class TDeflateStats
This class analyses the data stream to generate the frequency tables
for the deflation algorithm
@internalComponent
@released
*/
inline TDeflateStats::TDeflateStats(TEncoding& aEncoding)
	:iEncoding(aEncoding)
	{}
/*
Function LitLenL
@Leave
@param aCode
@internalComponent
@released
*/
void TDeflateStats::LitLenL(int32_t aCode)
{
	++iEncoding.iLitLen[aCode];
}

/*
@Leave ArrayIndexOutOfBounds
Finction OffsetL
@param aCode
@internalComponent
@released
*/
void TDeflateStats::OffsetL(int32_t aCode)
{
	++iEncoding.iDistance[aCode];
}

/**
Constructor of Class TDeflater
Extends MDeflater to provide huffman encoding of the output
@internalComponent
@released
*/
inline TDeflater::TDeflater(TBitOutput& aOutput,const TEncoding& aEncoding)
//
// construct for encoding
//
	:iOutput(aOutput),iEncoding(aEncoding)
	{}

/*
Function LitLenL
@Leave
@param aCode
@internalComponent
@released
*/
void TDeflater::LitLenL(int32_t aCode)
{
	iOutput.HuffmanL(iEncoding.iLitLen[aCode]);
}

/*
Function OffsetL
@Leave
@param aCdoe
@internalComponent
@released
*/
void TDeflater::OffsetL(int32_t aCode)
{
	iOutput.HuffmanL(iEncoding.iDistance[aCode]);
}

/*
Function ExtraL
@Leave
@param  aLen
@param aBits
@internalComponent
@released
*/
void TDeflater::ExtraL(int32_t aLen,uint32_t aBits)
{
	iOutput.WriteL(aBits,aLen);
}
/*
Function DoDeflateL
@Leave
@param aBuf
@param aLength
@param aOutput
@param aEncoding
@internalComponent
@released
*/
void DoDeflateL(const uint8_t* aBuf,int32_t aLength,TBitOutput& aOutput,TEncoding& aEncoding)
{
// analyse the data for symbol frequency
	TDeflateStats analyser(aEncoding);
	analyser.DeflateL(aBuf,aLength);

// generate the required huffman encodings
	Huffman::HuffmanL(aEncoding.iLitLen,TEncoding::ELitLens,aEncoding.iLitLen);
	Huffman::HuffmanL(aEncoding.iDistance,TEncoding::EDistances,aEncoding.iDistance);

// Store the encoding table
	Huffman::ExternalizeL(aOutput,aEncoding.iLitLen,KDeflationCodes);

// generate the tables
	Huffman::Encoding(aEncoding.iLitLen,TEncoding::ELitLens,aEncoding.iLitLen);
	Huffman::Encoding(aEncoding.iDistance,TEncoding::EDistances,aEncoding.iDistance);

// now finally deflate the data with the generated encoding
	TDeflater deflater(aOutput,aEncoding);
	deflater.DeflateL(aBuf,aLength);
	aOutput.PadL(1);
}

/*
Function DeflateL
@Leave
@param aBuf
@param aLength
@param aOutput
@internalComponent
@released
*/
void DeflateL(const uint8_t* aBuf, int32_t aLength, TBitOutput& aOutput)
{
	TEncoding* encoding=new TEncoding();
	DoDeflateL(aBuf,aLength,aOutput,*encoding);
	delete encoding;
}
/*
Function DeflateCompress
@param bytes
@param size
@param os
@internalComponent
@released
*/
void DeflateCompress(char *bytes,size_t size, std::ofstream & os)
{
	TFileOutput* output=new TFileOutput(os);
	output->iDataCount = 0;
	DeflateL((uint8_t*)bytes,size,*output);
	output->FlushL();
	delete output;
}

size_t CompressDeflate(const char* src, int srcsize, const char* dst, int dstsize)
{
	TBufferedOutput* output=new TBufferedOutput(dst, dstsize);
	DeflateL((const uint8_t*)src, srcsize, *output);
	ptrdiff_t r = output->GetCompressedSize();
	delete output;
	return r;
}
