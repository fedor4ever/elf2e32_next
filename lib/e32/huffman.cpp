// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2020 Strizhniou Fiodar.
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
// Implementation of the Huffman technique for the elf2e32 tool
// @internalComponent
// @released
//
//

#ifdef _MSC_VER
	#pragma warning(disable: 4710) // function not inlined
#endif

#include <vector>
#include <cassert>

#include "farray.h"
#include "huffman.h"
#include "common.hpp"

/**
Construct a bit stream output object

Following construction the bit stream is ready for writing bits, but will first call
OverflowL() as the output buffer is 'full'. A derived class can detect this state as
Ptr() will return null.
*/
TBitOutput::TBitOutput():iCode(0),iBits(-8),iPtr(nullptr),iEnd(nullptr)
{
}

/**
Construct a bit stream output object over a buffer

Data will be written to the buffer until it is full, at which point OverflowL() will
be called. This should handle the data and then can Set() again to reset the buffer
for further output.

@param "uint8_t* aBuf" The buffer for output
@param "int32_t aSize" The size of the buffer in bytes
*/
TBitOutput::TBitOutput(uint8_t* aBuf, int32_t aSize):iCode(0),iBits(-8),iPtr(aBuf),iEnd(aBuf+aSize)
{
}

/**
Write a huffman code

This expects a huffman code value as generated by Huffman::Encoding()

@param "uint32_t aHuffCode" The huffman code write to the stream
@leave "OverflowL()" If the output buffer is full, OverflowL() is called
*/
void TBitOutput::HuffmanL(uint32_t aHuffCode)
{
	DoWriteL(aHuffCode<<(32-Huffman::KMaxCodeLength),aHuffCode>>Huffman::KMaxCodeLength);
}

/**
Write an arbitrary integer value

Write an unsigned integer using the number of bits specified. Only the low order bits of the
value are written to the output, most significant bit first.

@param "uint32_t aValue" The value to write to the stream
@param "uint32_t aLength" The number of bits to output
@leave "OverflowL()" If the output buffer is full, OverflowL() is called
*/
void TBitOutput::WriteL(uint32_t aValue,int32_t aLength)
{
	if (aLength)
		DoWriteL(aValue<<=32-aLength,aLength);
}

/**
Pad the bitstream to the next byte boundary

Terminate the bitstream by padding the last byte with the requested value.
Following this operation the bitstream can continue to be used, the data will start at the
next byte.

@param "uint32_t aPadding" The bit value to pad the final byte with
@leave "OverflowL()" If the output buffer is full, OverflowL() is called
*/
void TBitOutput::PadL(uint32_t aPadding)
{
	if (iBits>-8)
		WriteL(aPadding?0xffffffffu:0,-iBits);
}

/**
Write the higher order bits to the stream
@internalComponent
@released
*/
void TBitOutput::DoWriteL(uint32_t aBits,int32_t aSize)
{
	if (aSize>25)
	{
		// cannot process >25 bits in a single pass so do the top 8 bits first
		assert(aSize<=32);
		DoWriteL(aBits&0xff000000u,8);
		aBits<<=8;
		aSize-=8;
	}

	int32_t bits=iBits;
	uint32_t code=iCode|(aBits>>(bits+8));
	bits+=aSize;
	if (bits>=0)
	{
		uint8_t* p=iPtr;
		do
		{
			if (p==iEnd)
			{
				// run out of buffer space so invoke the overflow handler
				iPtr=p;
				OverflowL();
				p=iPtr;
				assert(p!=iEnd);
			}
			*p++=uint8_t(code>>24);
			code<<=8;
			bits-=8;
		} while (bits>=0);
		iPtr=p;
	}
	iCode=code;
	iBits=bits;
}

/**
Constructor for class TFileOutput
@internalComponent
@released
*/
TFileOutput::TFileOutput(std::ofstream & os): iOutStream(os)
{// Copyright (c) 2020 Strizhniou Fiodar
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
	Set(iBuf,KBufSize);
}

