///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////
#ifndef CHUSTD_BUFFER_H
#define CHUSTD_BUFFER_H

#include "stdafx.h"
#include "Memory.h"
///////////////////////////////////////////////////////////////

namespace chustd {

// Manipulates a shared buffer of bytes
class Buffer
{
public:
	int GetSize() const;
	bool SetSize(int size);

	bool EnsureCapacity(int capacity);

	const uint8* GetReadPtr() const;
	uint8* GetWritePtr();

	Buffer();
	Buffer(const Buffer&);
	~Buffer();

	Buffer& operator=(const Buffer&);

	bool IsEmpty() const { return GetSize() == 0; }
	void Clear() { SetSize(0); }

	void Fill(uint8 byte);
	void Assign(const uint8* pBytes, int size);

private:
	uint8* m_pBytes;
};

} // namespace chustd

#endif
