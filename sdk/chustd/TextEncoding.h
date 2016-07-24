///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_TEXTENCODING_H
#define CHUSTD_TEXTENCODING_H

#include "Array.h"

namespace chustd {

class TextEncoding
{
public:
	virtual String    BytesToString(const ByteArray& bytes) const;
	ByteArray StringToBytes(const String& str) const;

	enum ExtractStatus
	{
		esNoError,
		esUnexpectedEndOfString,
		esBadParam,

		esUtf8BadByteSequenceStarter,
		esUtf8BadByteInSequence,

		esUtf16SecondSurrogateNotFound,
		esUtf16LowSurrogateFoundFirst,
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overridden by derived classes

	// Extracts a code point from an array of encoded bytes
	// bytes : the array of encoded bytes
	// start : (in) index in bytes where to start (out) index of the next code point in bytes 
	// codePoint : (out) result
	// Returns esNoError upon success
	virtual ExtractStatus ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const = 0;

	// Inserts a code point at the end of an array of encoded bytes
	// codePoint : the code point to insert
	// bytes : the array of encoded bytes
	virtual void          InsertCodePoint(int codePoint, ByteArray& bytes) const = 0;

	virtual String GetName() const = 0;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static const Array<const TextEncoding*>& GetKnownEncodings();

public:
	static const TextEncoding& Windows1252();
	static const TextEncoding& Iso8859_1();
	static const TextEncoding& Iso8859_15();
	static const TextEncoding& Utf8();
	static const TextEncoding& Utf16Le();
	static const TextEncoding& Utf16Be();
	static const TextEncoding& Utf32Le();
	static const TextEncoding& Utf32Be();

protected:
	TextEncoding();
	virtual ~TextEncoding();

private:
	static Array<const TextEncoding*> ms_apKnownEncodings;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TextEncoding8Bits : public TextEncoding
{
public:
	virtual String    BytesToString(const ByteArray& bytes) const;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TextEncodingWindows1252 : public TextEncoding8Bits
{
public:
	virtual ExtractStatus ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const;
	virtual void          InsertCodePoint(int codePoint, ByteArray& bytes) const;
	virtual String GetName() const;
};

class TextEncodingIso8859_1 : public TextEncoding8Bits
{
public:
	virtual ExtractStatus ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const;
	virtual void          InsertCodePoint(int codePoint, ByteArray& bytes) const;
	virtual String GetName() const;
};

class TextEncodingIso8859_15 : public TextEncoding8Bits
{
public:
	virtual ExtractStatus ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const;
	virtual void          InsertCodePoint(int codePoint, ByteArray& bytes) const;
	virtual String GetName() const;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TextEncodingUtf8 : public TextEncoding
{
public:
	virtual ExtractStatus ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const;
	virtual void          InsertCodePoint(int codePoint, ByteArray& bytes) const;
	virtual String GetName() const;

	static ExtractStatus ExtractChar(const ByteArray& bytes, int& startAt, uint32& ucs4);
	virtual String BytesToString(const ByteArray& bytes) const;

	static int InsertCodePoint(int codePoint, uint8* buf, int size);
	static String StringFromBytes(const uint8* buf, int size);
	static ExtractStatus ExtractCodePoint(const uint8* buf, int size, int& start, int& codePoint);
};

class TextEncodingUtf16 : public TextEncoding
{
public:
	TextEncodingUtf16(ByteOrder byteOrder);

	virtual ExtractStatus ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const;
	virtual void          InsertCodePoint(int codePoint, ByteArray& bytes) const;
	virtual String GetName() const;

	static ExtractStatus ExtractChar(const ByteArray& bytes, int& startAt, uint32& ucs4, ByteOrder byteOrder);
private:
	ByteOrder m_eByteOrder;
};

class TextEncodingUtf32 : public TextEncoding
{
public:
	TextEncodingUtf32(ByteOrder byteOrder);

	virtual ExtractStatus ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const;
	virtual void          InsertCodePoint(int codePoint, ByteArray& bytes) const;
	virtual String GetName() const;

private:
	ByteOrder m_eByteOrder;
};

} // namespace chustd

#endif // ndef CHUSTD_TEXTENCODING_H
