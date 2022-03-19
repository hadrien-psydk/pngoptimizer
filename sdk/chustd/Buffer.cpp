///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Buffer.h"

#include "Atomic.h"

using namespace chustd;

// Reference counted struct for the Buffer class
struct BufferData
{
	int32 refCount;
	int32 size;      // Size of the data below
	uint8  bytes[8];

	void Ref()
	{
		Atomic::Increment(&refCount);
	}

	void Unref()
	{
		int32 result = Atomic::Decrement(&refCount);
		if( result == 0 )
		{
			Memory::Free(this);
		}
	}

	static uint8* Alloc(int dataSize)
	{
		int allocSize = GetAllocSize(dataSize);
		BufferData* pData = (BufferData*)Memory::Alloc(allocSize);
		if( pData == nullptr )
		{
			return nullptr;
		}
		pData->refCount = 1;
		pData->size = dataSize;
		return pData->bytes;
	}

	static uint8* Realloc(BufferData* pData, int dataSize)
	{
		int allocSize = GetAllocSize(dataSize);
		size_t capacity = Memory::GetSize(pData);
		if( size_t(allocSize) < capacity )
		{
			// No realloc needed
			pData->size = dataSize;
			return pData->bytes;
		}
		BufferData* pData2 = (BufferData*)realloc(pData, allocSize);
		if( pData2 == nullptr )
		{
			return nullptr;
		}
		pData2->size = dataSize;
		return pData2->bytes;
	}

	static BufferData* GetPtr(uint8* pBytes)
	{
		return (BufferData*)(pBytes - 8);
	}
private:
	static int GetAllocSize(int dataSize)
	{
		return (sizeof(BufferData) - 8) + ROUND64(dataSize);
	}
};

///////////////////////////////////////////////////////////////////////////////
Buffer::Buffer()
{
	m_pBytes = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
Buffer::Buffer(const Buffer& buf)
{
	uint8* pBytes = buf.m_pBytes;
	if( pBytes )
	{
		BufferData::GetPtr(pBytes)->Ref();
		m_pBytes = pBytes;
	}
	else
	{
		m_pBytes = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////
Buffer::~Buffer()
{
	if( m_pBytes )
	{
		BufferData::GetPtr(m_pBytes)->Unref();
		m_pBytes = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////
Buffer& Buffer::operator=(const Buffer& buf)
{
	uint8* pBytes = buf.m_pBytes;
	if( pBytes )
	{
		BufferData::GetPtr(pBytes)->Ref();
		if( m_pBytes )
		{
			BufferData::GetPtr(m_pBytes)->Unref();
		}
		m_pBytes = pBytes;
	}
	else
	{
		m_pBytes = nullptr;
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the new size of the buffer.
// Returns true upon success.
bool Buffer::SetSize(int size)
{
	if( size < 0 )
	{
		ASSERT(0);
		return false;
	}
	if( m_pBytes == nullptr )
	{
		if( size > 0 )
		{
			m_pBytes = BufferData::Alloc(size);
		}
	}
	else
	{
		BufferData* pData = BufferData::GetPtr(m_pBytes);
		if( pData->refCount == 1 )
		{
			m_pBytes = BufferData::Realloc(pData, size);
		}
		else
		{
			// Have to create our own buffer, unless the new size is 0
			if( size == 0 )
			{
				m_pBytes = nullptr;
			}
			else
			{
				uint8* pBytes2 = BufferData::Alloc(size);
				if( pBytes2 )
				{
					int32 oldSize = pData->size;
					int32 minSize = MIN(oldSize, size);
					memcpy(pBytes2, pData->bytes, minSize);
				}
				m_pBytes = pBytes2;
			}
			pData->Unref();
		}
	}
	
	if( size > 0 )
	{
		return m_pBytes != nullptr;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Allocates memory without changing the size.
// Returns true upon success.
bool Buffer::EnsureCapacity(int capacity)
{
	if( capacity < 0 )
	{
		return false;
	}
	if( capacity == 0 )
	{
		return true;
	}
	int oldSize = 0;
	if( m_pBytes )
	{
		oldSize = BufferData::GetPtr(m_pBytes)->size;
	};
	bool ok = SetSize(capacity);
	if( ok )
	{
		BufferData* pData = BufferData::GetPtr(m_pBytes);
		pData->size = oldSize;
	}
	return ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int Buffer::GetSize() const
{
	if( m_pBytes )
	{
		BufferData* pData = BufferData::GetPtr(m_pBytes);
		return pData->size;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const uint8* Buffer::GetReadPtr() const
{
	return m_pBytes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
uint8* Buffer::GetWritePtr()
{
	if( m_pBytes == nullptr )
	{
		return nullptr;
	}
	BufferData* pData = BufferData::GetPtr(m_pBytes);
	if( pData->refCount == 1 )
	{
		return m_pBytes;
	}
	// Force own buffer
	if( !SetSize(pData->size) )
	{
		return nullptr;
	}
	return m_pBytes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fills the buffer with the same byte.
void Buffer::Fill(uint8 byte)
{
	int size = GetSize();
	if( size )
	{
		uint8* pDst = GetWritePtr();
		memset(pDst, byte, size);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Copies external bytes and sets this buffer with.
void Buffer::Assign(const uint8* pBytes, int size)
{
	if( SetSize(size) )
	{
		uint8* pDst = GetWritePtr();
		memcpy(pDst, pBytes, size);
	}
}


