///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormatType.h"
#include "String.h"
#include "Memory.h"

namespace chustd {

//////////////////////////////////////////////////////////
void FormatInt32(int32 value, wchar szBuffer[12])
{
	szBuffer[0] = '-';
	szBuffer[11] = 0;

	int32 i = 10;
	wchar* psz = szBuffer;
	int32 negative = value >> 31;
	value = (value ^ negative) + (negative & 1);

	do
	{
		int32 remain = value % 10;
		value /= 10;

		psz[i] = wchar( '0' + remain );

		--i;
	}
	while(value != 0);


	// Shift
	// Start at 1 to keep the '-' (negative number) or start at 0 (positive number) to blow up the '-'
	int32 offset = i + negative + 1;
	int count = 11 - i;

	psz = szBuffer + (negative & 1);
	do
	{
		psz[0] = psz[offset];
		--count;
		++psz;
	}
	while( count != 0);
}

//////////////////////////////////////////////////////////
void FormatUInt32(uint32 value, wchar szBuffer[12])
{
	szBuffer[11] = 0;

	uint32 i = 10;
	wchar* psz = szBuffer;

	do
	{
		uint32 remain = value % 10;
		value /= 10;

		psz[i] = char( '0' + remain );

		--i;
	}
	while(value != 0);


	// Shift
	int32 offset = i + 1;
	int count = 11 - i;

	psz = szBuffer;
	do
	{
		psz[0] = psz[offset];
		--count;
		++psz;
	}
	while( count != 0);
}

//////////////////////////////////////////////////////////
void FormatInt64(int64 value, wchar szBuffer[24])
{
	// MAX = 21 counting the final 0
	szBuffer[0] = '-';
	szBuffer[20] = 0;

	int32 i = 19;
	wchar* psz = szBuffer;
	int64 negative = value >> 63;
	value = (value ^ negative) + (negative & 1);

	do
	{
		int32 remain = int32(value % 10);
		value /= 10;

		psz[i] = wchar( '0' + remain );

		--i;
	}
	while(value != 0);


	// Shift
	// Start at 1 to keep the '-' (negative number) or start at 0 (positive number) to blow up the '-'
	int32 offset = i + int32(negative) + 1;
	int count = 21 - i;

	psz = szBuffer + (negative & 1);
	do
	{
		psz[0] = psz[offset];
		--count;
		++psz;
	}
	while( count != 0);
}

//////////////////////////////////////////////////////////
void FormatUInt64(uint64 value, wchar szBuffer[24])
{
	// MAX = 21 counting the final 0
	szBuffer[20] = 0;

	int32 i = 19;
	wchar* psz = szBuffer;

	do
	{
		int32 remain = int32(value % 10);
		value /= 10;

		psz[i] = wchar( '0' + remain );

		--i;
	}
	while(value != 0);


	// Shift
	// Start at 1 to keep the '-' (negative number) or start at 0 (positive number) to blow up the '-'
	int32 offset = i + 1;
	int count = 21 - i;

	psz = szBuffer;
	do
	{
		psz[0] = psz[offset];
		--count;
		++psz;
	}
	while( count != 0);
}

static void MoveAndFill(wchar* pszBuffer, int8 width, wchar cFill)
{
	int32 length = String::SZLength(pszBuffer);
	int32 moveCount = width - length;
	if( moveCount < 0 )
		return;

	Memory::Move16(pszBuffer + moveCount, pszBuffer, length);
	Memory::Set16(pszBuffer, cFill, moveCount);
	pszBuffer[moveCount + length] = 0;
}

void FormatInt32(int32 value, wchar* pszBuffer, int8 width, wchar cFill)
{
	FormatInt32(value, pszBuffer);
	MoveAndFill(pszBuffer, width, cFill);
}

void FormatInt64(int64 value, wchar* pszBuffer, int8 width, wchar cFill)
{
	FormatInt64(value, pszBuffer);
	MoveAndFill(pszBuffer, width, cFill);
}

void FormatUInt32(uint32 value, wchar* pszBuffer, int8 width, wchar cFill)
{
	FormatUInt32(value, pszBuffer);
	MoveAndFill(pszBuffer, width, cFill);
}

void FormatUInt64(uint64 value, wchar* pszBuffer, int8 width, wchar cFill)
{
	FormatUInt64(value, pszBuffer);
	MoveAndFill(pszBuffer, width, cFill);
}

void FormatPtr(void* p, wchar* pszBuffer)
{
	const int sizeofPtr = sizeof(p);
	const int32 digitCount = (sizeofPtr * 2);

	int64 ptr = int64(p);
	for(int i = digitCount - 1; i >= 0; --i)
	{
		uint32 digit = uint32(ptr & 0x0f);
		wchar c = 0;
		if( digit <= 9 )
		{
			c = wchar(digit + '0');
		}
		else
		{
			c = wchar((digit - 10) + 'a');
		}
		pszBuffer[i] = c;
		ptr >>= 4;
	}
	pszBuffer[digitCount] = 0;
}

////////////////////////////////////
void FormatUIntNHex(uint64 value, wchar* pszBuffer, int8 digitCount, bool uppercase)
{
	// Count heading zeros
	int i = 0;
	for(; i < digitCount; ++i)
	{
		uint64 rem = (value >> (4 * digitCount - 4));
		if( rem != 0 )
		{
			break;
		}
		value <<= 4;
	}
	uint32 cpA = uppercase ? 'A' : 'a';
	for(; i < digitCount; ++i)
	{
		uint64 rem = (value >> (4 * digitCount - 4));
		uint32 digit = uint32(rem) & 0x0000000f;
		if( digit <= 9 )
		{
			digit += '0';
		}
		else
		{
			digit = (digit - 10) + cpA;
		}
		pszBuffer[0] = uint8(digit);
		pszBuffer++;
		value <<= 4;
	}
	pszBuffer[0] = 0;
}

void FormatUInt8Hex(uint8 value, wchar* pszBuffer, bool uppercase)
{
	FormatUIntNHex(value, pszBuffer, 2, uppercase);
}

void FormatUInt16Hex(uint16 value, wchar* pszBuffer, bool uppercase)
{
	FormatUIntNHex(value, pszBuffer, 4, uppercase);
}

void FormatUInt32Hex(uint32 value, wchar* pszBuffer, bool uppercase)
{
	FormatUIntNHex(value, pszBuffer, 8, uppercase);
}

void FormatUInt32Hex(uint32 value, wchar* pszBuffer, int8 width, wchar cFill, bool uppercase)
{
	FormatUInt32Hex(value, pszBuffer, uppercase);
	MoveAndFill(pszBuffer, width, cFill);
}

void FormatUInt64Hex(uint64 value, wchar* pszBuffer, bool uppercase)
{
	FormatUIntNHex(value, pszBuffer, 16, uppercase);
}

void FormatUInt64Hex(uint64 value, wchar* pszBuffer, int8 width, wchar cFill, bool uppercase)
{
	FormatUInt64Hex(value, pszBuffer, uppercase);
	MoveAndFill(pszBuffer, width, cFill);
}

////////////////////////////////////
void FormatUIntNBin(uint64 value, wchar* pszBuffer, int8 digitCount)
{
	int i = 0;
	for(; i < digitCount; ++i)
	{
		uint32 digit = (value >> (digitCount - 1)) & 0x00000001;
		if( digit != 0 )
		{
			break;
		}
		value <<= 1;
	}
	for(; i < digitCount; ++i)
	{
		uint32 digit = (value >> (digitCount - 1)) & 0x00000001;
		digit += '0';
		pszBuffer[0] = uint8(digit);
		pszBuffer++;
		value <<= 1;
	}
	pszBuffer[0] = 0;
}

void FormatUInt8Bin(uint8 value, wchar* pszBuffer)
{
	FormatUIntNBin(value, pszBuffer, 8);
}

void FormatUInt16Bin(uint16 value, wchar* pszBuffer)
{
	FormatUIntNBin(value, pszBuffer, 16);
}

void FormatUInt32Bin(uint32 value, wchar* pszBuffer)
{
	FormatUIntNBin(value, pszBuffer, 32);
}

void FormatUInt32Bin(uint32 value, wchar* pszBuffer, int8 width, wchar cFill)
{
	FormatUInt32Bin(value, pszBuffer);
	MoveAndFill(pszBuffer, width, cFill);
}

void FormatUInt64Bin(uint64 value, wchar* pszBuffer)
{
	FormatUIntNBin(value, pszBuffer, 64);
}

void FormatUInt64Bin(uint64 value, wchar* pszBuffer, int8 width, wchar cFill)
{
	FormatUInt64Bin(value, pszBuffer);
	MoveAndFill(pszBuffer, width, cFill);
}

} // namespace chustd