/**
Function to empty the buffer and reset the pointers
@internalComponent
@released
*/
void TFileOutput::OverflowL()
{
	FlushL();
	Set(iBuf,KBufSize);
}

/**
Function to write out the contents of the buffer
@internalComponent
@released
*/
void TFileOutput::FlushL()
{
	auto len=Ptr()-iBuf;
	if (len)
	{
		iOutStream.write(reinterpret_cast<char *>(iBuf), len); // write extended header
		iDataCount += len;
	}
}

TBufferedOutput::TBufferedOutput(const char* dst, uint32_t dstSize):
    iOutStream(dst)
{
    Set((uint8_t *)dst, dstSize);
}

ptrdiff_t TBufferedOutput::GetCompressedSize() const
{
	return Ptr() - (const uint8_t*)iOutStream;
}

/**
Function to empty the buffer and reset the pointers
@internalComponent
@released
*/
void TBufferedOutput::OverflowL()
{
	ReportError(HUFFMANBUFFEROVERFLOWERROR);
}

/**
Recursive function to calculate the code lengths from the node tree
@internalComponent
@released
*/
void HuffmanLengthsL(uint32_t* aLengths,const TNode* aNodes,int32_t aNode,int32_t aLen)
{
	if (++aLen>Huffman::KMaxCodeLength)
		ReportError(HUFFMANBUFFEROVERFLOWERROR);

	const TNode& node=aNodes[aNode];
	uint32_t x=node.iLeft;
	if (x&KLeaf)
		aLengths[x&~KLeaf]=aLen;
	else
		HuffmanLengthsL(aLengths,aNodes,x,aLen);
	x=node.iRight;
	if (x&KLeaf)
		aLengths[x&~KLeaf]=aLen;
	else
		HuffmanLengthsL(aLengths,aNodes,x,aLen);
}

/**
Function to Insert the {aCount,aValue} pair into the already sorted array of nodes
@internalComponent
@released
*/
void InsertInOrder(TNode* aNodes, int32_t aSize, uint32_t aCount, int32_t aVal)
{
	// Uses Insertion sort following a binary search...
	int32_t left=0, right=aSize;
	while (left < right)
	{
		int32_t m = (left+right) >> 1;
		if (aNodes[m].iCount<aCount)
			right=m;
		else
			left=m+1;
	}
	memmove(aNodes+left+1,aNodes+left,sizeof(TNode)*(aSize-left));
	aNodes[left].iCount=aCount;
	aNodes[left].iRight=uint16_t(aVal);
}

/**
Generate a Huffman code.

This generates a Huffman code for a given set of code frequencies. The output is a table of
code lengths which can be used to build canonincal encoding tables or decoding trees for use
with the TBitInput and TBitOutput classes.

Entries in the table with a frequency of zero will have a zero code length and thus no
associated huffman encoding. If each such symbol should have a maximum length encoding, they
must be given at least a frequency of 1.

For an alphabet of n symbols, this algorithm has a transient memory overhead of 8n, and a
time complexity of O(n*log(n)).

@param "const uint32_t aFrequency[]" The table of code frequencies
@param "int32_t aNumCodes" The number of codes in the table
@param "uint32_t aHuffman[]" The table for the output code-length table. This must be the same
size as the frequency table, and can safely be the same table

@leave "KErrNoMemory" If memory used for code generation cannot be allocated
@panic "USER ???" If the number of codes exceeds Huffman::KMaxCodes
*/
void Huffman::HuffmanL(const uint32_t aFrequency[],int32_t aNumCodes,uint32_t aHuffman[])
{
	if(uint32_t(aNumCodes)>uint32_t(KMaxCodes))
		ReportError(HUFFMANTOOMANYCODESERROR);

	// Sort the values into decreasing order of frequency
	TNode* nodes = new TNode[aNumCodes];

	int32_t lCount=0;

	for (int32_t ii=0;ii<aNumCodes;++ii)
	{
		int32_t c=aFrequency[ii];
		if (c!=0)
			InsertInOrder(nodes,lCount++,c,ii|KLeaf);
	}

	// default code length is zero
	memset(aHuffman,0,aNumCodes*sizeof(uint32_t));

	if (lCount==0)
	{
		// no codes with frequency>0. No code has a length
	}
	else if (lCount==1)
	{
		// special case for a single value (always encode as "0")
		aHuffman[nodes[0].iRight&~KLeaf]=1;
	}
	else
	{
		// Huffman algorithm: pair off least frequent nodes and reorder
		do
		{
			--lCount;
			uint32_t c=nodes[lCount].iCount + nodes[lCount-1].iCount;
			nodes[lCount].iLeft=nodes[lCount-1].iRight;
			// re-order the leaves now to reflect new combined frequency 'c'
			InsertInOrder(nodes,lCount-1,c,lCount);
		} while (lCount>1);
		// generate code lengths in aHuffman[]
		HuffmanLengthsL(aHuffman,nodes,1,0);
	}

	delete [] nodes;

	if(!IsValid(aHuffman,aNumCodes))
		ReportError(HUFFMANINVALIDCODINGERROR);
}

