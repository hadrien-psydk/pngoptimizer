///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextEncoding.h"
#include "StringBuilder.h"
#include "StringData.h"

using namespace chustd;

///////////////////////////////////////////////////////////////////////////////
Array<const TextEncoding*> TextEncoding::ms_apKnownEncodings;

const uint8 k_nReplaceChar = '?';

///////////////////////////////////////////////////////////////////////////////
// Globals as we want them to be all constructed so they are registered in ms_apKnownEncodings
static TextEncodingWindows1252 g_teWindows1252;
static TextEncodingIso8859_1   g_teIso8859_1;
static TextEncodingIso8859_15  g_teIso8859_15;
static TextEncodingUtf8        g_teIsoUtf8;
static TextEncodingUtf16       g_teIsoUtf16Le(boLittleEndian);
static TextEncodingUtf16       g_teIsoUtf16Be(boBigEndian);
static TextEncodingUtf32       g_teIsoUtf32Le(boLittleEndian);
static TextEncodingUtf32       g_teIsoUtf32Be(boBigEndian);
///////////////////////////////////////////////////////////////////////////////
const TextEncoding& TextEncoding::Windows1252()
{
	return g_teWindows1252;
}

const TextEncoding& TextEncoding::Iso8859_1()
{
	return g_teIso8859_1;
}

const TextEncoding& TextEncoding::Iso8859_15()
{
	return g_teIso8859_15;
}

const TextEncoding& TextEncoding::Utf8()
{
	return g_teIsoUtf8;
}

const TextEncoding& TextEncoding::Utf16Le()
{
	return g_teIsoUtf16Le;
}

const TextEncoding& TextEncoding::Utf16Be()
{
	return g_teIsoUtf16Be;
}

const TextEncoding& TextEncoding::Utf32Le()
{
	return g_teIsoUtf32Le;
}

const TextEncoding& TextEncoding::Utf32Be()
{
	return g_teIsoUtf32Be;
}

///////////////////////////////////////////////////////////////////////////////

TextEncoding::TextEncoding()
{
	ms_apKnownEncodings.Add(this);
}

TextEncoding::~TextEncoding()
{
	ms_apKnownEncodings.Remove(this);
}

const Array<const TextEncoding*>& TextEncoding::GetKnownEncodings()
{
	return ms_apKnownEncodings;	
}

String TextEncoding::BytesToString(const ByteArray& bytes) const
{
	const int size = bytes.GetSize();
	int32 start = 0;

	StringBuilder sb;
	while(true)
	{
		if( start >= size )
			break;

		int codePoint = 0;
		ExtractStatus es = ExtractCodePoint(bytes, start, codePoint);
		if( es != esNoError )
			break;

		sb += codePoint;
	}
	return sb.ToString();
}

