///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StaticMemoryFile.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////////////////////////
StaticMemoryFile::StaticMemoryFile()
{
	m_buf = nullptr;
	m_bufSize = 0;
	m_position = 0;
	m_maxPosition = 0;
	m_byteOrder = boBigEndian;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool StaticMemoryFile::SetPosition(int64 offset, Whence whence)
{
	int64 newPos = 0;
	if( whence == IFile::posBegin )
	{
		newPos = offset;
	}
	else if( whence == IFile::posEnd )
	{
		newPos = m_position - offset;
	}
	else if( whence == IFile::posCurrent )
	{
		newPos = m_position + offset;
	}

	if( newPos < 0 )
	{
		return false;
	}
	if( newPos > MAX_INT32 )
	{
		return false;
	}

	if( newPos > m_maxPosition )
	{
		return false;
	}
	m_position = int32(newPos);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int64 StaticMemoryFile::GetPosition() const
{
	return m_position;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int64 StaticMemoryFile::GetSize()
{
	return m_maxPosition;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int StaticMemoryFile::Read(void* pBuffer, int size)
{
	int remaining = m_bufSize - m_position;
	if( size > remaining )
	{
		size = remaining;
	}

	if( size > 0 )
	{
		const uint8* pContent = (const uint8*)m_buf;
		Memory::Copy(pBuffer, pContent + m_position, size);
		m_position += size;
	}
	return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int StaticMemoryFile::Write(const void* pBuffer, int size)
{
	int newPos = m_position + size;
	if( newPos > m_bufSize )
	{
		size = m_bufSize - m_position;
		if( size <= 0 )
		{
			return size;
		}
	}

	uint8* pDst = (uint8*)m_buf + m_position;
	Memory::Copy(pDst, pBuffer, size);
	m_position += size;
	if( m_position > m_maxPosition )
	{
		m_maxPosition = m_position;
	}
	return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ByteOrder StaticMemoryFile::GetByteOrder() const
{
	return m_byteOrder;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void StaticMemoryFile::SetByteOrder(ByteOrder byteOrder)
{
	m_byteOrder = byteOrder;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void StaticMemoryFile::Close()
{
	m_buf = nullptr;
	m_bufSize = 0;
	m_position = 0;
	m_maxPosition = 0;
	m_byteOrder = boBigEndian;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Opens the StaticMemoryFile for reading from an external read-only buffer
bool StaticMemoryFile::OpenRead(const void* buf, int size)
{
	if( buf == nullptr || size < 0 )
	{
		return false;
	}
	Close();
	m_buf = const_cast<void*>(buf);
	m_bufSize = size;
	m_position = 0;
	m_maxPosition = size;
	m_byteOrder = boBigEndian;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Opens the StaticMemoryFile for reading from an external write buffer
bool StaticMemoryFile::OpenWrite(void* buf, int size)
{
	if( buf == nullptr || size < 0 )
	{
		return false;
	}
	Close();
	m_buf = buf;
	m_bufSize = size;
	m_position = 0;
	m_maxPosition = 0;
	m_byteOrder = boBigEndian;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Writes to this StaticMemoryFile using another file as the source
//
// [in] fileSrc   File to read from
// |in] size      Amount of bytes to read from fileSrc
//
// Returns the amound read. Equals to size upon success.
int StaticMemoryFile::WriteFromFile(IFile& fileSrc, int size)
{
	if( size < 0 )
	{
		return -1;
	}
	int newPos = m_position + size;
	if( newPos > m_bufSize )
	{
		return -1;
	}

	uint8* pDst = (uint8*)m_buf + m_position;
	int32 read = fileSrc.Read(pDst, size);
	m_position += read;
	if( m_position > m_maxPosition )
	{
		m_maxPosition = m_position;
	}
	return read;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace chustd