/**
Validate a Huffman encoding

This verifies that a Huffman coding described by the code lengths is valid. In particular,
it ensures that no code exceeds the maximum length and that it is possible to generate a
canonical coding for the specified lengths.

@param "const uint32_t aHuffman[]" The table of code lengths as generated by Huffman::HuffmanL()
@param "int32_t aNumCodes" The number of codes in the table

@return True if the code is valid, otherwise false
*/
bool Huffman::IsValid(const uint32_t aHuffman[],int32_t aNumCodes)
{
	// The code is valid if one of the following holds:
	// (a) the code exactly fills the 'code space'
	// (b) there is only a single symbol with code length 1
	// (c) there are no encoded symbols
	//
	uint32_t remain=1<<KMaxCodeLength;
	int32_t totlen=0;
	for (const uint32_t* p=aHuffman+aNumCodes; p>aHuffman;)
	{
		int32_t len=*--p;
		if (len>0)
		{
			totlen+=len;
			if (len>KMaxCodeLength)
				return 0;

			uint32_t c=1<<(KMaxCodeLength-len);
			if (c>remain)
				return 0;

			remain-=c;
		}
	}

	return remain==0 || totlen<=1;
}

/**
Create a canonical Huffman encoding table

This generates the huffman codes used by TBitOutput::HuffmanL() to write huffman encoded data.
The input is table of code lengths, as generated by Huffman::HuffmanL() and must represent a
valid huffman code.

@param "const uint32_t aHuffman[]" The table of code lengths as generated by Huffman::HuffmanL()
@param "int32_t aNumCodes" The number of codes in the table
@param "uint32_t aEncodeTable[]" The table for the output huffman codes. This must be the same
size as the code-length table, and can safely be the same table.

@panic "USER ???" If the provided code is not a valid Huffman coding

@see IsValid()
@see HuffmanL()
*/
void Huffman::Encoding(const uint32_t aHuffman[],int32_t aNumCodes,uint32_t aEncodeTable[])
{
	if (!IsValid(aHuffman,aNumCodes))
		ReportError(HUFFMANINVALIDCODINGERROR);

	TFixedArray<int32_t,KMaxCodeLength> lenCount;
//	std::vector<int32_t> lenCount;
//	lenCount.reserve(KMaxCodeLength);
	lenCount.Reset();

	int32_t ii;
	for (ii=0;ii<aNumCodes;++ii)
	{
		int32_t len=aHuffman[ii]-1;
		if (len>=0)
			++lenCount[len];
	}

	TFixedArray<uint32_t,KMaxCodeLength> nextCode;
	uint32_t code=0;
	for (ii=0;ii<KMaxCodeLength;++ii)
	{
		code<<=1;
		nextCode[ii]=code;
		code+=lenCount[ii];
	}

	for (ii=0;ii<aNumCodes;++ii)
	{
		int32_t len=aHuffman[ii];
		if (len==0)
			aEncodeTable[ii]=0;
		else
		{
			aEncodeTable[ii] = (nextCode[len-1]<<(KMaxCodeLength-len))|(len<<KMaxCodeLength);
			++nextCode[len-1];
		}
	}
}

