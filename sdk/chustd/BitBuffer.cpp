///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BitBuffer.h"

using namespace chustd;
//////////////////////////////////////////////////////////////////////
BitBuffer::BitBuffer()
{
	m_pBuffer = null;
	m_nSize = 0;
}

void BitBuffer::SetBuffer(uint8* pBuffer, int32 byteSize)
{
	m_pBuffer = pBuffer;
	m_nSize = byteSize;
}

void BitBuffer::ClearBuffer()
{
	for(int i = 0; i < m_nSize; ++i)
	{
		m_pBuffer[i] = 0;
	}
}

void BitBuffer::ClearBit(int32 bitPosition)
{
	const int32 bytePos = bitPosition / 8;
	int32 bitPosInByte = bitPosition % 8;

	uint8 value = 0x80;
	value >>= (bitPosInByte);
	m_pBuffer[bytePos] &= ~value;
}

void BitBuffer::SetBit(int32 bitPosition)
{
	const int32 bytePos = bitPosition / 8;
	int32 bitPosInByte = bitPosition % 8;

	uint8 value = 0x80;
	value >>= (bitPosInByte);
	m_pBuffer[bytePos] |= value;
}

void BitBuffer::ClearBitRange(int32 bitPosition, int32 range)
{
	// TODO: optimize
	for(int i = 0; i < range; ++i)
	{
		ClearBit(bitPosition + i);
	}
}

void BitBuffer::SetBitRange(int32 bitPosition, int32 range)
{
	// TODO: optimize
	for(int i = 0; i < range; ++i)
	{
		SetBit(bitPosition + i);
	}
}


int32 BitBuffer::FindFirstZero()
{
	uint8* pBuffer = m_pBuffer;
	const int size = m_nSize;
	
	for(int i = 0; i < size; ++i)
	{
		uint8 byte = pBuffer[i];
		if( byte != 0xff )
		{
			for(int iBit = 0; iBit < 8; ++iBit)
			{
				uint8 mask = uint8(0x80 >> iBit); // 1000_0000, 0100_0000, 0010_0000...
				if( (byte & mask) == 0 )
				{
					// Found
					return i * 8 + iBit;
				}
			}
		}
	}
	return -1;
}

int32 BitBuffer::FindZeroRange(int32 range)
{
	uint8* pBuffer = m_pBuffer;
	const int size = m_nSize;

	int32 seqStart = 0;
	int32 seqLength = 0;

	for(int i = 0; i < size; ++i)
	{
		uint8 byte = pBuffer[i];
		if( byte != 0xff )
		{
			for(int iBit = 0; iBit < 8; ++iBit)
			{
				uint8 mask = uint8(0x80 >> iBit); // 1000_0000, 0100_0000, 0010_0000...
				if( (byte & mask) == 0 )
				{
					if( seqLength == 0 )
					{
						// Start of sequence
						seqStart = i * 8 + iBit;
					}
					seqLength++;

					if( seqLength == range )
					{
						// Found ^^
						return seqStart;
					}
				}
				else
				{
					// End of sequence
					seqLength = 0;
				}
			}
		}
	}
	return -1;
}

