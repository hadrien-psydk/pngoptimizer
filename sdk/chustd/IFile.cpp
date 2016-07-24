///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IFile.h"

#include "Array.h"
#include "TextEncoding.h"

namespace chustd {\

//////////////////////////////////////////////////////////////////////
IFile::IFile()
{

}

IFile::~IFile()
{

}

//////////////////////////////////////////////////////////////////////
uint16 IFile::Swap16(uint16 value)
{
	union UConvert
	{
		uint16 n;
		struct { uint8 n0, n1; } bytes;
	};

	UConvert conv = { value };
	uint16 n2 = uint16( (conv.bytes.n0 << 8) | conv.bytes.n1);

	return n2;
}

uint32 IFile::Swap32(uint32 value)
{
	union UConvert
	{
		uint32 n;
		struct { uint8 n0, n1, n2, n3; } bytes;
	};

	UConvert conv = { value };
	uint32 n2 = (conv.bytes.n0 << 24) | (conv.bytes.n1 << 16)
		| (conv.bytes.n2 << 8) | conv.bytes.n3;

	return n2;
}

uint64 IFile::Swap64(uint64 value)
{
	union UConvert
	{
		uint64 n;
		struct { uint32 n0, n1; } words32;
	};

	UConvert conv = { value };
	uint32 n0 = conv.words32.n0;
	uint32 n1 = conv.words32.n1;
	conv.words32.n0 = Swap32(n1);
	conv.words32.n1 = Swap32(n0);
	return conv.n;
}

float32 IFile::SwapFloat32(float32 fValue)
{
	union UConvert
	{
		float32 f;
		struct { uint8 nHi, midHi, midLo, nLo; } bytes;
		uint32 n;
	};

	UConvert conv = { fValue };
	conv.n = (conv.bytes.nHi << 24) | (conv.bytes.midHi << 16)
		| (conv.bytes.midLo << 8) | conv.bytes.nLo;

	return conv.f;
}

float64 IFile::SwapFloat64(float64 fValue)
{
	union UConvert
	{
		float64 f;
		struct { uint32 n0, n1; } words32;
		uint64 n;
	};

	UConvert conv = { fValue };
	uint32 n0 = conv.words32.n0;
	uint32 n1 = conv.words32.n1;
	conv.words32.n0 = Swap32(n1);
	conv.words32.n1 = Swap32(n0);
	return conv.f;
}

void IFile::Swap16(uint16* paValues, int count)
{
	uint8* paBytes = (uint8*) paValues;
	const int32 byteCount = count * 2;

	for(int i = 0; i < byteCount; i += 2)
	{
		uint8 byteA = paBytes[i];
		uint8 byteB = paBytes[i + 1];
		
		paBytes[i + 1] = byteA;
		paBytes[i] = byteB;
	}
}

void IFile::Swap32(uint32* paValues, int count)
{
	for(int i = 0; i < count; ++i)
	{
		paValues[i] = Swap32(paValues[i]);
	}
}

//////////////////////////////////////
bool IFile::ShouldSwapBytes() const
{
	ByteOrder eFileByteOrder = GetByteOrder();

	if( eFileByteOrder != chustd::k_ePlatformByteOrder )
	{
		// We are trying to store little endian data on a big endian machine
		// Or
		// We are trying to store big endian data on a little endian machine
		// We should swap bytes !
		return true;
	}

	// No need to swap

	// We are trying to store little endian data on a little endian machine
	// Or
	// We are trying to store big endian data on a big endian machine

	return false;
}

//////////////////////////////////////
bool IFile::Read8(uint8& value)
{
	return Read(&value, 1) == 1;
}

bool IFile::Read16(uint16& value)
{
	bool bRet = Read(&value, 2) == 2;
	if( ShouldSwapBytes() )
	{
		value = Swap16(value);
	}
	return bRet;
}

bool IFile::Read32(uint32& value)
{
	bool bRet = Read(&value, 4) == 4;
	if( ShouldSwapBytes() )
	{
		value = Swap32(value);
	}
	return bRet;
}

bool IFile::Read64(uint64& value)
{
	bool bRet = Read(&value, 8) == 8;
	if( ShouldSwapBytes() )
	{
		value = Swap64(value);
	}
	return bRet;
}

bool IFile::Read8(int8& value)
{
	return Read(&value, 1) == 1;
}

bool IFile::Read16(int16& value)
{
	bool bRet = Read(&value, 2) == 2;
	if( ShouldSwapBytes() )
	{
		value = Swap16(value);
	}
	return bRet;
}

bool IFile::Read32(int32& value)
{
	bool bRet = Read(&value, 4) == 4;
	if( ShouldSwapBytes() )
	{
		value = Swap32(value);
	}
	return bRet;
}

bool IFile::Read64(int64& value)
{
	bool bRet = Read(&value, 8) == 8;
	if( ShouldSwapBytes() )
	{
		value = Swap64(value);
	}
	return bRet;
}

bool IFile::Read32(float32& fValue)
{
	return Read32((uint32&) fValue);
}

bool IFile::Read64(float64& fValue)
{
	return Read64((uint64&) fValue);
}

bool IFile::Read8(bool& bValue)
{
	uint8 value = 0;
	bool bRet = Read8(value);
	bValue = value != 0;
	return bRet;
}

//////////////////////////////////////
bool IFile::Write8(uint8 value)
{
	return Write(&value, 1) == 1;
}

bool IFile::Write16(uint16 value)
{
	if( ShouldSwapBytes() )
	{
		value = Swap16(value);
	}
	return Write(&value, 2) == 2;
}

bool IFile::Write32(uint32 value)
{
	if( ShouldSwapBytes() )
	{
		value = Swap32(value);
	}
	return Write(&value, 4) == 4;
}

bool IFile::Write64(uint64 value)
{
	if( ShouldSwapBytes() )
	{
		value = Swap64(value);
	}
	return Write(&value, 8) == 8;
}

bool IFile::Write8(int8 value)
{
	return Write(&value, 1) == 1;
}

bool IFile::Write16(int16 value)
{
	if( ShouldSwapBytes() )
	{
		value = Swap16(value);
	}
	return Write(&value, 2) == 2;
}

bool IFile::Write32(int32 value)
{
	if( ShouldSwapBytes() )
	{
		value = Swap32(value);
	}
	return Write(&value, 4) == 4;
}

bool IFile::Write64(int64 value)
{
	if( ShouldSwapBytes() )
	{
		value = Swap64(value);
	}
	return Write(&value, 8) == 8;
}

bool IFile::Write32(float32 fValue)
{
	union FloatAsInt32 { float32 f; int32 n; } fai = { fValue };
	return Write32(fai.n);

}

bool IFile::Write64(float64 fValue)
{
	union FloatAsInt64 { float64 f; int64 n; } fai = { fValue };
	return Write64(fai.n);
}

bool IFile::Write8(bool bValue)
{
	uint8 value = bValue ? 0x01 : 0x00;
	return Write8(value);
}

bool IFile::WriteString(const String& strValue)
{
	const int32 length = strValue.GetLength();
	const int32 byteCount = length * 2;
	return Write(strValue.GetBuffer(), byteCount) == byteCount;
}

bool IFile::WriteStringLine(const String& strValue)
{
	if( !WriteString(strValue) )
	{
		return false;
	}
	return WriteString("\n");
}

bool IFile::WriteString(const String& strValue, const TextEncoding& te)
{
	ByteArray anContent = strValue.ToBytes(te, false); // Don't add ending 0

	const int size = anContent.GetSize();
	return Write(anContent.GetPtr(), size) == size;
}

bool IFile::WriteStringLine(const String& strValue, const TextEncoding& te)
{
	if( !WriteString(strValue, te) )
	{
		return false;
	}
	return WriteString("\n", te);
}

bool IFile::WriteStringUtf8(const String& strValue)
{
	ByteArray anContent = strValue.ToBytes(TextEncoding::Utf8(), false); // Don't add ending 0
	const int size = anContent.GetSize();
	return Write(anContent.GetPtr(), size) == size;
}

///////////////////////////////////////////////////////////////////////////////
}