/**
The encoding table for the externalised code
@internalComponent
@released
*/
const uint32_t HuffmanEncoding[]=
{
	0x10000000,
	0x1c000000,
	0x12000000,
	0x1d000000,
	0x26000000,
	0x26800000,
	0x2f000000,
	0x37400000,
	0x37600000,
	0x37800000,
	0x3fa00000,
	0x3fb00000,
	0x3fc00000,
	0x3fd00000,
	0x47e00000,
	0x47e80000,
	0x47f00000,
	0x4ff80000,
	0x57fc0000,
	0x5ffe0000,
	0x67ff0000,
	0x77ff8000,
	0x7fffa000,
	0x7fffb000,
	0x7fffc000,
	0x7fffd000,
	0x7fffe000,
	0x87fff000,
	0x87fff800
};

/**
Function to encode 0a as '0' and 0b as '1', return number of symbols created
@internalComponent
@released
*/
void EncodeRunLengthL(TBitOutput& aOutput, int32_t aLength)
{
	if (aLength>0)
	{
		EncodeRunLengthL(aOutput,(aLength-1)>>1);
		aOutput.HuffmanL(HuffmanEncoding[1-(aLength&1)]);
	}
}

/**
Store a canonical huffman encoding in compact form

As the encoding is canonical, only the code lengths of each code needs to be saved.

Due to the nature of code length tables, these can usually be stored very compactly by
encoding the encoding itself, hence the use of the bit output stream.

@param "TBitOutput& aOutput" The output stream for the encoding
@param "const uint32_t aHuffman[]" The table of code lengths as generated by Huffman::HuffmanL()
@param "int32_t aNumCodes" The number of huffman codes in the table

@leave "TBitOutput::HuffmanL()"
*/
void Huffman::ExternalizeL(TBitOutput& aOutput,const uint32_t aHuffman[],int32_t aNumCodes)
{
	// We assume that the code length table is generated by the huffman generator,
	// in which case the maxmimum code length is 27 bits.
	//
	// We apply three transformations to the data:
	// 1. the data goes through a move-to-front coder
	// 2. apply a rle-0 coder which replace runs of '0' with streams of '0a' and '0b'
	// 3. encode the result using a predefined (average) huffman coding
	//
	// This can be done in a single pass over the data, avoiding the need for additional
	// memory.
	//
	// initialise the list for the MTF coder
	TFixedArray<uint8_t,Huffman::KMetaCodes> list;
	for (int32_t i=0;i<list.Count();++i)
		list[i]=uint8_t(i);
	int32_t last=0;

	int32_t rl=0;
	const uint32_t* p32=aHuffman;
	const uint32_t* e32=p32+aNumCodes;
	while (p32<e32)
	{
		int32_t c=*p32++;
		if (c==last)
			++rl;	// repeat of last symbol
		else
		{
			// encode run-length
			EncodeRunLengthL(aOutput,rl);
			rl=0;
			// find code in MTF list
			int32_t j;
			for (j=1;list[j]!=c;++j)
				;
			// store this code
			aOutput.HuffmanL(HuffmanEncoding[j+1]);
			// adjust list for MTF algorithm
			while (--j>0)
				list[j+1]=list[j];
			list[1]=uint8_t(last);
			last=c;
		}
	}
	// encod any remaining run-length
	EncodeRunLengthL(aOutput,rl);
}

const int32_t KHuffTerminate=0x0001;
const uint32_t KBranch1=sizeof(uint32_t)<<16;

