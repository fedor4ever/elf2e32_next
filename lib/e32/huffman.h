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
// Huffman Class for deflate and inflate
// @internalComponent
// @released
//
//

#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include <fstream>

/** Bit output stream.
	Good for writing bit streams for packed, compressed or huffman data algorithms.

	This class must be derived from and OverflowL() reimplemented if the bitstream data
	cannot be generated into a single memory buffer.
*/
class TBitOutput
{
	public:
		TBitOutput();
		TBitOutput(uint8_t* aBuf, int32_t aSize);
		inline void Set(uint8_t* aBuf, int32_t aSize);
		inline const uint8_t* Ptr() const;
		inline int32_t BufferedBits() const;
		void WriteL(uint32_t aValue, int32_t aLength);
		void HuffmanL(uint32_t aHuffCode);
		void PadL(uint32_t aPadding);
		virtual ~TBitOutput() = default;
	private:
		void DoWriteL(uint32_t aBits, int32_t aSize);
		virtual void OverflowL() = 0;
	private:
		uint32_t iCode;		// code in production
		int32_t iBits;
		uint8_t* iPtr;
		uint8_t* iEnd;
};

/**
Set the memory buffer to use for output

Data will be written to this buffer until it is full, at which point OverflowL() will be
called. This should handle the data and then can Set() again to reset the buffer for further
output.

@param "uint8_t* aBuf" The buffer for output
@param "int32_t aSize" The size of the buffer in bytes
*/
inline void TBitOutput::Set(uint8_t* aBuf,int32_t aSize)
{
	iPtr=aBuf;
	iEnd=aBuf+aSize;
}

/**
Get the current write position in the output buffer

In conjunction with the address of the buffer, which should be known to the caller, this
describes the data in the bitstream.
*/
inline const uint8_t* TBitOutput::Ptr() const
{return iPtr;}

/**
Get the number of bits that are buffered

This reports the number of bits that have not yet been written into the output buffer.It will
always lie in the range 0..7. Use PadL() to pad the data out to the next byte and write it to
the buffer.
*/
inline int32_t TBitOutput::BufferedBits() const
{
	return iBits+8;
}

/**
This class is derived from TBitOutput
@internalComponent
@released
*/
class TFileOutput : public TBitOutput
{
	enum {KBufSize=0x1000};
	public:
		explicit TFileOutput(std::ofstream & os);
		void FlushL();
		uint32_t iDataCount = 0;
		virtual ~TFileOutput() = default;
	private:
		void OverflowL() override;
	private:
		std::ofstream & iOutStream;
		uint8_t iBuf[KBufSize];
};

/**
This class is derived from TBitOutput
@internalComponent
@released
*/
class TBufferedOutput : public TBitOutput
{
	public:
		explicit TBufferedOutput(const char* dst, uint32_t dstSize);
		void FlushL();
		ptrdiff_t GetCompressedSize() const;
		virtual ~TBufferedOutput() = default;
	private:
		void OverflowL() override;
	private:
		const char* iOutStream;
};

/**
Class for Bit input stream.
Good for reading bit streams for packed, compressed or huffman data algorithms.
@since 8.0
@library euser.lib
@internalComponent
@released
*/
class TBitInput
{
public:
    TBitInput();
    TBitInput(const uint8_t* aPtr, int32_t aLength, int32_t aOffset=0);
    void Set(const uint8_t* aPtr, int32_t aLength, int32_t aOffset=0);
    uint32_t ReadL();
    uint32_t ReadL(int32_t aSize);
    uint32_t HuffmanL(const uint32_t* aTree);
    virtual ~TBitInput();
private:
    virtual void UnderflowL();
private:
    int32_t iCount;
    uint32_t iBits;
    int32_t iRemain;
    const uint32_t* iPtr;
};

/**
Class derived from TBitInput
@internalComponent
@released
*/
class TFileInput : public TBitInput
{
	public:
		TFileInput(unsigned char* source,int size);
		virtual ~TFileInput(){}
	private:
		void UnderflowL();
	private:
		uint8_t* iReadBuf;
		int32_t iSize;
};

/*
Class for Huffman code toolkit.

This class builds a huffman encoding from a frequency table and builds a decoding tree from a
code-lengths table.

The encoding generated is based on the rule that given two symbols s1 and s2, with code
length l1 and l2, and huffman codes h1 and h2:
	if l1<l2 then h1<h2 when compared lexicographically
	if l1==l2 and s1<s2 then h1<h2 ditto

This allows the encoding to be stored compactly as a table of code lengths

@since 8.0
@library euser.lib
@internalComponent
@released
*/
class Huffman
{
	public:
		enum {KMaxCodeLength=27};
		enum {KMetaCodes=KMaxCodeLength+1};
		enum {KMaxCodes=0x8000};
	public:
		static void HuffmanL(const uint32_t aFrequency[],int32_t aNumCodes,uint32_t aHuffman[]);
		static void Encoding(const uint32_t aHuffman[],int32_t aNumCodes,uint32_t aEncodeTable[]);
		static bool IsValid(const uint32_t aHuffman[],int32_t aNumCodes);
		static void ExternalizeL(TBitOutput& aOutput,const uint32_t aHuffman[],int32_t aNumCodes);
		static void Decoding(const uint32_t aHuffman[],int32_t aNumCodes,uint32_t aDecodeTree[],int32_t aSymbolBase=0);
		static void InternalizeL(TBitInput& aInput,uint32_t aHuffman[],int32_t aNumCodes);
};

// local definitions used for Huffman code generation
typedef uint16_t THuff;		/** @internal */
const THuff KLeaf=0x8000;	/** @internal */
struct TNode
/** @internal */
{
	uint32_t iCount;
	THuff iLeft;
	THuff iRight;
};

const int32_t KDeflateLengthMag=8;
const int32_t KDeflateDistanceMag=12;
const int32_t KDeflateMaxDistance=(1<<KDeflateDistanceMag);

const int32_t KDeflateDistCodeBase=0x200;

/**
class for TEncoding
@internalComponent
@released
*/
struct TEncoding
{
    enum {ELiterals=256,ELengths=(KDeflateLengthMag-1)*4,ESpecials=1,EDistances=(KDeflateDistanceMag-1)*4};
    enum {ELitLens=ELiterals+ELengths+ESpecials};
    enum {EEos=ELiterals+ELengths};

    uint32_t iLitLen[ELitLens];
    uint32_t iDistance[EDistances];
};

// hashing
const uint32_t KDeflateHashMultiplier=0xAC4B9B19u;
const int32_t KDeflateHashShift=24;

const int32_t KDeflationCodes=TEncoding::ELitLens+TEncoding::EDistances;
const int32_t KDeflateMinLength=3;
const int32_t KDeflateMaxLength=KDeflateMinLength-1 + (1<<KDeflateLengthMag);

#endif

