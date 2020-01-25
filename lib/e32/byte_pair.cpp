// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2020 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors: Strizhniou Fiodar - fix build and runtime errors, refactoring.
//
// Description:
//

#include <string.h>
#include <assert.h>
#include "byte_pair.h"
#include "e32common.h"

const int32_t MaxBlockSize = 0x1000;

uint16_t PairCount[0x10000];
uint16_t PairBuffer[MaxBlockSize*2];

uint16_t GlobalPairs[0x10000] = {0};
uint16_t GlobalTokenCounts[0x100] = {0};

uint16_t ByteCount[0x100+4];

void CountBytes(uint8_t* data, int32_t size)
	{
	memset(ByteCount,0,sizeof(ByteCount));
	uint8_t* dataEnd = data+size;
	while(data<dataEnd)
		++ByteCount[*data++];
	}


inline void ByteUsed(int32_t b)
	{
	ByteCount[b] = 0xffff;
	}


// 11915620
// 11913551  	return -ByteCount[b1]-ByteCount[b2];
// 11913185

#if 0
int TieBreak(int b1,int b2)
	{
	int i;
	int x = 0;
	for(i=0; i<0x100; i++)
		x += PairCount[(b1<<8)+i];
	int y = 0;
	for(i=b2; i<0x10000; i+=0x100)
		y += PairCount[i];
	return -x-y;
	}
#endif

int TieBreak(int b1,int b2)
	{
	return -ByteCount[b1]-ByteCount[b2];
	}

int32_t MostCommonPair(int32_t& pair, uint8_t* data, int32_t size, int32_t minFrequency, int32_t marker)
{
	memset(PairCount,0,sizeof(PairCount));
	uint8_t* dataEnd = data+size-1;
	int32_t pairsFound = 0;
	int32_t lastPair = -1;
	while(data<dataEnd)
    {
		int32_t b1 = *data++;
		if(b1==marker)
        {
			// skip marker and following byte
			lastPair = -1;
			++data;
			continue;
        }
		int32_t b2 = *data;
		if(b2==marker)
        {
			// skip marker and following byte
			lastPair = -1;
			data+=2;
			continue;
        }
		int32_t p = (b2<<8)|b1;
		if(p==lastPair)
        {
			// ensure a pair of identical bytes don't get double counted
			lastPair = -1;
			continue;
        }
		lastPair = p;
		++PairCount[p];
		if(PairCount[p]==minFrequency)
			PairBuffer[pairsFound++] = (uint16_t)p;
    }

	int32_t bestCount = -1;
	int32_t bestPair = -1;
	int32_t bestTieBreak = 0;
	int32_t p;
	while(pairsFound--)
    {
		p = PairBuffer[pairsFound];
		int32_t f=PairCount[p];
		if(f>bestCount)
        {
			bestCount = f;
			bestPair = p;
			bestTieBreak = TieBreak(p&0xff,p>>8);
        }
		else if(f==bestCount)
        {
			int32_t tieBreak = TieBreak(p&0xff,p>>8);
			if(tieBreak>bestTieBreak)
            {
				bestCount = f;
				bestPair = p;
				bestTieBreak = tieBreak;
            }
        }
    }
	pair = bestPair;
	return bestCount;
}


int32_t LeastCommonByte(int32_t& byte)
{
	int32_t bestCount = 0xffff;
	int32_t bestByte = -1;
	for(int32_t b=0; b<0x100; b++)
    {
		int32_t f = ByteCount[b];
		if(f<bestCount)
        {
			bestCount = f;
			bestByte = b;
        }
    }
	byte = bestByte;
	return bestCount;
}


