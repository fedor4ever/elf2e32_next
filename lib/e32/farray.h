// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2024 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors: Strizhniou Fiodar - fix build and runtime errors.
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// @internalComponent
// @released
//
//


#ifndef __FARRAY_H__
#define __FARRAY_H__

#include <cstdint>
#include <string.h>
#ifdef NDEBUG
#undef NDEBUG
#endif //NDEBUG
#include <cassert>

template <class T, int32_t S>
class TFixedArray
// Range checking wrapper+ class for C++ arrays
// Can be embedded in C-objects, or used on the stack: use Reset() to zero it
	{
	typedef TFixedArray<T,S> ThisClass;
public:
	inline TFixedArray();
	inline TFixedArray(const T* aList, int32_t aLength);
	//
	inline void Copy(const T* aList, int32_t aLength);
	inline void Reset();		// zero fill
	inline void DeleteAll();
	//
	inline int32_t Count() const;
	inline int32_t Length() const;
	// Accessors - debug range checking
	inline T& operator[](int32_t aIndex);
	inline const T& operator[] (int32_t aIndex) const;
	// Accessors - always range checking
	inline T& At(int32_t aIndex);
	inline const T& At(int32_t aIndex) const;
	// Provides pointers to the beginning and end of the array
	inline T* Begin();
	inline T* End();
	inline const T* Begin() const;
	inline const T* End() const;
	//
protected:
	inline static bool InRange(int32_t aIndex);
protected:
	T iRep[S];
	};


template <class T,int32_t S>
inline TFixedArray<T,S>::TFixedArray()
	{}
template <class T,int32_t S>
inline void TFixedArray<T,S>::Copy(const T* aList,int32_t aLength)
	{
	// Never used.
    assert(uint32_t(aLength)<=uint32_t(S));
    // HMdem::Copy(iRep,aList,aLength*sizeof(T));
    }
template <class T,int32_t S>
inline TFixedArray<T,S>::TFixedArray(const T* aList,int32_t aLength)
	{Copy(aList,aLength);}
template <class T,int32_t S>
inline void TFixedArray<T,S>::Reset()
	{memset(iRep,0,sizeof(iRep));}
template <class T,int32_t S>
inline int32_t TFixedArray<T,S>::Count() const
	{return S;}
template <class T,int32_t S>
inline int32_t TFixedArray<T,S>::Length() const
	{return sizeof(T);}
template <class T,int32_t S>
inline bool TFixedArray<T,S>::InRange(int32_t aIndex)
	{return uint32_t(aIndex)<S;}
template <class T,int32_t S>
inline T& TFixedArray<T,S>::operator[](int32_t aIndex)
	{assert(InRange(aIndex));return iRep[aIndex];}
template <class T,int32_t S>
inline const T& TFixedArray<T,S>::operator[](int32_t aIndex) const
	{return const_cast<ThisClass&>(*this)[aIndex];}
template <class T,int32_t S>
inline T& TFixedArray<T,S>::At(int32_t aIndex)
	{assert(InRange(aIndex));return iRep[aIndex];}
template <class T,int32_t S>
inline const T& TFixedArray<T,S>::At(int32_t aIndex) const
	{return const_cast<ThisClass&>(*this).At(aIndex);}
template <class T,int32_t S>
inline T* TFixedArray<T,S>::Begin()
	{return &iRep[0];}
template <class T,int32_t S>
inline T* TFixedArray<T,S>::End()
	{return &iRep[S];}
template <class T,int32_t S>
inline const T* TFixedArray<T,S>::Begin() const
	{return &iRep[0];}
template <class T,int32_t S>
inline const T* TFixedArray<T,S>::End() const
	{return &iRep[S];}


#endif

