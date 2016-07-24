///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Memory.h"
#include "Array.h"

using namespace chustd;

/////////////////////////////////////////////////////////////////////////////
void Memory::Move(void* pDst, const void* pSrc, int count)
{
	if( pDst < pSrc )
	{
		Copy(pDst, pSrc, count);
	}
	else
	{
		// My poor cache is gonna be killed here
		uint8* pDst8 = (uint8*) pDst;
		uint8* pSrc8 = (uint8*) pSrc;

		for(int i = count - 1; i >= 0; --i)
		{
			pDst8[i] = pSrc8[i];
		}
	}
}

void Memory::Move16(void* pDst, const void* pSrc, int count)
{
	ASSERT( Is16BitsAligned(pDst));
	ASSERT( Is16BitsAligned(pSrc));

	if( pDst < pSrc )
	{
		Copy16(pDst, pSrc, count);
	}
	else
	{
		// My poor cache is gonna be killed here
		uint16* pDst16 = (uint16*) pDst;
		uint16* pSrc16 = (uint16*) pSrc;

		for(int i = count - 1; i >= 0; --i)
		{
			pDst16[i] = pSrc16[i];
		}
	}
}

void Memory::Copy(void* pDst, const void* pSrc, int32 byteCount)
{
	if( Is32BitsAligned(pDst) && Is32BitsAligned(pSrc) && (byteCount & 0x03) == 0 )
	{
		Copy32(pDst, pSrc, byteCount / 4);
		return;
	}

	uint8* pDst8 = (uint8*) pDst;
	uint8* pSrc8 = (uint8*) pSrc;

	for(int i = 0; i < byteCount; ++i)
	{
		pDst8[i] = pSrc8[i];
	}
}

void Memory::Copy16(void* pDst, const void* pSrc, int count)
{
	ASSERT( Is16BitsAligned(pDst));
	ASSERT( Is16BitsAligned(pSrc));

	uint16* pDst16 = (uint16*) pDst;
	uint16* pSrc16 = (uint16*) pSrc;

	for(int i = 0; i < count; ++i)
	{
		pDst16[i] = pSrc16[i];
	}
}

void Memory::Copy32(void* pDst, const void* pSrc, int count)
{
	ASSERT( Is32BitsAligned(pDst));
	ASSERT( Is32BitsAligned(pSrc));

	uint32* pDst32 = (uint32*) pDst;
	uint32* pSrc32 = (uint32*) pSrc;

	for(int i = 0; i < count; ++i)
	{
		pDst32[i] = pSrc32[i];
	}
}

void Memory::Copy64(void* pDst, const void* pSrc, int count)
{
	ASSERT( Is64BitsAligned(pDst));
	ASSERT( Is64BitsAligned(pSrc));

	uint64* pDst64 = (uint64*) pDst;
	uint64* pSrc64 = (uint64*) pSrc;

	for(int i = 0; i < count; ++i)
	{
		pDst64[i] = pSrc64[i];
	}
}

void Memory::Copy64From8(void* pDst, const void* pSrc, int32 byteCount)
{
	ASSERT( Is64BitsAligned(pDst));
	
	uint8* pDst8 = (uint8*) pDst;
	uint8* pSrc8 = (uint8*) pSrc;

	int32 i = 0;
	for(; i < byteCount - 7; i += 8)
	{
		pDst8[i] = pSrc8[i];
		pDst8[i + 1] = pSrc8[i + 1];
		pDst8[i + 2] = pSrc8[i + 2];
		pDst8[i + 3] = pSrc8[i + 3];
		pDst8[i + 4] = pSrc8[i + 4];
		pDst8[i + 5] = pSrc8[i + 5];
		pDst8[i + 6] = pSrc8[i + 6];
		pDst8[i + 7] = pSrc8[i + 7];
	}

	for(; i < byteCount; ++i)
	{
		pDst8[i] = pSrc8[i];
	}
}

void Memory::Copy64From16(void* pDst, const void* pSrc, int32 nUInt16Count)
{
	ASSERT( Is64BitsAligned(pDst));
	ASSERT( Is16BitsAligned(pSrc));
	
	uint16* pDst16 = (uint16*) pDst;
	uint16* pSrc16 = (uint16*) pSrc;

	int32 i = 0;
	for(; i < nUInt16Count - 3; i += 4)
	{
		pDst16[i] = pSrc16[i];
		pDst16[i + 1] = pSrc16[i + 1];
		pDst16[i + 2] = pSrc16[i + 2];
		pDst16[i + 3] = pSrc16[i + 3];
	}

	for(; i < nUInt16Count; ++i)
	{
		pDst16[i] = pSrc16[i];
	}
}