ByteArray TextEncoding::StringToBytes(const String& str) const
{
	ByteArray bytes;

	int index = 0;
	for(;;)
	{
		int codePoint = str.GetCodePoint(index);
		if( codePoint == -1 )
		{
			break;
		}
		
		InsertCodePoint(codePoint, bytes);
	}
	return bytes;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Optimized version for 8 bits encodings
String TextEncoding8Bits::BytesToString(const ByteArray& bytes) const
{
	const int size = bytes.GetSize();
	int32 start = 0;

	String strOut;
	uint16* pDst = (uint16*) strOut.GetUnsafeBuffer(size);

	for(int i = 0; i < size; ++i)
	{
		int codePoint = 0;
		ExtractStatus es = ExtractCodePoint(bytes, start, codePoint);
		if( es != esNoError )
			break;

		// No known 8 bits encoding that encodes CodePoints > 65536,
		// but a text encoding class that does not follow this rule is 
		// still free to override this function and do specific handling
		pDst[i] = uint16(codePoint);
	}
	
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////
// windows-1252

// From 0x80 to 0x9f
static const uint16 TextEncodingWindows1252_aConvert[32] = 
{
	0x20AC,	// 0x80 EURO SIGN
    0x0081,	// 0x81 UNDEFINED
	0x201A,	// 0x82 SINGLE LOW-9 QUOTATION MARK
	0x0192,	// 0x83 LATIN SMALL LETTER F WITH HOOK
	0x201E,	// 0x84 DOUBLE LOW-9 QUOTATION MARK
	0x2026,	// 0x85 HORIZONTAL ELLIPSIS
	0x2020,	// 0x86 DAGGER
	0x2021,	// 0x87 DOUBLE DAGGER
	0x02C6,	// 0x88 MODIFIER LETTER CIRCUMFLEX ACCENT
	0x2030,	// 0x89 PER MILLE SIGN
	0x0160,	// 0x8A LATIN CAPITAL LETTER S WITH CARON
	0x2039,	// 0x8B SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	0x0152,	// 0x8C LATIN CAPITAL LIGATURE OE
    0x008D,	// 0x8D UNDEFINED
	0x017D,	// 0x8E LATIN CAPITAL LETTER Z WITH CARON
    0x008F,	// 0x8F UNDEFINED
    0x0090,	// 0x90 UNDEFINED
	0x2018,	// 0x91 LEFT SINGLE QUOTATION MARK
	0x2019,	// 0x92 RIGHT SINGLE QUOTATION MARK
	0x201C,	// 0x93 LEFT DOUBLE QUOTATION MARK
	0x201D,	// 0x94 RIGHT DOUBLE QUOTATION MARK
	0x2022,	// 0x95 BULLET
	0x2013,	// 0x96 EN DASH
	0x2014,	// 0x97 EM DASH
	0x02DC,	// 0x98 SMALL TILDE
	0x2122,	// 0x99 TRADE MARK SIGN
	0x0161,	// 0x9A LATIN SMALL LETTER S WITH CARON
	0x203A,	// 0x9B SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	0x0153,	// 0x9C LATIN SMALL LIGATURE OE
    0x009D,	// 0x9D	UNDEFINED
	0x017E,	// 0x9E LATIN SMALL LETTER Z WITH CARON
	0x0178	// 0x9F LATIN CAPITAL LETTER Y WITH DIAERESIS
};

String TextEncodingWindows1252::GetName() const
{
	static const String strName = "windows-1252";
	return strName;
}

TextEncoding::ExtractStatus TextEncodingWindows1252::ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const
{
	const int size = bytes.GetSize();
	if( start >= size )
	{
		ASSERT(0); // Bad param, warn the user
		return esBadParam;
	}

	uint8 value = bytes[start];
	start++;

	if( value < 0x0080 || value > 0x009f )
	{
		// Compatible value
		codePoint = value;
	}
	else
	{
		value -= 0x0080;
		codePoint = TextEncodingWindows1252_aConvert[value];
	}
	return esNoError;
}

void TextEncodingWindows1252::InsertCodePoint(int codePoint, ByteArray& bytes) const
{
	if( codePoint < 0x0080 || (0x009f < codePoint && codePoint <= 0x00ff) )
	{
		// Compatible value
		bytes.Add( uint8(codePoint));
		return;
	}

	// Look for our code point
	for(int i = 0; i < 32; ++i)
	{
		if( codePoint == TextEncodingWindows1252_aConvert[i] )
		{
			// Found !
			uint32 result = 0x0080 + i;
			bytes.Add( uint8(result));
			return;
		}
	}

	bytes.Add(k_nReplaceChar);
}
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
// ISO-8859-1

String TextEncodingIso8859_1::GetName() const
{
	static const String strName = "ISO-8859-1";
	return strName;
}

TextEncoding::ExtractStatus TextEncodingIso8859_1::ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const
{
	const int size = bytes.GetSize();
	if( start >= size )
	{
		ASSERT(0); // Bad param, warn the user
		return esBadParam;
	}

	const uint8 value = bytes[start];
	start++;

	// Compatible value
	codePoint = value;
	return esNoError;
}

void TextEncodingIso8859_1::InsertCodePoint(int codePoint, ByteArray& bytes) const
{
	if( (0x0000 < codePoint && codePoint <= 0x00ff) )
	{
		// Compatible value
		bytes.Add( uint8(codePoint));
		return;
	}

	bytes.Add(k_nReplaceChar);
}
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
// ISO-8859-15

// From 0xA0 to 0xbf
static const wchar CharsetIso8859_15_aConvert[32] = 
{
	0x00A0,	// 0xA0 --NO-BREAK SPACE
	0x00A1,	// 0xA1 --INVERTED EXCLAMATION MARK
	0x00A2,	// 0xA2 --CENT SIGN
	0x00A3,	// 0xA3 --POUND SIGN
	0x20AC,	// 0xA4 EURO SIGN
	0x00A5,	// 0xA5 --YEN SIGN
	0x0160,	// 0xA6 LATIN CAPITAL LETTER S WITH CARON
	0x00A7,	// 0xA7 --SECTION SIGN
	0x0161,	// 0xA8 LATIN SMALL LETTER S WITH CARON
	0x00A9,	// 0xA9 --COPYRIGHT SIGN
	0x00AA,	// 0xAA --FEMININE ORDINAL INDICATOR
	0x00AB,	// 0xAB --LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x00AC,	// 0xAC --NOT SIGN
	0x00AD,	// 0xAD --SOFT HYPHEN
	0x00AE,	// 0xAE --REGISTERED SIGN
	0x00AF,	// 0xAF --MACRON
	0x00B0,	// 0xB0 --DEGREE SIGN
	0x00B1,	// 0xB1 --PLUS-MINUS SIGN
	0x00B2,	// 0xB2 --SUPERSCRIPT TWO
	0x00B3,	// 0xB3 --SUPERSCRIPT THREE
	0x017D,	// 0xB4 LATIN CAPITAL LETTER Z WITH CARON
	0x00B5,	// 0xB5 --MICRO SIGN
	0x00B6,	// 0xB6 --PILCROW SIGN
	0x00B7,	// 0xB7 --MIDDLE DOT
	0x017E,	// 0xB8 LATIN SMALL LETTER Z WITH CARON
	0x00B9,	// 0xB9 --SUPERSCRIPT ONE
	0x00BA,	// 0xBA --MASCULINE ORDINAL INDICATOR
	0x00BB,	// 0xBB --RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x0152,	// 0xBC LATIN CAPITAL LIGATURE OE
	0x0153,	// 0xBD LATIN SMALL LIGATURE OE
	0x0178,	// 0xBE LATIN CAPITAL LETTER Y WITH DIAERESIS
	0x00BF	// 0xBF --INVERTED QUESTION MARK
};

String TextEncodingIso8859_15::GetName() const
{
	static const String strName = "ISO-8859-15";
	return strName;
}

TextEncoding::ExtractStatus TextEncodingIso8859_15::ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const
{
	const int size = bytes.GetSize();
	if( start >= size )
	{
		ASSERT(0); // Bad param, warn the user
		return esBadParam;
	}

	uint8 value = bytes[start];
	start++;

	if( value < 0x00a0 || value > 0x00bf )
	{
		// Compatible value
		codePoint = value;
	}
	else
	{
		value -= 0x00a0;
		codePoint = CharsetIso8859_15_aConvert[value];
	}
	return esNoError;
}

void TextEncodingIso8859_15::InsertCodePoint(int codePoint, ByteArray& bytes) const
{
	if( codePoint < 0x00a0 || (0x00bf < codePoint && codePoint <= 0x00ff) )
	{
		// Compatible value
		bytes.Add( uint8(codePoint));
		return;
	}

	// Look for our code point
	for(int i = 0; i < 32; ++i)
	{
		if( codePoint == CharsetIso8859_15_aConvert[i] )
		{
			// Found !
			uint32 result = 0x00a0 + i;
			bytes.Add( uint8(result));
			return;
		}
	}

	bytes.Add(k_nReplaceChar);
}
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
// UTF-8

String TextEncodingUtf8::GetName() const
{
	static const String strName = "UTF-8";
	return strName;
}

TextEncoding::ExtractStatus TextEncodingUtf8::ExtractCodePoint(const ByteArray& bytes,
                                                               int& start, int& codePoint) const
{
	return ExtractCodePoint(bytes.GetPtr(), bytes.GetSize(), start, codePoint);
}

TextEncoding::ExtractStatus TextEncodingUtf8::ExtractCodePoint(const uint8* buf, int size,
                                                    int& start, int& codePoint)
{
	if( start >= size )
	{
		return esBadParam;
	}

	int8 byteSequence = 0;
	uint32 char32 = 0;

	const uint8* pSrc = buf;
	
	uint8 byte = pSrc[start];
	start++;

	if( byte <= 0x7F )
	{
		codePoint = byte;
		return esNoError;
	}
	else if( (byte & 0xE0) == 0xC0 )
	{
		// 110xxxxx 10xxxxxx
		char32 = byte & 0x1F;
		byteSequence = 1;
	}
	else if( (byte & 0xF0) == 0xE0 )
	{
		// 1110xxxx 10xxxxxx 10xxxxxx
		char32 = byte & 0x0F;
		byteSequence = 2;
	}
	else if( (byte & 0xF8) == 0xF0 )
	{
		// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		char32 = byte & 0x07;
		byteSequence = 3;
	}
	else if( (byte & 0xFC) == 0xF8 )
	{
		// 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		char32 = byte & 0x03;
		byteSequence = 4;
	}
	else if( (byte & 0xFE) == 0xFC )
	{
		// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		char32 = byte & 0x01;
		byteSequence = 5;
	}
	else
	{
		// Bad byte sequence starter
		return esUtf8BadByteSequenceStarter;
	}

	if( start + byteSequence > size )
	{
		return esUnexpectedEndOfString;
	}

	for(int i = 0; i < byteSequence; ++i)
	{
		uint8 byte = pSrc[start + i];

		// Remaining bytes
		if( (byte & 0xC0) != 0x80 )
		{
			// Bad byte in sequence
			return esUtf8BadByteInSequence;
		}

		char32 <<= 6;
		char32 |= (byte & 0x3F);
	}

	// Ok
	codePoint = char32;
	start += byteSequence;

	return esNoError;
}

String TextEncodingUtf8::BytesToString(const ByteArray& bytes) const
{
	return StringFromBytes(bytes.GetPtr(), bytes.GetSize());
}

String TextEncodingUtf8::StringFromBytes(const uint8* buf, int size)
{
	int start = 0;
	
	Array<uint16> utf16;
	if( !utf16.EnsureCapacity(size + size / 4) )
	{
		return String();
	}
	
	int utf16Length = 0;
	for(;;)
	{
		//int32 iDstUnit = 0;
		ExtractStatus es = esNoError;

		int codePoint = 0;
		es = ExtractCodePoint(buf, size, start, codePoint);
		if( es != esNoError )
		{
			break;
		}
			
		if( codePoint <= 0x0000ffff )
		{
			utf16.Add(uint16(codePoint));
			utf16Length++;
		}
		else
		{
			// Manage surrogate pairs
			uint16 unit0, unit1;
			int32 codeUnitCount = String::CodePointToCodeUnits(codePoint, unit0, unit1);
			if( codeUnitCount == 1 )
			{
				utf16.Add(unit0);
				utf16Length++;
			}
			else if( codeUnitCount == 2 )
			{
				utf16.Add(unit0);
				utf16.Add(unit1);
				utf16Length += 2;
			}
		}
	}
		
	return String((wchar*)(utf16.GetPtr()), utf16Length);
}

///////////////////////////////////////////////////////////////////////////////
void TextEncodingUtf8::InsertCodePoint(int codePoint, ByteArray& bytes) const
{
	uint8 buf[6];
	int len = InsertCodePoint(codePoint, buf, sizeof(buf));
	bytes.Add(buf, len);
}

///////////////////////////////////////////////////////////////////////////////
// Returns the size of the UTF-8 sequence, 0 upon error
int TextEncodingUtf8::InsertCodePoint(int codePoint, uint8* buf, int size)
{
	if( codePoint <= 0x0000007F )
	{
		if( size < 1 ) return 0;
		buf[0] = uint8(codePoint);
		return 1;
	}
	if( codePoint <= 0x000007FF )
	{
		// 110xxxxx 10xxxxxx (11 bits)
		if( size < 2 ) return 0;
		buf[0] = uint8( (codePoint >> 6)     | 0x00C0);
		buf[1] = uint8( (codePoint & 0x003F) | 0x0080);
		return 2;
	}
	if( codePoint <= 0x0000FFFF )
	{
		// 1110xxxx 10xxxxxx 10xxxxxx (16 bits)
		if( size < 3 ) return 0;
		buf[0] = uint8( (codePoint >> 12)           | 0x00E0);
		buf[1] = uint8( ((codePoint >> 6) & 0x003F) | 0x0080);
		buf[2] = uint8( ((codePoint) & 0x003F)      | 0x0080);
		return 3;
	}
	if( codePoint <= 0x0010FFFF )
	{
		// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (21 bits)
		if( size < 4 ) return 0;
		buf[0] = uint8(  (codePoint >> 18)           | 0x00F0);
		buf[1] = uint8( ((codePoint >> 12) & 0x003F) | 0x0080);
		buf[2] = uint8( ((codePoint >>  6) & 0x003F) | 0x0080);
		buf[3] = uint8( ((codePoint      ) & 0x003F) | 0x0080);
		return 4;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
// UTF-16

TextEncodingUtf16::TextEncodingUtf16(ByteOrder byteOrder) : m_eByteOrder(byteOrder)
{

}

String TextEncodingUtf16::GetName() const
{
	static const String strNameBe = "UTF-16LE";
	static const String strNameLe = "UTF-16BE";
	
	if( m_eByteOrder == boLittleEndian )
	{
		return strNameLe;
	}
	return strNameBe;
}

TextEncoding::ExtractStatus TextEncodingUtf16::ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const
{
	const int size = bytes.GetSize();
	if( start >= size )
	{
		ASSERT(0); // Bad param, warn the user
		return esBadParam;
	}

	const uint8* pSrc = bytes.GetPtr();

	// Is next byte out of bound ?
	if( start + 1 >= size )
	{
		return esUnexpectedEndOfString; // Unexpected end of string
	}

	uint8 byte0 = pSrc[start];
	uint8 byte1 = pSrc[start + 1];
	
	uint16 nHi = 0;
	if( m_eByteOrder == boBigEndian )
	{
		nHi = uint8((byte0 << 8) | byte1);
	}
	else
	{
		nHi = uint8((byte1 << 8) | byte0);
	}
	start += 2;

	// A surrogate ?
	if( nHi < 0xD800 || nHi > 0xDFFF )
	{
		// Nop, standard char
		codePoint = nHi;
		return esNoError;
	}

	if( 0xDBFF < nHi )
	{
		// Low surrogate used first, this is an error
		return esUtf16LowSurrogateFoundFirst;
	}

	/////////////////////////////////////////////
	// Get the second surrogate (low)
	if( start + 2 < size )
	{
		return esUnexpectedEndOfString; // Unexpected end of string
	}

	uint8 byte2 = pSrc[start];
	uint8 byte3 = pSrc[start + 1];
	
	uint16 nLo = 0;
	if( m_eByteOrder == boBigEndian )
	{
		nLo = uint8((byte2 << 8) | byte3);
	}
	else
	{
		nLo = uint8((byte3 << 8) | byte2);
	}
	start += 2;


	// A low surrogate ?
	if( !(0xDBFF < nLo && nLo <= 0xDFFF) )
	{
		// Nop	
		return esUtf16SecondSurrogateNotFound;
	}

	codePoint = 0x10000 + ((nHi - 0xD800) << 10) + (nLo - 0xDC00);

	return esNoError;
}

void TextEncodingUtf16::InsertCodePoint(int codePoint, ByteArray& bytes) const
{
	uint16 utf16First, utf16Next;
	int32 codeUnitCount = String::CodePointToCodeUnits(codePoint, utf16First, utf16Next);
	if( codeUnitCount == 0 )
		return;

	uint8 nHi = uint8(utf16First >> 8);
	uint8 nLo = uint8(utf16First & 0x00ff);

	if( m_eByteOrder == boBigEndian )
	{
		bytes.Add(nHi);
		bytes.Add(nLo);
	}
	else
	{
		bytes.Add(nLo);
		bytes.Add(nHi);
	}

	if( codeUnitCount == 1 )
		return;

	uint8 nHi2 = uint8(utf16Next >> 8);
	uint8 nLo2 = uint8(utf16Next & 0x00ff);

	if( m_eByteOrder == boBigEndian )
	{
		bytes.Add(nHi2);
		bytes.Add(nLo2);
	}
	else
	{
		bytes.Add(nLo2);
		bytes.Add(nHi2);
	}

}


///////////////////////////////////////////////////////////////////////////////////////////
// UTF-32

TextEncodingUtf32::TextEncodingUtf32(ByteOrder byteOrder) : m_eByteOrder(byteOrder)
{

}

String TextEncodingUtf32::GetName() const
{
	static const String strNameBe = "UTF-32LE";
	static const String strNameLe = "UTF-32BE";
	
	if( m_eByteOrder == boLittleEndian )
	{
		return strNameLe;
	}
	return strNameBe;
}


////////////////////////////////////
TextEncoding::ExtractStatus TextEncodingUtf32::ExtractCodePoint(const ByteArray& bytes, int& start, int& codePoint) const
{
	const int size = bytes.GetSize();
	if( start >= size )
	{
		ASSERT(0); // Bad param, warn the user
		return esBadParam;
	}

	if( (start + 4) >= size )
	{
		return esUnexpectedEndOfString;
	}

	uint8 byte0 = bytes[start];
	uint8 byte1 = bytes[start + 1];
	uint8 byte2 = bytes[start + 2];
	uint8 byte3 = bytes[start + 3];

	if( m_eByteOrder == boBigEndian )
	{
		codePoint = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	}
	else
	{
		codePoint = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;
	}

	start += 4;
	return esNoError;
}

void TextEncodingUtf32::InsertCodePoint(int codePoint, ByteArray& bytes) const
{
	uint8 byte0 = uint8( (codePoint >> 24) & 0x00ff);
	uint8 byte1 = uint8( (codePoint >> 16) & 0x00ff);
	uint8 byte2 = uint8( (codePoint >> 8) & 0x00ff);
	uint8 byte3 = uint8( (codePoint) & 0x00ff);

	if( m_eByteOrder == boBigEndian )
	{
		bytes.Add(byte0);
		bytes.Add(byte1);
		bytes.Add(byte2);
		bytes.Add(byte3);
	}
	else
	{
		bytes.Add(byte3);
		bytes.Add(byte2);
		bytes.Add(byte1);
		bytes.Add(byte0);
	}
}
