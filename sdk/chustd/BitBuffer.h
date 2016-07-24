///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_BITBUFFER_H
#define CHUSTD_BITBUFFER_H

namespace chustd {

class BitBuffer
{
public:
	BitBuffer();

	// Initial function to call bedore others
	void SetBuffer(uint8* pBuffer, int32 byteSize);
	void ClearBuffer();

	void ClearBit(int32 bitPosition);
	void SetBit(int32 bitPosition);
	
	// Clear or set a consecutive range of bits
	void ClearBitRange(int32 bitPosition, int32 range);
	void SetBitRange(int32 bitPosition, int32 range);

	// Returns -1 if not found
	int32 FindFirstZero();

	// Find a consecutive range of bits with value 0
	// Returns -1 if not found
	int32 FindZeroRange(int32 range);

private:
	uint8* m_pBuffer; // TODO: Could be optimized later with uint32*
	int32 m_nSize;
};

} // namespace chustd

#endif // ndef CHUSTD_BITBUFFER_H
