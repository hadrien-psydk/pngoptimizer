///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_IFILE_H
#define CHUSTD_IFILE_H

#include "String.h"

namespace chustd {

class IFile  
{
public:
	enum
	{
		modeRead = 1,
		modeWrite = 2,
		modeReadWrite = 3,
		modeAppend = 4,
		modeLittleEndian = 8
	};

	enum Whence
	{
		posBegin = 0,
		posCurrent,
		posEnd
	};

	//////////////////////////////////////
	virtual bool  SetPosition(int64 offset, Whence eWhence = posBegin) = 0;
	virtual int64 GetPosition() const = 0;
	virtual int64 GetSize() = 0;
	virtual int Read(void* pBuffer, int size) = 0;
	virtual int Write(const void* pBuffer, int size) = 0;
	virtual ByteOrder GetByteOrder() const = 0;
	virtual void SetByteOrder(ByteOrder byteOrder) = 0;
	virtual void Close() = 0;
	//////////////////////////////////////

	//////////////////////////////////////
	bool Read8(uint8& value);
	bool Read16(uint16& value);
	bool Read32(uint32& value);
	bool Read64(uint64& value);

	bool Read8(int8& value);
	bool Read16(int16& value);
	bool Read32(int32& value);
	bool Read64(int64& value);

	bool Read32(float32& fValue);
	bool Read64(float64& fValue);
	bool Read8(bool& bValue);

	/*
	//////////////////////////////////////
	IFile& operator >> (uint8& value)  { Read8(value); return *this; };
	IFile& operator >> (uint16& value) { Read16(value); return *this; };
	IFile& operator >> (uint32& value) { Read32(value); return *this; };
	IFile& operator >> (uint64& value) { Read64(value); return *this; };
	
	IFile& operator >> (int8& value)  { Read8(value); return *this; };
	IFile& operator >> (int16& value) { Read16(value); return *this; };
	IFile& operator >> (int32& value) { Read32(value); return *this; };
	IFile& operator >> (int64& value) { Read64(value); return *this; };

	IFile& operator >> (float32& fValue) { Read32(fValue); return *this; };
	IFile& operator >> (float64& fValue) { Read64(fValue); return *this; };
	IFile& operator >> (bool& bValue)    { Read8(bValue); return *this; };
	//////////////////////////////////////
	*/

	//////////////////////////////////////
	bool Write8(uint8 value);
	bool Write16(uint16 value);
	bool Write32(uint32 value);
	bool Write64(uint64 value);

	bool Write8(int8 value);
	bool Write16(int16 value);
	bool Write32(int32 value);
	bool Write64(int64 value);

	bool Write32(float32 fValue);
	bool Write64(float64 fValue);
	bool Write8(bool bValue);

	// Writes a string to the file as encoded in memory (UTF-16LE or UTF16-BE)
	bool WriteString(const String& strValue);
	bool WriteStringLine(const String& strValue);

	// Writes a string converted to a specific text encoding
	bool WriteString(const String& strValue, const TextEncoding& te);
	bool WriteStringLine(const String& strValue, const TextEncoding& te);

	bool WriteStringUtf8(const String& strValue);

	/*
	//////////////////////////////////////
	IFile& operator << (uint8 value)  { Write8(value); return *this; };
	IFile& operator << (uint16 value) { Write16(value); return *this; };
	IFile& operator << (uint32 value) { Write32(value); return *this; };
	IFile& operator << (uint64 value) { Write64(value); return *this; };
	
	IFile& operator << (int8 value)  { Write8(value); return *this; };
	IFile& operator << (int16 value) { Write16(value); return *this; };
	IFile& operator << (int32 value) { Write32(value); return *this; };
	IFile& operator << (int64 value) { Write64(value); return *this; };

	IFile& operator << (float32 fValue) { Write32(fValue); return *this; };
	IFile& operator << (float64 fValue) { Write64(fValue); return *this; };
	IFile& operator << (bool bValue)    { Write8(bValue); return *this; };

	IFile& operator << (const String& strValue) { WriteString(strValue); return *this; }
	//////////////////////////////////////
	*/

	////////////////////////////////////////
	// Byte swap for little endian/big endian conversions
	static uint16 Swap16(uint16 value);
	static uint32 Swap32(uint32 value);
	static uint64 Swap64(uint64 value);
	static float32 SwapFloat32(float32 fValue);
	static float64 SwapFloat64(float64 fValue);

	static void Swap16(uint16* paValues, int count);
	static void Swap32(uint32* paValues, int count);

	static void Swap16(int16* paValues, int count) { Swap16( (uint16*) paValues, count); }
	static void Swap32(int32* paValues, int count) { Swap32( (uint32*) paValues, count); }

	bool ShouldSwapBytes() const;

	////////////////////////////////////////
	IFile();
	virtual ~IFile();
};

/////////////////////////////////////////////////////////
class TmpEndianMode
{
public:
	TmpEndianMode(IFile& file, ByteOrder byteOrder)
	{
		m_ePrevious = file.GetByteOrder();
		file.SetByteOrder(byteOrder);
		m_pFile = &file;
	}

	~TmpEndianMode()
	{
		m_pFile->SetByteOrder(m_ePrevious);
	}
private:
	IFile* m_pFile;
	ByteOrder m_ePrevious;
};

} // namespace chustd

#endif // ndef CHUSTD_IFILE_H
