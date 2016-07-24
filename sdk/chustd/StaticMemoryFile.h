///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_STATICMEMORYFILE_H
#define CHUSTD_STATICMEMORYFILE_H

#include "IFile.h"
#include "Buffer.h"

namespace chustd {

// Acts like a file, but in memory. Works with an external memory buffer.
class StaticMemoryFile : public IFile
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

	bool OpenRead(const void* buf, int size);
	bool OpenWrite(void* buf, int size);
	
	// Writes to this StaticMemoryFile from content found in an external file
	int WriteFromFile(IFile& fileSrc, int size);

	StaticMemoryFile();

private:
	void*     m_buf;
	int       m_bufSize;
	int       m_position;
	int       m_maxPosition; // Also the size of the file
	ByteOrder m_byteOrder;
};

} // namespace chustd

#endif // ndef CHUSTD_DYNAMICMEMORYFILE_H
