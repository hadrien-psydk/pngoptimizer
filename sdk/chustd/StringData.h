///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_STRINGDATA_H
#define CHUSTD_STRINGDATA_H

// Private header

#include "Memory.h"
#include "Math.h"

namespace chustd {

class StringData
{
public:
	static uint16* CreateInstance(int32 wantedLength);
	static uint16* CreateInstance(StringData* pData);

	uint16* GetBuffer() { return m_szBuffer; }
	const uint16* GetBuffer() const { return m_szBuffer; }
	int32 GetLength() const { return m_length; }
	void AddRef() { ASSERT(m_ref != MAX_INT32); m_ref++;}
	void Release() { ASSERT(m_ref != MAX_INT32); m_ref--;}
	int32 GetRef() const { return m_ref; }

	void SetLength(int32 length) { ASSERT(m_ref != MAX_INT32); m_length = length; }
	void Delete() {	uint64* pThis = (uint64*) this;	::free(pThis);	}

	// Returns the number of chunks allocated for the char buffer
	// Each chunk is an uint64
	static inline int32 GetAllocatedChunkCount(int32 length)
	{
		// Counting the ending 0
		const int32 sizeInBytes = (length + 1) * sizeof(uint16);
		
		// Number of needed blocks. Each block is 64 bytes.
		const int32 blockCount = chustd::Math::DivCeil(sizeInBytes, 64);
		
		// Number of needed chunks. Each chunk is 8 bytes.
		const int32 chunkCount = blockCount * (64 / 8);
		
		return chunkCount;
	}

	static inline int32 GetAllocatedCharCount(int32 length)
	{
		return GetAllocatedChunkCount(length) * 8;
	}

	static inline int32 GetUsedChunkCount(int32 length)
	{
		int32 sizeInBytes = (length + 1) * sizeof(uint16);
		return Memory::ByteCountToInt64Count(sizeInBytes);
	}

	static inline StringData* GetInstance(uint16* psz)
	{
		ASSERT(psz != nullptr);
		uint64* pBuf64 = (uint64*) psz;
		pBuf64 -= 1;
		
		StringData* pInstance = (StringData*) pBuf64;
		
		ASSERT(pInstance->m_ref >= 0);
		ASSERT(pInstance->m_length >= 0);

		return pInstance;
	}

	static StringData* GetNullInstance()
	{
		static int32 nullData[] = { MAX_INT32, 0, 0 };
		return (StringData*) nullData;
	}

public:
	// Only for ms_nullData. See StringTemplate.h
	StringData(int32) : m_ref(MAX_INT32), m_length(0) { m_szBuffer[0] = 0; }
private:

	StringData() {}

private:
	int32 m_ref;        // Reference counter for this StringData
	int32 m_length;     // Character count (0 not included)
	uint16 m_szBuffer[1]; // C string, always ends with a 0
};


} // namespace chustd

#endif // ndef CHUSTD_STRINGDATA_H
