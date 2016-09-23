///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DynamicMemoryFile.h"

///////////////////////////////////////////////////////////////////////////////
using namespace chustd;
///////////////////////////////////////////////////////////////////////////////

DynamicMemoryFile::DynamicMemoryFile()
{
	m_byteOrder = boBigEndian;
	m_position = 0;
}

bool DynamicMemoryFile::SetPosition(int64 offset, Whence whence)
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

	if( newPos > m_content.GetSize() )
	{
		return false;
	}
	m_position = int32(newPos);
	return true;
}

int64 DynamicMemoryFile::GetPosition() const
{
	return m_position;
}

int64 DynamicMemoryFile::GetSize()
{
	return m_content.GetSize();
}

int DynamicMemoryFile::Read(void* pBuffer, int size)
{
	int remaining = m_content.GetSize() - m_position;
	if( size > remaining )
	{
		size = remaining;
	}

	if( size > 0 )
	{
		const uint8* pContent = m_content.GetReadPtr();
		Memory::Copy(pBuffer, pContent + m_position, size);
		m_position += size;
	}
	return size;
}

int DynamicMemoryFile::Write(const void* pBuffer, int size)
{
	int newSize = m_position + size;
	if( newSize > m_content.GetSize() )
	{
		// Realloc
		if( !m_content.SetSize(newSize) )
		{
			return -1;
		}
	}

	uint8* pDst = m_content.GetWritePtr() + m_position;
	Memory::Copy(pDst, pBuffer, size);
	m_position += size;
	return size;
}

ByteOrder DynamicMemoryFile::GetByteOrder() const
{
	return m_byteOrder;
}

void DynamicMemoryFile::SetByteOrder(ByteOrder byteOrder)
{
	m_byteOrder = byteOrder;
}

void DynamicMemoryFile::Close()
{
	m_content.Clear();
	m_byteOrder = boBigEndian;
	m_position = 0;
}

bool DynamicMemoryFile::Open(int32 initialCapacity)
{
	if( initialCapacity < 0 )
	{
		return false;
	}
	m_position = 0;
	return m_content.EnsureCapacity(initialCapacity);
}

bool DynamicMemoryFile::EnsureCapacity(int32 capacity)
{
	return m_content.EnsureCapacity(capacity);
}

const Buffer& DynamicMemoryFile::GetContent()
{
	return m_content;
}

///////////////////////////////////////////////////////////////////////////////
// Writes to this DynamicMemoryFile using another file as the source
//
// [in,out] fileSrc     Source file
// [in]     size        Number of bytes to read from fileSrc. If -1, the
//                      source file is read until EOF is found
// [in]     readAmount  If size is -1, the number of bytes to read in a loop
//                      until EOF is found. If -1, a default value is used.
//
// [ret] number of bytes read, negative upon error
///////////////////////////////////////////////////////////////////////////////
int DynamicMemoryFile::WriteFromFile(IFile& fileSrc, int size, int readAmount)
{
	if( size < 0 )
	{
		if( size != -1 )
		{
			return -1;
		}
		if( readAmount <= 0 )
		{
			// readAmount is not provided, use a default arbitrary value
			readAmount = 1024*1024;
		}
	}
	else
	{
		int newSize = m_position + size;
		if( newSize > m_content.GetSize() )
		{
			if( !m_content.SetSize(newSize) )
			{
				return -1;
			}
		}
		readAmount = size;
	}

	int64 totalRead = 0;
	for(;;)
	{
		int newSize = m_position + readAmount;
		if( newSize > m_content.GetSize() )
		{
			// Realloc
			if( !m_content.SetSize(newSize) )
			{
				return -1;
			}
		}

		uint8* pDst = m_content.GetWritePtr() + m_position;
		int32 read = fileSrc.Read(pDst, readAmount);
		if( read > 0 )
		{
			m_position += read;
			totalRead += read;

			if( totalRead == size )
			{
				// Requested size reached
				break;
			}
		}
		else if( read == 0 )
		{
			// EOF found
			break;
		}
		else
		{
			// Error
			return -1;
		}
	}
	// Update size in case we read less
	m_content.SetSize(m_position);

	return static_cast<int>(totalRead);
}
