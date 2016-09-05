///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StdFile.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////////////////////////
StdFile::StdFile(StdFileType type)
{
#if defined(_WIN32)
	if( type == StdFileType::Stdin )
	{
		m_impl.handle = GetStdHandle(STD_INPUT_HANDLE);
	}
	else if( type == StdFileType::Stdout )
	{
		m_impl.handle = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	else if( type == StdFileType::Stderr )
	{
		m_impl.handle = GetStdHandle(STD_ERROR_HANDLE);
	}
	else
	{
		m_impl.handle = INVALID_HANDLE_VALUE;
	}
#elif defined(__linux__)
	if( type == StdFileType::Stdin )
	{
		m_impl.fd = STDIN_FILENO;
	}
	else if( type == StdFileType::Stdout )
	{
		m_impl.fd = STDOUT_FILENO;
	}
	else if( type == StdFileType::Stderr )
	{
		m_impl.fd = STDERR_FILENO;
	}
	else
	{
		m_impl.fd = -1;
	}
#endif
	m_byteOrder = boBigEndian;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool StdFile::SetPosition(int64, Whence)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int64 StdFile::GetPosition() const
{
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int64 StdFile::GetSize()
{
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int StdFile::Read(void* pBuffer, int size)
{
	if( !m_impl.IsValid() )
		return -1;

	if( size < 0 )
		return -2;

	if( size == 0 )
		return 0; // Not an error

#if defined(_WIN32)
	DWORD read = 0;
	BOOL ret = ReadFile(m_impl.handle, pBuffer, size, &read, nullptr);
	if( !ret )
	{
		return -3;
	}
#elif defined(__linux__)
	int read = ::read(m_impl.fd, pBuffer, size);
	//Console::Write("StdFile::Read" + String::FromInt(size) + " returns " + String::FromInt(read));
#endif
	return int(read);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
int StdFile::Write(const void* pBuffer, int size)
{
	if( !m_impl.IsValid() )
		return -1;

	if( size < 0 )
		return -2;

	if( size == 0 )
		return 0; // Not an error

#if defined(_WIN32)
	DWORD written = 0;
	BOOL ret = WriteFile(m_impl.handle, pBuffer, size, &written, nullptr);
	if( !ret )
	{
		return -3;
	}

#elif defined(__linux__)
	int written = ::write(m_impl.fd, pBuffer, size);
#endif
	return int(written);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ByteOrder StdFile::GetByteOrder() const
{
	return m_byteOrder;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void StdFile::SetByteOrder(ByteOrder byteOrder)
{
	m_byteOrder = byteOrder;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void StdFile::Close()
{
	m_byteOrder = boBigEndian;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace chustd
