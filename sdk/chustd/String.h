///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_STRING_H
#define CHUSTD_STRING_H

namespace chustd {

class StringData;
class ByteArray;
class TextEncoding;

class String;
class StringArray;

// Immutable UTF-16 string
class String  
{
public:
	String();
	String(const char* psz); // Encoding = latin-1
	String(const char* psz, int length); // Encoding = latin-1
	String(const wchar* psz);
	String(const wchar* psz, int length);
	String(const String& str);
			
	~String();

	// Makes the string empty - Frees all characters 
	void Empty();

	// Returns true if the string is empty
	bool IsEmpty() const {	return ( m_pszBuffer[0] == 0); }

	// Gets the string length in number of UTF-16 code units
	int GetLength() const;

	static int SZLength(const wchar* psz);
	static int SZLength(const char* psz);

	// Gets a code unit at a specific position
	uint16 GetAt(int index) const;

	// Gets a code point at a specific position and increment the index
	int GetCodePoint(int& start) const;

	// http://www.unicode.org/faq/casemap_charprop.html
	
	// Returns a lowercase version of the string
	String ToLowerCase() const;
	
	// Returns an uppercase version of the string
	String ToUpperCase() const;

	// Initializes a string with an external string
	String& operator = (const String& str);
	//String& operator = (const wchar* psz);
	String& operator = (int codePoint);

	// Converts the string to an atomic type
	// cArg : 'x' for hexa, 'b' for binary
	bool ToInt(int& val, char cArg = 0) const;
	bool ToBool(bool& bVal) const;
	bool ToFloat(float32& fVal, bool bStrict=true) const; // bStrict : no garbage chars after the float
	bool ToFloat(float64& fVal, bool bStrict=true) const; // bStrict : no garbage chars after the float

	// Converts an int to a string
	// cArg : 'x' for hexa, 'b' for binary
	static String FromInt(int val, char cArg = 0);
	static String FromInt(int val, char cArg, int8 width, wchar cFill = ' ');
	static String FromInt64(int64 val, char arg = 0, int8 width = -1, wchar fill = ' ');

	bool IsBinaryEqual(const wchar* psz) const;
	bool IsBinaryEqual(const String& str) const;

	bool IsBinaryEqualPartially(int startAt, const String& str) const;

	bool StartsWith(const String& str) const;
	bool EndsWith(const String& str) const;

	// Conversion operator (never returns null)
	const wchar* GetBuffer() const { return (wchar*)m_pszBuffer; }
	const wchar* GetPtr() const { return (wchar*)m_pszBuffer; }

	// Sets the content of the string with a buffer
	// length gives the number of character (final 0 not included)
	void SetBuffer(const wchar* pBuffer, int length);

	// Gets the internal buffer
	wchar* GetUnsafeBuffer(int wantedLength);

	// Gets a copy of the left, right or middle part of the current string
	String Left(int count) const;
	String Right(int count) const;
	String Mid(int first, int count) const;
	String Mid(int first) const { return Mid(first, GetLength() - first); }

	// Cats two strings
	friend const String operator + (const String& str1, const String& str2);
	friend const String operator + (const String& str1, const wchar* psz);
	friend const String operator + (const wchar* psz, const String& str2);

	// Splits a string by endlines : single \r and \n or \r\n couples
	StringArray SplitByEndlines() const;

	// Splits a string by a character
	StringArray Split(int codePoint) const;

	// Splits a string by another string
	StringArray Split(const String& str) const;

	// Converts this String to an array of charset encoded characters
	ByteArray ToBytes(const chustd::TextEncoding& te, bool addEndingZero = false) const;

	// Creates a new string from an array of charset encoded characters
	static String FromBytes(const ByteArray& bytes, const chustd::TextEncoding& te);
	
	// Creates a new string from a C string ended with 0
	static String FromAsciiSZ(const char* psz);

	// Returns the index of the first occurrence of a string
	// Returns -1 if nothing found
	int Find(const String& strToFind, int startAt) const;

	// Returns the index of the first occurrence of a one of the characters defined in strChars
	// Returns -1 if nothing found
	int FindOneOf(const String& strChars, int startAt) const;

	// Replaces all occurrences of a string with another string
	String ReplaceAll(const String& strWhat, const String& strWith) const;

	// Returns a copy of the string without both leading and trailing blank chars
	String Trim() const;

	// Returns a string right filled with characters up to length
	String Pad(int length, int codePoint) const;

	// Inserts a string at a specific position
	String InsertAt(int index, const String& str) const;

	// Unifies newline characters: convert all newlines to either dos or unix types
	enum NewlineType { NT_Unix, NT_Dos };
	String UnifyNewlines(NewlineType nt) const;

	bool ToUtf8Z(char* buf, int size) const;

	template<int TSIZE>
	bool ToUtf8Z(char (&buf)[TSIZE]) const { return ToUtf8Z(buf, TSIZE); }

	static String FromUtf8(const char* buf, int len);
	static String FromUtf8Z(const char* buf);
	
	// Converts an Unicode code point into one or two UTF-16 code units
	// If the code point can fit in one uint16 only, then utf16First is filled and utf16Next is set to 0
	// Returns the number of assigned code units
	static int CodePointToCodeUnits(int codePoint, uint16& utf16First, uint16& utf16Next);

	bool Contains(const String& what) const { return Find(what, 0) >= 0; }
	
	int CompareBin(const String& right) const;

private:
	// Points actually on StringData::m_szBuffer
	// so we keep 99% compatibility with C strings :-)
	// This pointer is never null
	uint16* m_pszBuffer;

private:
	void Unref();

	// Gets a CStringData pointer from the StringData sz buffer of the String instance
	const chustd::StringData* GetData() const;
	chustd::StringData* GetData();

	// Computes the difference between two addresses, saturates and asserts if the length
	// found is greater to the max length (0x7fffffff)
	static int DiffPtr(const void* pA, const void* pB, int charSize);

private:
	struct SCharRange { uint16 min, max; };
};

inline bool operator == (const String& str, const wchar* psz)
{
	return str.IsBinaryEqual(psz);
}
inline bool operator == (const wchar* psz, const String& str)
{
	return str.IsBinaryEqual(psz);
}
inline bool operator == (const String& str1, const String& str2)
{
	return str1.IsBinaryEqual(str2);
}

inline bool operator != (const String& str, const wchar* psz)
{
	return !str.IsBinaryEqual(psz);
}
inline bool operator != (const wchar* psz, const String& str)
{
	return !str.IsBinaryEqual(psz);
}
inline bool operator != (const String& str1, const String& str2)
{
	return !str1.IsBinaryEqual(str2);
}

} // namespace chustd

#endif // ndef CHUSTD_STRING_H