/**
Function to write the subtree below aPtr and return the head
*/
uint32_t* HuffmanSubTree(uint32_t* aPtr,const uint32_t* aValue,uint32_t** aLevel)
{
	uint32_t* left=*aLevel++;
	if (left > aValue)
	{
		uint32_t* sub0=HuffmanSubTree(aPtr,aValue,aLevel);		// 0-tree first
		aPtr=HuffmanSubTree(sub0,aValue-(aPtr-sub0)-1,aLevel);	// 1-tree
		int32_t branch0=(uint8_t*)sub0-(uint8_t*)(aPtr-1);
		*--aPtr=KBranch1|branch0;
	}
	else if (left == aValue)
	{
		uint32_t term0=*aValue--;						// 0-term
		aPtr=HuffmanSubTree(aPtr,aValue,aLevel);	// 1-tree
		*--aPtr=KBranch1|(term0>>16);
	}
	else	// l<iNext
	{
		uint32_t term0=*aValue--;						// 0-term
		uint32_t term1=*aValue--;
		*--aPtr=(term1>>16<<16)|(term0>>16);
	}
	return aPtr;
}

/**
Create a canonical Huffman decoding tree

This generates the huffman decoding tree used by TBitInput::HuffmanL() to read huffman
encoded data. The input is table of code lengths, as generated by Huffman::HuffmanL()
and must represent a valid huffman code.

@param "const uint32_t aHuffman[]" The table of code lengths as generated by Huffman::HuffmanL()
@param "int32_t aNumCodes" The number of codes in the table
@param "uint32_t aDecodeTree[]" The space for the decoding tree. This must be the same
size as the code-length table, and can safely be the same memory
@param "int32_t aSymbolBase" the base value for the output 'symbols' from the decoding tree, by default
this is zero.

@panic "USER ???" If the provided code is not a valid Huffman coding

@see IsValid()
@see HuffmanL()
*/
void Huffman::Decoding(const uint32_t aHuffman[],int32_t aNumCodes,uint32_t aDecodeTree[],int32_t aSymbolBase)
{
	if(!IsValid(aHuffman,aNumCodes))
		ReportError(HUFFMANINVALIDCODINGERROR);

	TFixedArray<int32_t,KMaxCodeLength> counts;
	counts.Reset();
	int32_t codes=0;
	int32_t ii;
	for (ii=0;ii<aNumCodes;++ii)
	{
		int32_t len=aHuffman[ii];
		aDecodeTree[ii]=len;
		if (--len>=0)
		{
			++counts[len];
			++codes;
		}
	}

	TFixedArray<uint32_t*,KMaxCodeLength> level;
	uint32_t* lit=aDecodeTree+codes;
	for (ii=0;ii<KMaxCodeLength;++ii)
	{
		level[ii]=lit;
		lit-=counts[ii];
	}

	aSymbolBase=(aSymbolBase<<17)+(KHuffTerminate<<16);
	for (ii=0;ii<aNumCodes;++ii)
	{
		uint32_t len=uint8_t(aDecodeTree[ii]);
		if (len)
			*--level[len-1]|=(ii<<17)+aSymbolBase;
	}

	if (codes==1)	// codes==1 special case: tree isn't complete
	{
		uint32_t term=aDecodeTree[0]>>16;
		aDecodeTree[0]=term|(term<<16); // 0- and 1-terminate at root
	}
	else if (codes>1)
		HuffmanSubTree(aDecodeTree+codes-1,aDecodeTree+codes-1,&level[0]);
}

/**
The decoding tree for the externalised code
*/
const uint32_t HuffmanDecoding[]=
{
	0x0004006c,
	0x00040064,
	0x0004005c,
	0x00040050,
	0x00040044,
	0x0004003c,
	0x00040034,
	0x00040021,
	0x00040023,
	0x00040025,
	0x00040027,
	0x00040029,
	0x00040014,
	0x0004000c,
	0x00040035,
	0x00390037,
	0x00330031,
	0x0004002b,
	0x002f002d,
	0x001f001d,
	0x001b0019,
	0x00040013,
	0x00170015,
	0x0004000d,
	0x0011000f,
	0x000b0009,
	0x00070003,
	0x00050001
};


