///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringData.h"

using namespace chustd;

/////////////////////////////////////////////////////////////////////////////////////
uint16* StringData::CreateInstance(int32 wantedLength)
{
	ASSERT(wantedLength >= 0);

	///////////////////////////////////////////////////////
	// Number of uint64 to allocate
	// 1 is the two int32 (length and ref)
	const int32 total = 1 + GetAllocatedChunkCount(wantedLength);

	uint64* pAlloc = (uint64*) ::malloc(total * 8);
	ASSERT(pAlloc); // Raise an exception ?
	
	StringData* pInst = (StringData*) pAlloc;
	pInst->m_ref = 1;
	pInst->m_length = wantedLength; // Character count (0 not included)
	pInst->m_szBuffer[wantedLength] = 0; // Puts a final 0 after the last used char
	///////////////////////////////////////////////////////

	return pInst->m_szBuffer;
}

// New instance copy of StringData pointed by pData
uint16* StringData::CreateInstance(StringData* pData)
{
	const int32 wantedLength = pData->GetLength();

	ASSERT(wantedLength >= 0);

	///////////////////////////////////////////////////////
	// Number of uint64 to allocate
	// 1 is the two int32 (length and ref)
	const int32 totalAlloc = 1 + GetAllocatedChunkCount(wantedLength);

	uint64* pAlloc = (uint64*) ::malloc(totalAlloc * 8);
	ASSERT(pAlloc); // Raise an exception ?

	const uint64* const pSrc = (uint64*) pData;

	const int32 totalUsed = 1 + GetUsedChunkCount(wantedLength);
	Memory::Copy64(pAlloc, pSrc, totalUsed);
		
	StringData* pInst = (StringData*) pAlloc;
	pInst->m_ref = 1;

	return pInst->m_szBuffer;
}
//////////////////////////////////////////////////////////////////////