int32_t Pak(uint8_t* dst, uint8_t* src, int32_t size)
{
	int32_t originalSize = size;
	uint8_t* dst2 = dst+size*2;
	uint8_t* in = src;
	uint8_t* out = dst;

	uint8_t tokens[0x100*3];
	int32_t tokenCount = 0;

	CountBytes(in,size);

	int32_t marker = -1;
	int32_t overhead = 1+3+LeastCommonByte(marker);
	ByteUsed(marker);

	uint8_t* inEnd = in+size;
	uint8_t* outStart = out;
	while(in<inEnd)
    {
		int32_t b=*in++;
		if(b==marker)
			*out++ = (uint8_t)b;
		*out++ = (uint8_t)b;
    }
	size = out-outStart;

	int32_t outToggle = 1;
	in = dst;
	out = dst2;

	for(int32_t r=256; r>0; --r)
    {
		int32_t byte;
		int32_t byteCount = LeastCommonByte(byte);
		int32_t pair;
		int32_t pairCount = MostCommonPair(pair,in,size,overhead+1,marker);
		int32_t saving = pairCount-byteCount;
		if(saving<=overhead)
			break;

		overhead = 3;
		if(tokenCount>=32)
			overhead = 2;

		uint8_t* d=tokens+3*tokenCount;
		++tokenCount;
		*d++ = (uint8_t)byte;
		ByteUsed(byte);
		*d++ = (uint8_t)pair;
		ByteUsed(pair&0xff);
		*d++ = (uint8_t)(pair>>8);
		ByteUsed(pair>>8);
		++GlobalPairs[pair];

		inEnd = in+size;
		outStart = out;
		while(in<inEnd)
        {
			int32_t b=*in++;
			if(b==marker)
            {
				*out++ = (uint8_t)marker;
				b = *in++;
            }
			else if(b==byte)
            {
				*out++ = (uint8_t)marker;
				--byteCount;
            }
			else if(b==(pair&0xff) && in<inEnd && *in==(pair>>8))
            {
				++in;
				b = byte;
				--pairCount;
            }
			*out++ = (uint8_t)b;
        }
		assert(!byteCount);
		assert(!pairCount);
		size = out-outStart;

		outToggle ^= 1;
		if(outToggle)
        {
			in = dst;
			out = dst2;
        }
		else
        {
			in = dst2;
			out = dst;
        }
    }

	// sort tokens with a bubble sort...
	for(int32_t x=0; x<tokenCount-1; x++)
		for(int32_t y=x+1; y<tokenCount; y++)
			if(tokens[x*3]>tokens[y*3])
            {
				int32_t z = tokens[x*3];
				tokens[x*3] = tokens[y*3];
				tokens[y*3] = (uint8_t)z;
				z = tokens[x*3+1];
				tokens[x*3+1] = tokens[y*3+1];
				tokens[y*3+1] = (uint8_t)z;
				z = tokens[x*3+2];
				tokens[x*3+2] = tokens[y*3+2];
				tokens[y*3+2] = (uint8_t)z;
            }

	// check for not being able to compress...
	if(size>originalSize)
    {
		*dst++ = 0; // store zero token count
		memcpy(dst,src,originalSize); // store original data
		return originalSize+1;
    }

	// make sure data is in second buffer (dst2)
	if(in!=dst2)
		memcpy(dst2,dst,size);

	// store tokens...
	uint8_t* originalDst = dst;
	*dst++ = (uint8_t)tokenCount;
	if(tokenCount)
    {
		*dst++ = (uint8_t)marker;
		if(tokenCount<32)
        {
			memcpy(dst,tokens,tokenCount*3);
			dst += tokenCount*3;
        }
		else
        {
			uint8_t* bitMask = dst;
			memset(bitMask,0,32);
			dst += 32;
			uint8_t* d=tokens;
			do
				{
				int32_t t=*d++;
				bitMask[t>>3] |= (1<<(t&7));
				*dst++ = *d++;
				*dst++ = *d++;
				}
			while(--tokenCount);
        }
    }
	// store data...
	memcpy(dst,dst2,size);
	dst += size;

	// get stats...
	++GlobalTokenCounts[tokenCount];

	// return total size of compressed data...
	return dst-originalDst;
}


