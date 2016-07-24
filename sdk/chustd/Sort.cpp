///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Sort.h"
#include "Memory.h"
#include "Math.h"
#include "String.h"
#include "Array.h"

using namespace chustd;


void Sort::ByteSortAtt(uint32* paElements, uint32* paSortAttributs, int size)
{
	uint32* paTmpElements = new uint32[size];
	uint32* paTmpSortAttributs = new uint32[size];

	ByteSortAtt(paElements, paSortAttributs, paTmpElements, paTmpSortAttributs, size);

	delete[] paTmpSortAttributs;
	delete[] paTmpElements;
}

void Sort::ByteSortAtt(uint32* paElements, uint32* paSortAttributs, 
		uint32* paNewElements, uint32* paNewSortAttributs, int size)
{
	uint32 aOccurrences[256];
	
	// 4 passes for each byte of the uint32
	for( int8 shift = 0 ; shift < 4 ; shift++ )
	{
		Memory::Zero32(aOccurrences, 256);
			
		for( int32 i = 0; i < size; i++ )
		{
			const uint8 byte = Byte( paSortAttributs[i], shift );
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
			const uint8 byte = Byte( paSortAttributs[i], shift );
			position = aOccurrences[byte];
			aOccurrences[byte]++;

			paNewSortAttributs[position] = paSortAttributs[i];
			paNewElements[position] = paElements[i];
		}

		// Swap array pointers
		// Final array is valid because we have an even number of passes
		uint32* pTmp = paSortAttributs;
		paSortAttributs = paNewSortAttributs;
		paNewSortAttributs = pTmp;

		uint32* pTmpElements = paElements;
		paElements = paNewElements;
		paNewElements = pTmpElements;
	}
}

void Sort::BubbleSort(uint32* pa, int size)
{
	for( int32 i = 0; i < size; ++i )
	{
		bool bHasSwapped = false;

		for( int32 j = 0; j < size - 1 - i; ++j )
		{
			if ( pa[j] > pa[j + 1] )
			{
				bHasSwapped = true;
				
				// Swap
				const uint32 tmp = pa[j + 1];
				pa[j + 1] = pa[j];
				pa[j] = tmp;
			}
			
			if ( !bHasSwapped )
				return;
		}
	}
}

void Sort::ByteSortAtt(uint32* paElements, uint8* paSortAttributs, int size)
{
	uint32* paTmpElements = new uint32[size];

	uint32 aOccurrences[256];
	
	Memory::Zero32(aOccurrences, 256);
		
	for( int32 i = 0; i < size; i++ )
	{
		const uint8 byte = paSortAttributs[i];
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
		const uint8 byte = paSortAttributs[i];
		position = aOccurrences[byte];
		aOccurrences[byte]++;

		paTmpElements[position] = paElements[i];
	}

	Memory::Copy32(paElements, paTmpElements, size);

	delete[] paTmpElements;
}

void Sort::ByteSort(const uint8* pIn, uint8* pOut, int size)
{
	uint32 aOccurrences[256];
	
	// Step 1 : clear the occurrence array
	Memory::Zero32(aOccurrences, 256);
	
	// Step 2 : count the occurrence of each element
	for(int i = 0; i < size; ++i)
	{
		const uint8 byte = pIn[i];
		aOccurrences[byte]++;
	}

	// Step 3 : compute final positions in the sorted array
	uint32 position = 0;
	for(int i = 0; i < 256; ++i)
	{			
		const uint32 increment = aOccurrences[i];
		aOccurrences[i] = position;
		position += increment;
	}
	
	for(int i = 0; i < size; ++i)
	{			
		const uint8 byte = pIn[i];
		position = aOccurrences[byte];
		aOccurrences[byte]++;

		pOut[position] = pIn[i];
	}
}

void Sort::ByteSort(uint32* pIn, uint32* pOut, int size)
{
	uint32 aOccurrences[256];
	
	// 4 passes for each byte of the uint32
	for( int8 shift = 0 ; shift < 4 ; shift++ )
	{
		Memory::Zero32(aOccurrences, 256);
			
		for( int32 i = 0; i < size; i++ )
		{
			const uint8 byte = Byte( pIn[i], shift );
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
			const uint8 byte = Byte( pIn[i], shift );
			position = aOccurrences[byte];
			aOccurrences[byte]++;

			pOut[position] = pIn[i];
		}

		// Swap array pointers
		// Final array is valid because we have an even number of passes
		Math::Swap(pIn, pOut);
	}
}

// The uint32 are stored and managed in Little Endian, even on big endian machines
// Final result in pOut1
// pIn and pOut2 can point on the same data if pIn data can be overwritten
void Sort::ByteSortLittleEndian(const uint32* pIn, uint32* pOut1, uint32* pOut2, int size)
{
	uint32 aOccurrences[256];
	
	uint32* pOut = pOut1;

	// 4 passes for each byte of the uint32
	for(int iPass = 0; iPass < 4; ++iPass)
	{
		Memory::Zero32(aOccurrences, 256);

		int shift;
		if( k_ePlatformByteOrder == boBigEndian )
		{
			// Shift in reverse order
			shift = 4 - iPass;
		}
		else
		{
			shift = iPass;
		}
			
		for( int32 i = 0; i < size; i++ )
		{
			const uint8 byte = Byte( pIn[i], shift );
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
			const uint8 byte = Byte( pIn[i], shift );
			position = aOccurrences[byte];
			aOccurrences[byte]++;

			pOut[position] = pIn[i];
		}

		// Swap pointers for next byte
		if( (iPass & 0x01) == 0 )
		{
			pIn = pOut1;
			pOut = pOut2;
		}
		else
		{
			pIn = pOut2;
			pOut = pOut1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
static int CompareStrings(const void* p1, const void* p2)
{
	const String* pString1 = reinterpret_cast<const String*>(p1);
	const String* pString2 = reinterpret_cast<const String*>(p2);
	return pString1->CompareBin(*pString2);
}

///////////////////////////////////////////////////////////////////////////////
// Sorts an array of strings.
///////////////////////////////////////////////////////////////////////////////
StringArray Sort::SortStrings(const StringArray& in)
{
	int size = in.GetSize();
	StringArray out(in);
	String* rawOut = out.GetPtr();
	qsort(rawOut, size, sizeof(String), CompareStrings);
	return out;
}

