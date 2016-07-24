///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_MEMORY_H
#define CHUSTD_MEMORY_H

namespace chustd {

class Memory
{
public:
	static void Move(void* pDst, const void* pSrc, int count);
	static void Move16(void* pDst, const void* pSrc, int count);
	
	static void Copy(void* pDst, const void* pSrc, int32 byteCount);
	static void Copy16(void* pDst, const void* pSrc, int count);
	static void Copy32(void* pDst, const void* pSrc, int count);
	static void Copy64(void* pDst, const void* pSrc, int count);
	static void Copy64From8(void* pDst, const void* pSrc, int32 byteCount);
	static void Copy64From16(void* pDst, const void* pSrc, int32 nUInt16Count);
	
	static void Swap(void* pDst, void* pSrc, int count);

	static void Zero(void* pDst, int32 byteCount);
	static void Zero16(void* pDst, int count);
	static void Zero32(void* pDst, int count);
	
	static void Set(void* pDst, uint8 value, int32 byteCount);
	static void Set16(void* pDst, uint16 value, int count);
	static void Set32(void* pDst, uint32 value, int count);
	
	static bool Equals(const void* pSrc0, const void* pSrc1, int32 byteCount);
	static bool Equals32(const void* pSrc0, const void* pSrc1, int count);

	static inline int32 ByteCountToInt64Count(int32 sizeInBytes)
	{
		const int32 remainBit0 = (sizeInBytes >> 0) & 1;
		const int32 remainBit1 = (sizeInBytes >> 1) & 1;
		const int32 remainBit2 = (sizeInBytes >> 2) & 1;
		const int32 chunkCount = (sizeInBytes >> 3) + (remainBit2 | remainBit1 | remainBit0);

		return chunkCount;
	}

	static inline bool Is16BitsAligned(const void* p);
	static inline bool Is32BitsAligned(const void* p);
	static inline bool Is64BitsAligned(const void* p);

	static void* Alloc(int size);
	static void  Free(void* p);
	static int   GetSize(void* p);
};

bool Memory::Is16BitsAligned(const void* p)
{
	uint8* pNull = null;
	uint8* p8 = (uint8*) p;
	uint32 diff = uint32(p8 - pNull);
	return (diff & 0x0001) == 0;
}

bool Memory::Is32BitsAligned(const void* p)
{
	uint8* pNull = null;
	uint8* p8 = (uint8*) p;
	uint32 diff = uint32(p8 - pNull);
	return (diff & 0x0003) == 0;
}

bool Memory::Is64BitsAligned(const void* p)
{
	uint8* pNull = null;
	uint8* p8 = (uint8*) p;
	uint32 diff = uint32(p8 - pNull);
	return (diff & 0x0007) == 0;
}

template <int32 t_nCount>
class AlignedStackBuffer64Bits
{
public:
	uint64* GetBuffer()
	{
		uint8* pBuf8 = (uint8*) m_aBuffer;
		int lowPart = pBuf8 - (uint8*)(0);
		pBuf8 += ((8 - lowPart) & 0x07);
		return (uint64*)pBuf8;
	}
	int32 Sizeof() const { return t_nCount * 8; }
private:
	uint64 m_aBuffer[ t_nCount + 8];
};

} // namespace chustd

#endif // ndef CHUSTD_MEMORY_H