/**
Restore a canonical huffman encoding from a bit stream

The encoding must have been stored using Huffman::ExternalizeL(). The resulting
code-length table can be used to create an encoding table using Huffman::Encoding()
or a decoding tree using Huffman::Decoding().

@param "TBitInput& aInput" The input stream with the encoding
@param "uint32_t aHuffman[]" The internalized code-length table is placed here
@param "int32_t aNumCodes" The number of huffman codes in the table

@leave "TBitInput::HuffmanL()"

@see ExternalizeL()
See ExternalizeL for a description of the format
*/
void Huffman::InternalizeL(TBitInput& aInput,uint32_t aHuffman[],int32_t aNumCodes)
{
	// initialise move-to-front list
	TFixedArray<uint8_t,Huffman::KMetaCodes> list;
	for (int32_t i=0;i<list.Count();++i)
		list[i]=uint8_t(i);

	int32_t last=0;
	// extract codes, reverse rle-0 and mtf encoding in one pass
	uint32_t* p=aHuffman;
	const uint32_t* end=aHuffman+aNumCodes;
	int32_t rl=0;
	while (p+rl<end)
	{
		int32_t c=aInput.HuffmanL(HuffmanDecoding);
		if (c<2)
		{
			// one of the zero codes used by RLE-0
			// update he run-length
			rl+=rl+c+1;
		}
		else
		{
			while (rl>0)
			{
				if (p>end)
				{
					ReportError(HUFFMANINVALIDCODINGERROR);
				}
				*p++=last;
				--rl;
			}
			--c;
			list[0]=uint8_t(last);
			last=list[c];

			memmove((void * const)&list[1],(const void * const)&list[0],(size_t)c);
			if (p>end)
			{
				ReportError(HUFFMANINVALIDCODINGERROR);
			}
			*p++=last;
		}
	}
	while (rl>0)
	{
		if (p>end)
		{
			ReportError(HUFFMANINVALIDCODINGERROR);
		}
		*p++=last;
		--rl;
	}
}

/**
bit-stream input class
Reverse the byte-order of a 32 bit value
This generates optimal ARM code (4 instructions)
*/
inline uint32_t reverse(uint32_t aVal)
{
	uint32_t v=(aVal<<16)|(aVal>>16);
	v^=aVal;
	v&=0xff00ffff;
	aVal=(aVal>>8)|(aVal<<24);
	return aVal^(v>>8);
}

/**
Construct a bit stream input object

Following construction the bit stream is ready for reading bits, but will
immediately call UnderflowL() as the input buffer is empty.
*/
TBitInput::TBitInput()
{
	Set(nullptr, 0, 0);
}

/**
Construct a bit stream input object over a buffer

Following construction the bit stream is ready for reading bits from the specified buffer.

@param "const uint8_t* aPtr" The address of the buffer containing the bit stream
@param "int32_t aLength" The length of the bitstream in bits
@param "int32_t aOffset" The bit offset from the start of the buffer to the bit stream (defaults to zero)
*/
TBitInput::TBitInput(const uint8_t* aPtr, int32_t aLength, int32_t aOffset)
{
	Set(aPtr,aLength,aOffset);
}

