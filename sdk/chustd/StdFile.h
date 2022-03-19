///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_STDFILE_H
#define CHUSTD_STDFILE_H

#include "IFile.h"
#include "Console.h"

namespace chustd {

// Acts like a file, but for stdin, stdout or stderr
class StdFile : public IFile
{
public:
	///////////////////////////////////////////////////////////////////////
	virtual bool  SetPosition(int64 offset, Whence whence = posBegin);
	virtual int64 GetPosition() const;
	virtual int64 GetSize();
	virtual int   Read(void* pBuffer, int size);
	virtual int   Write(const void* pBuffer, int size);
	virtual ByteOrder GetByteOrder() const;
	virtual void SetByteOrder(ByteOrder byteOrder);
	virtual void Close();
	///////////////////////////////////////////////////////////////////////

	StdFile(StdFileType);

private:
	union Impl
	{
		void* handle;
		int fd;
		bool IsValid() const { return handle != (void*)-1; }
	} m_impl;
	ByteOrder   m_byteOrder;
};

} // namespace chustd

#endif
