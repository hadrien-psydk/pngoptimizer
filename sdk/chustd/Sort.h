///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_SORT_H
#define CHUSTD_SORT_H

#include "Memory.h"

namespace chustd {

class StringArray;

namespace Sort {

void ByteSortAtt(uint32* paElements, uint8* paSortAttributes, int size);
void ByteSortAtt(uint32* paElements, uint32* paSortAttributes, int size);

// in/out :  paElements : the elements to sort
//          paSortAttributs : the numbers associated to each element which
//          are to be used in the sort
void ByteSortAtt(uint32* paElements, uint32* paSortAttributes, 
	uint32* paTmpElements, uint32* paTmpSortAttributs, int size);

void ByteSort(const uint8* pIn, uint8* pOut, int size);
void ByteSort(uint32* pIn, uint32* pOut, int size);

void ByteSortLittleEndian(const uint32* pIn, uint32* pOut1, uint32* pOut2, int size);

void BubbleSort(uint32* pa, int size);

inline uint8 Byte( uint32 n, int shift )
{
	return uint8( n >> (shift << 3) );
}

template <class T>
void ByteSort(T* paElementsIn, T* paElementsOut, uint8* paSortAttributes, int size)
{
	uint32 aOccurrences[256];
	
	Memory::Zero32(aOccurrences, 256);
		
	for( int32 i = 0; i < size; i++ )
	{
		const uint8 byte = paSortAttributes[i];
		aOccurrences[byte]++;
	}

	// Compute final positions in the sorted array
	uint32 position = 0;
	for( int32 i = 0; i < 256; i++ )
	{			
		const uint32 increment = aOccurrences[i];
		aOccurrences[i] = position;
		position += increment;
	}

	for( int32 i = 0; i < size; i++ )
	{			
		const uint8 byte = paSortAttributes[i];
		position = aOccurrences[byte];
		aOccurrences[byte]++;

		paElementsOut[position] = paElementsIn[i];
	}
}

StringArray SortStrings(const StringArray& in);

} // namespace Sort
} // namespace chustd

#endif // ndef CHUSTD_SORT_H
