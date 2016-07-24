///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_FORMATTYPE_H
#define CHUSTD_FORMATTYPE_H

namespace chustd
{
	void FormatInt32(int32 value, wchar* pszBuffer); // pszBuffer : 12 chars
	void FormatInt64(int64 value, wchar* pszBuffer); // pszBuffer : 22 chars

	void FormatUInt32(uint32 value, wchar* pszBuffer); // pszBuffer : 12 chars
	void FormatUInt64(uint64 value, wchar* pszBuffer); // pszBuffer : 24 chars

	void FormatInt32(int32 value, wchar* pszBuffer, int8 width, wchar cFill); // pszBuffer : 12 chars
	void FormatInt64(int64 value, wchar* pszBuffer, int8 width, wchar cFill); // pszBuffer : 22 chars

	void FormatUInt32(uint32 value, wchar* pszBuffer, int8 width, wchar cFill); // pszBuffer : 12 chars
	void FormatUInt64(uint64 value, wchar* pszBuffer, int8 width, wchar cFill); // pszBuffer : 24 chars

	void FormatPtr(void* p, wchar* pszBuffer); // pszBuffer : 20 chars
	
	void FormatUInt8Hex(uint8 value, wchar* pszBuffer, bool uppercase); // pszBuffer : 12 chars
	void FormatUInt16Hex(uint16 value, wchar* pszBuffer, bool uppercase); // pszBuffer : 12 chars
	void FormatUInt32Hex(uint32 value, wchar* pszBuffer, bool uppercase); // pszBuffer : 12 chars
	void FormatUInt32Hex(uint32 value, wchar* pszBuffer, int8 width, wchar cFill, bool uppercase); // pszBuffer : 12 chars
	void FormatUInt64Hex(uint64 value, wchar* pszBuffer, bool uppercase); // pszBuffer : 12 chars
	void FormatUInt64Hex(uint64 value, wchar* pszBuffer, int8 width, wchar cFill, bool uppercase); // pszBuffer : 12 chars

	void FormatUInt8Bin(uint8 value, wchar* pszBuffer); // pszBuffer : 9 chars
	void FormatUInt16Bin(uint16 value, wchar* pszBuffer); // pszBuffer : 17 chars
	void FormatUInt32Bin(uint32 value, wchar* pszBuffer); // pszBuffer : 33 chars
	void FormatUInt32Bin(uint32 value, wchar* pszBuffer, int8 width, wchar cFill); // pszBuffer : 33 chars
	void FormatUInt64Bin(uint64 value, wchar* pszBuffer); // pszBuffer : 33 chars
	void FormatUInt64Bin(uint64 value, wchar* pszBuffer, int8 width, wchar cFill); // pszBuffer : 33 chars
}

#endif // ndef CHUSTD_FORMATTYPE_H