int32_t Unpak(uint8_t* dst, uint8_t* src, int32_t srcSize, uint8_t*& srcNext)
{
	uint8_t* dstStart = dst;
	uint8_t* dstEnd = dst + MaxBlockSize;
	uint8_t* srcEnd = src+srcSize;

	uint32_t LUT[0x100/2];
	uint8_t* LUT0 = (uint8_t*)LUT;
	uint8_t* LUT1 = LUT0+0x100;

	uint8_t stack[0x100];
	uint8_t* stackStart = stack+sizeof(stack);
	uint8_t* sp = stackStart;

	uint32_t marker = ~0u;
	int32_t numTokens;
	uint32_t p1;
	uint32_t p2;

	uint32_t* lut = (uint32_t*)LUT;
	uint32_t b = 0x03020100;
	uint32_t step = 0x04040404;
	do
    {
		*lut++ = b;
		b += step;
    }
	while(b>step);

	if(src>=srcEnd)
		goto error;
	numTokens = *src++;
	if(numTokens)
    {
		if(src>=srcEnd)
			goto error;
		marker = *src++;
		LUT0[marker] = (uint8_t)~marker;

		if(numTokens<32)
        {
			uint8_t* tokenEnd = src+3*numTokens;
			if(tokenEnd>srcEnd)
				goto error;
			do
            {
				int32_t b = *src++;
				int32_t p1 = *src++;
				int32_t p2 = *src++;
				LUT0[b] = (uint8_t)p1;
				LUT1[b] = (uint8_t)p2;
            }while(src<tokenEnd);
        }
		else
        {
			uint8_t* bitMask = src;
			src += 32;
			if(src>srcEnd)
				goto error;
			int32_t b=0;
			do
            {
                uint8_t mask = bitMask[b>>3];
                if(mask&(1<<(b&7)))
                {
                    if(src>srcEnd)
                        goto error;
                    int32_t p1 = *src++;
                    if(src>srcEnd)
                        goto error;
                    int32_t p2 = *src++;
                    LUT0[b] = (uint8_t)p1;
                    LUT1[b] = (uint8_t)p2;
                    --numTokens;
                }
                ++b;
            }while(b<0x100);

			if(numTokens)
				goto error;
        }
    }

	if(src>=srcEnd)
		goto error;
	b = *src++;
	if(dst>=dstEnd)
		goto error;
	p1 = LUT0[b];
	if(p1!=b)
		goto not_single;
next:
	if(src>=srcEnd)
		goto done_s;
	b = *src++;
	*dst++ = (uint8_t)p1;
	if(dst>=dstEnd)
		goto done_d;
	p1 = LUT0[b];
	if(p1==b)
		goto next;

not_single:
	if(b==marker)
		goto do_marker;

do_pair:
	p2 = LUT1[b];
	b = p1;
	p1 = LUT0[b];
	if(sp<=stack)
		goto error;
	*--sp = (uint8_t)p2;

recurse:
	if(b!=p1)
		goto do_pair;

	if(sp==stackStart)
		goto next;
	b = *sp++;
	if(dst>=dstEnd)
		goto error;
	*dst++ = (uint8_t)p1;
	p1 = LUT0[b];
	goto recurse;

do_marker:
	if(src>=srcEnd)
		goto error;
	p1 = *src++;
	goto next;

error:
	srcNext = nullptr;
	return KErrCorrupt;

done_s:
	*dst++ = (uint8_t)p1;
	srcNext = src;
	return dst-dstStart;

done_d:
	if(dst>=dstEnd)
		--src;
	srcNext = src;
	return dst-dstStart;
}


uint8_t PakBuffer[MaxBlockSize*4];
uint8_t UnpakBuffer[MaxBlockSize];


int32_t BytePairCompress(uint8_t* dst, uint8_t* src, int32_t size)
{
	assert(size<=MaxBlockSize);
	int32_t compressedSize = Pak(PakBuffer,src,size);
	uint8_t* pakEnd;
	int32_t us = Unpak(UnpakBuffer, PakBuffer, compressedSize, pakEnd);
	assert(us==size);
	assert(pakEnd==PakBuffer+compressedSize);
	assert(!memcmp(src,UnpakBuffer,size));
	if(compressedSize>=size)
		return KErrTooBig;
	memcpy(dst,PakBuffer,compressedSize);
	return compressedSize;
}