/**
Set the memory buffer to use for input.

Bits will be read from this buffer until it is empty, at which point UnderflowL() will be called.

@param "const uint8_t* aPtr" The address of the buffer containing the bit stream
@param "int32_t aLength" The length of the bitstream in bits
@param "int32_t aOffset" The bit offset from the start of the buffer to the bit stream (defaults to zero)
*/
void TBitInput::Set(const uint8_t* aPtr, int32_t aLength, int32_t aOffset)
{
	size_t p=(size_t)aPtr;
	p+=aOffset>>3;			// nearest byte to the specified bit offset
	aOffset&=7;				// bit offset within the byte
	const uint32_t* ptr=(const uint32_t*)(p&~3);	// word containing this byte
	aOffset+=(p&3)<<3;		// bit offset within the word
	if (aLength==0)
		iCount=0;
	else
	{
		// read the first few bits of the stream
		iBits=reverse(*ptr++)<<aOffset;
		aOffset=32-aOffset;
		aLength-=aOffset;
		if (aLength<0)
			aOffset+=aLength;
		iCount=aOffset;
	}
	iRemain=aLength;
	iPtr=ptr;
}

#ifndef __HUFFMAN_MACHINE_CODED__

/**
Read a single bit from the input

Return the next bit in the input stream. This will call UnderflowL() if there are no more
bits available.

@return The next bit in the stream

@leave "UnderflowL()" It the bit stream is exhausted more UnderflowL is called to get more
data
*/
uint32_t TBitInput::ReadL()
{
	int32_t c=iCount;
	uint32_t bits=iBits;
	if (--c<0)
		return ReadL(1);
	iCount=c;
	iBits=bits<<1;
	return bits>>31;
}

/**
Read a multi-bit value from the input

Return the next few bits as an unsigned integer. The last bit read is the least significant
bit of the returned value, and the value is zero extended to return a 32-bit result.

A read of zero bits will always reaturn zero.

This will call UnderflowL() if there are not enough bits available.

@param "int32_t aSize" The number of bits to read

@return The bits read from the stream

@leave "UnderflowL()" It the bit stream is exhausted more UnderflowL is called to get more
data
*/
uint32_t TBitInput::ReadL(int32_t aSize)
{
	if (!aSize)
		return 0;
	uint32_t val=0;
	uint32_t bits=iBits;
	iCount-=aSize;
	while (iCount<0)
	{
		// need more bits
#ifdef __CPU_X86
		// X86 does not allow shift-by-32
		if (iCount+aSize!=0)
			val|=bits>>(32-(iCount+aSize))<<(-iCount);	// scrub low order bits
#else
		val|=bits>>(32-(iCount+aSize))<<(-iCount);	// scrub low order bits
#endif
		aSize=-iCount;	// bits still required
		if (iRemain>0)
		{
			bits=reverse(*iPtr++);
			iCount+=32;
			iRemain-=32;
			if (iRemain<0)
				iCount+=iRemain;
		}
		else
		{
			UnderflowL();
			bits=iBits;
			iCount-=aSize;
		}
	}

#ifdef __CPU_X86
	// X86 does not allow shift-by-32
	iBits=aSize==32?0:bits<<aSize;
#else
	iBits=bits<<aSize;
#endif

	return val|(bits>>(32-aSize));
}

/**
Read and decode a Huffman Code

Interpret the next bits in the input as a Huffman code in the specified decoding.
The decoding tree should be the output from Huffman::Decoding().

@param "const uint32_t* aTree" The huffman decoding tree

@return The symbol that was decoded

@leave "UnderflowL()" It the bit stream is exhausted more UnderflowL is called to get more
data
*/
uint32_t TBitInput::HuffmanL(const uint32_t* aTree)
{
	uint32_t huff=0;
	do
	{
		aTree=(const uint32_t*)(((uint8_t*)aTree)+(huff>>16));
		huff=*aTree;
		if (ReadL()==0)
			huff<<=16;
	} while ((huff&0x10000u)==0);

	return huff>>17;
}

#endif

/**
Handle an empty input buffer

This virtual function is called when the input buffer is empty and more bits are required.
It should reset the input buffer with more data using Set().

A derived class can replace this to read the data from a file (for example) before reseting
the input buffer.

@leave "KErrUnderflow" The default implementation leaves
*/
void TBitInput::UnderflowL()
{
	ReportError(HUFFMANBUFFEROVERFLOWERROR);
}

TBitInput::~TBitInput()
{
	if(!iPtr) delete iPtr;
}