void Memory::Swap(void* pDst, void* pSrc, int count)
{
	uint8* pDst8 = (uint8*) pDst;
	uint8* pSrc8 = (uint8*) pSrc;

	for(int i = 0; i < count; ++i)
	{
		uint8 tmp = pDst8[i];
		pDst8[i] = pSrc8[i];
		pSrc8[i] = tmp;
	}
}

void Memory::Zero(void* pDst, int32 byteCount)
{
	if( Is16BitsAligned(pDst) && (byteCount & 0x0001) == 0 )
	{
		Zero16(pDst, byteCount / 2);
		return;
	}

	uint8* pDst8 = (uint8*) pDst;

	int32 lastByte = byteCount & 0x01;
	byteCount -= lastByte;

	for(int i = 0; i < byteCount; i += 2)
	{
		pDst8[0] = 0;
		pDst8[1] = 0;
		pDst8 += 2;
	}

	if( lastByte == 0 )
		return;
	
	pDst8[0] = 0;
}

void Memory::Zero16(void* pDst, int count)
{
	ASSERT( Is16BitsAligned(pDst));

	uint16* pDst16 = (uint16*) pDst;

	for(int i = 0; i < count; ++i)
	{
		pDst16[i] = 0;
	}
}

void Memory::Zero32(void* pDst, int count)
{
	ASSERT( Is32BitsAligned(pDst));

	uint32* pDst32 = (uint32*) pDst;

	for(int i = 0; i < count; ++i)
	{
		pDst32[i] = 0;
	}
}

void Memory::Set(void* pDst, uint8 value, int32 byteCount)
{
	if( Is16BitsAligned(pDst) && (byteCount & 0x0001) == 0 )
	{
		uint16 value16 = MAKE16(value, value);
		Set16(pDst, value16, byteCount / 2);
		return;
	}

	uint8* pDst8 = (uint8*) pDst;

	int32 lastByte = byteCount & 0x01;
	byteCount -= lastByte;
	for(int i = 0; i < byteCount; i += 2)
	{
		pDst8[0] = value;
		pDst8[1] = value;
		pDst8 += 2;
	}

	if( lastByte == 0 )
		return;
	
	pDst8[0] = value;
}

void Memory::Set16(void* pDst, uint16 value, int count)
{
	ASSERT( Is16BitsAligned(pDst));

	uint16* pDst16 = (uint16*) pDst;

	for(int i = 0; i < count; ++i)
	{
		pDst16[i] = value;
	}
}

void Memory::Set32(void* pDst, uint32 value, int count)
{
	ASSERT( Is32BitsAligned(pDst));

	uint32* pDst32 = (uint32*) pDst;

	for(int i = 0; i < count; ++i)
	{
		pDst32[i] = value;
	}
}

bool Memory::Equals(const void* pSrc0, const void* pSrc1, int32 byteCount)
{
	uint8* pSrc08 = (uint8*) pSrc0;
	uint8* pSrc18 = (uint8*) pSrc1;

	for(int i = 0; i < byteCount; ++i)
	{
		if( pSrc08[i] != pSrc18[i] )
			return false;
	}
	return true;
}

bool Memory::Equals32(const void* pSrc0, const void* pSrc1, int count)
{
	ASSERT( Is32BitsAligned(pSrc0));
	ASSERT( Is32BitsAligned(pSrc1));

	uint32* pSrc032 = (uint32*) pSrc0;
	uint32* pSrc132 = (uint32*) pSrc1;

	for(int i = 0; i < count; ++i)
	{
		if( pSrc032[i] != pSrc132[i] )
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void* Memory::Alloc(int size)
{
	return ::malloc(size);
}

///////////////////////////////////////////////////////////////////////////////
void  Memory::Free(void* p)
{
	::free(p);
}

///////////////////////////////////////////////////////////////////////////////
// Gets the size of a buffer allocated with Alloc(). The size returned can be
// greater than the argument of Alloc.
///////////////////////////////////////////////////////////////////////////////
int Memory::GetSize(void* p)
{
#if defined(_MSC_VER)
	return static_cast<int>(_msize(p));
#elif defined(__linux__)
	return malloc_usable_size(p);
#endif
}

