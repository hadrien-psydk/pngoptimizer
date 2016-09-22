///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_DYNAMICMEMORYFILE_H
#define CHUSTD_DYNAMICMEMORYFILE_H

#include "IFile.h"
#include "Buffer.h"

namespace chustd {

// Acts like a file, but in memory. The content automatically grows if needed.
class DynamicMemoryFile : public IFile
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

	// Opens the memory file. Default flags are :
	// ReadWrite mode + Big Endian mode
	bool Open(int32 initialCapacity);
	
	bool EnsureCapacity(int32 capacity);

	// Gets the file content
	const Buffer& GetContent();

	// Writes to this DynamicMemoryFile from content found in an external file
	int WriteFromFile(IFile& fileSrc, int size, int readAmount = -1);

	DynamicMemoryFile();

private:
	Buffer    m_content;
	ByteOrder m_byteOrder;
	int32     m_position;
};

} // namespace chustd

#endif // ndef CHUSTD_DYNAMICMEMORYFILE_H
