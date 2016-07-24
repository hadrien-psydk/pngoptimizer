///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "String.h"
#include "StringData.h"
#include "UnicodeCaseMapping.h"
#include "Array.h"
#include "ScanType.h"
#include "FormatType.h"
#include "TextEncoding.h"
#include "StringBuilder.h"
#include "CodePoint.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
String::String()
{
	m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
}

///////////////////////////////////////////////////////////////////////////////
String::String(const char* psz)
{
	if( psz == null || psz[0] == 0 )
	{
		m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
	}
	else
	{
		int length = String::SZLength(psz);
		uint16* pszNew = StringData::CreateInstance(length);
		if( pszNew == null )
		{
			// Raise exception ?
			m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
		}
		else
		{
			for(int i = 0; i < length; ++i)
			{
				pszNew[i] = wchar(psz[i] & 0x00ff);
			}
			m_pszBuffer = pszNew;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
String::String(const char* psz, int length)
{
	if( psz == null || psz[0] == 0 || length <= 0 )
	{
		m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
	}
	else
	{
		uint16* pszNew = StringData::CreateInstance(length);
		if( pszNew == null )
		{
			// Raise exception ?
			m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
		}
		else
		{
			for(int i = 0; i < length; ++i)
			{
				pszNew[i] = wchar(psz[i] & 0x00ff);
			}
			m_pszBuffer = pszNew;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
String::String(const wchar* psz)
{
	if( psz == null || psz[0] == 0 )
	{
		m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
	}
	else
	{
		const int length = SZLength(psz);
		m_pszBuffer = StringData::CreateInstance(length);
		
		// Note : no need for final zero as CreateInstance already did it for us
		Memory::Copy64From16(m_pszBuffer, psz, length);
	}
}

///////////////////////////////////////////////////////////////////////////////
String::String(const wchar* psz, int length)
{
	if( psz == null )
	{
		m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
	}
	else
	{
		m_pszBuffer = StringData::CreateInstance(length);
		
		// Note : no need for final zero as CreateInstance already did it for us
		Memory::Copy64From16(m_pszBuffer, psz, length);
	}
}

///////////////////////////////////////////////////////////////////////////////
String::String(const String& str)
{
	m_pszBuffer = str.m_pszBuffer;
	ASSERT(m_pszBuffer != null);
	
	StringData* pData = GetData();
	if( pData != StringData::GetNullInstance() )
	{
		pData->AddRef();
	}
}

///////////////////////////////////////////////////////////////////////////////
String::~String()
{
	Unref();
}

//////////////////////////////////////////////////////////////////////
void String::Empty()
{
	Unref();
	m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
}

///////////////////////////////////////////////////////////////////////////////
void String::SetBuffer(const wchar* pBuffer, int length)
{
	Unref();

	if( pBuffer != null && length != 0 )
	{
		m_pszBuffer = StringData::CreateInstance(length);
		Memory::Copy16(m_pszBuffer, pBuffer, length);
		m_pszBuffer[length] = 0;
	}
	else
	{
		m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
	}
}

///////////////////////////////////////////////////////////////////////////////
String& String::operator = (const String& str)
{
	if( this == &str )
		return *this;

	Unref();

	m_pszBuffer = str.m_pszBuffer;
	
	StringData* pData = GetData();
	if( pData != StringData::GetNullInstance() )
	{
		pData->AddRef();
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
String& String::operator = (int codePoint)
{
	Unref();

	int length = (codePoint > 0x0000ffff) ? 2 : 1;
	m_pszBuffer = StringData::CreateInstance(length);
		
	if( length == 1 )
	{
		m_pszBuffer[0] = uint16(codePoint);
	}
	else
	{
		// TODO
		ASSERT(0);

		uint16 surrogateHi = 0;
		uint16 surrogateLo = 0;

		m_pszBuffer[0] = surrogateHi;
		m_pszBuffer[1] = surrogateLo;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
int String::DiffPtr(const void* pA, const void* pB, int charSize)
{
	const uint8* pA8 = (uint8*) pA;
	const uint8* pB8 = (uint8*) pB;

	if( sizeof(void*) > 4 )
	{
		// Check overflow
		const int maxLength = 0x7fffffff;
		int64 length64 = int64(pA8 - pB8);
		length64 >>= (charSize - 1);

		if( length64 > 0x7fffffff )
		{
			ASSERT(0); // Something is bad in the machinery
			return maxLength;
		}
		return int(length64);
	}

	int length = int(pA8 - pB8) >> (charSize - 1);
	return length;
}

///////////////////////////////////////////////////////////////////////////////
int String::SZLength(const wchar* psz)
{
	const wchar* pszStart = psz;
	while( *psz != 0 )
	{
		psz++;
	}
	return DiffPtr(psz, pszStart, 2);
}

///////////////////////////////////////////////////////////////////////////////
int String::SZLength(const char* psz)
{
	const char* pszStart = psz;
	while( *psz != 0 )
	{
		psz++;
	}
	return DiffPtr(psz, pszStart, 1);
}

///////////////////////////////////////////////////////////////////////////////
// Unrefs the StringData instance.
// If the reference counter reachs 0 the StringData instance is deleted
///////////////////////////////////////////////////////////////////////////////
void String::Unref()
{
	StringData* pData = GetData();
	if( pData == StringData::GetNullInstance() )
		return;

	// Should always passed, otherwise some memory trashing occured or one of the String 
	// functions is buggy
	ASSERT( pData->GetRef() > 0);

	pData->Release();
	if( pData->GetRef() <= 0 )
	{
		// The instance owns the data
		pData->Delete();
		m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
int String::GetLength() const
{
	return GetData()->GetLength();
}

///////////////////////////////////////////////////////////////////////////////
uint16 String::GetAt(int index) const
{
	ASSERT(0 <= index && index < GetLength());

	return m_pszBuffer[index];
}

///////////////////////////////////////////////////////////////////////////////////////
// Gets a StringData pointer from the StringData sz buffer of the String instance
const chustd::StringData* String::GetData() const
{
	return chustd::StringData::GetInstance(m_pszBuffer);
}

///////////////////////////////////////////////////////////////////////////////
chustd::StringData* String::GetData()
{
	return chustd::StringData::GetInstance(m_pszBuffer);
}

///////////////////////////////////////////////////////////////////////////////
String String::ToUpperCase() const
{
	const uint16* const * const pPages = UnicodeCaseMapping::GetUcs2ToUpperPages();
	const uint32* const pPage10428 = UnicodeCaseMapping::GetUcs4ToUpperPage10428();

	StringBuilder sb;
	int codeUnitIndex = 0;
	while(true)
	{
		int codePoint = GetCodePoint(codeUnitIndex);
		if( codePoint < 0 )
		{
			break;
		}

		if( codePoint <= 0x0000ffff )
		{
			// Use UCS-2 pages
			uint8 page = uint8(codePoint >> 8);
			uint8 subCode = uint8(codePoint & 0x00ff);

			const uint16* const pPage = pPages[page];
			if( pPage != null && pPage[subCode] != 0 )
			{
				codePoint = pPage[subCode];
			}
			else
			{
				// Find unconditionnal 1:N mapping
				const uint16* pMulti = UnicodeCaseMapping::CodeUnitToUpperMulti( uint16(codePoint));
				if( pMulti != null )
				{
					for(int iNew = 0; iNew < 3; ++iNew)
					{
						if( pMulti[iNew] == 0 )
						{
							break;
						}
						sb += pMulti[iNew];
					}
					
					// Reset the main loop, as 1:N is a special case
					continue;
				}
			}
		}
		else
		{
			int page10428Index = codePoint - 0x10428;
			if( 0 <= page10428Index && page10428Index <= 0x27 )
			{
				codePoint = pPage10428[page10428Index];
			}
		}
		sb += codePoint;
	}
	return sb.ToString();
}

///////////////////////////////////////////////////////////////////////////////
String String::ToLowerCase() const
{
	const uint16* const * const pPages = UnicodeCaseMapping::GetUcs2ToLowerPages();
	const uint32* const pPage10400 = UnicodeCaseMapping::GetUcs4ToLowerPage10400();

	StringBuilder sb;
	int codeUnitIndex = 0;
	while(true)
	{
		int codePoint = GetCodePoint(codeUnitIndex);
		if( codePoint < 0 )
		{
			break;
		}

		if( codePoint <= 0x0000ffff )
		{
			// Use UCS-2 pages
			uint8 page = uint8(codePoint >> 8);
			uint8 subCode = uint8(codePoint & 0x00ff);

			const uint16* const pPage = pPages[page];
			if( pPage != null && pPage[subCode] != 0 )
			{
				codePoint = pPage[subCode];
			}
			else
			{
				// Find unconditionnal 1:N mapping
				const uint16* pMulti = UnicodeCaseMapping::CodeUnitToLowerMulti( uint16(codePoint));
				if( pMulti != null )
				{
					for(int iNew = 0; iNew < 3; ++iNew)
					{
						if( pMulti[iNew] == 0 )
						{
							break;
						}
						sb += pMulti[iNew];
					}
					
					// Reset the main loop, as 1:N is a special case
					continue;
				}
			}
		}
		else
		{
			int page10400Index = codePoint - 0x10400;
			if( 0 <= page10400Index && page10400Index <= 0x27 )
			{
				codePoint = pPage10400[page10400Index];
			}
		}
		sb += codePoint;
	}
	return sb.ToString();
}

//////////////////////////////////////////////////////////////////////
// Global friend functions

///////////////////////////////////////////////////////////////////////////////
const String operator + (const String& str1, const String& str2)
{
	const int dataLength1 = str1.GetData()->GetLength();
	const int dataLength2 = str2.GetData()->GetLength();
	
	if( dataLength1 == 0 )
	{
		return str2;
	}
	if( dataLength2 == 0 )
	{
		return str1;
	}

	uint16* pszNew = StringData::CreateInstance(dataLength1 + dataLength2);
	
	if( dataLength1 > 0 )
	{
		// Copy first part
		const int chunkCount1 = StringData::GetUsedChunkCount(dataLength1);
		Memory::Copy64(pszNew, str1.m_pszBuffer, chunkCount1);
	}

	// Copy second part
	// + 1 to copy the final 0 which might not be at the right place after a FastUInt32Copy
	Memory::Copy16(pszNew + dataLength1, str2.m_pszBuffer, (dataLength2 + 1));

	String str;
	str.m_pszBuffer = pszNew;
	return str;
}

///////////////////////////////////////////////////////////////////////////////
const String operator + (const String& str1, const wchar* psz)
{
	if( psz == null || psz[0] == 0 )
		return str1;

	const int dataLength1 = str1.GetData()->GetLength();
	const int pszLength = String::SZLength(psz);
	
	uint16* pszNew = StringData::CreateInstance(dataLength1 + pszLength);

	if( dataLength1 > 0 )
	{
		// Copy first part
		const int chunkCount1 = StringData::GetUsedChunkCount(dataLength1);
		Memory::Copy64(pszNew, str1.m_pszBuffer, chunkCount1);
	}
	
	// Copy second part
	// + 1 to copy the final 0 which might not be at the right place after a FastUInt32Copy
	Memory::Copy16(pszNew + dataLength1, psz, (pszLength + 1));

	String str;
	str.m_pszBuffer = pszNew;
	return str;
}

///////////////////////////////////////////////////////////////////////////////
const String operator + (const wchar* psz, const String& str2)
{
	if( psz == null || psz[0] == 0 )
		return str2;

	const int pszLength = String::SZLength(psz);
	const int dataLength2 = str2.GetData()->GetLength();

	uint16* pszNew = StringData::CreateInstance(pszLength + dataLength2);

	Memory::Copy16(pszNew, psz, pszLength);
	Memory::Copy16(pszNew + pszLength, str2.m_pszBuffer, dataLength2);

	String str;
	str.m_pszBuffer = pszNew;
	return str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::IsBinaryEqual(const wchar* psz) const
{
	const int thisLength = GetData()->GetLength();
	const int length = (psz != null) ? SZLength(psz) : 0;
	
	if( thisLength == 0 && length == 0 )
		return true;

	if( thisLength != length )
		return false;

	if( Memory::Is32BitsAligned(psz) )
	{	
		// Multiple of 4
		
		// First we compare each block
		int i = 0;
		for(; i < length - 3; i += 4)
		{
			const uint32* pBuf1 = (uint32*) (m_pszBuffer + i);
			const uint32* pBuf2 = (uint32*) (psz + i);

			if( pBuf1[0] != pBuf2[0] )
			{
				return false;
			}
		}
		
		// Then we compare the remaining bytes
		for(; i < length; ++i)
		{
			const uint16* pBuf1 = (uint16*) (m_pszBuffer + i);
			const uint16* pBuf2 = (uint16*) (psz + i);

			if( pBuf1[0] != pBuf2[0] )
			{
				return false;
			}
		}
		return true;
	}

	for(int i = 0; i < length; ++i)
	{
		const uint16* pBuf1 = (uint16*) (m_pszBuffer + i);
		const uint16* pBuf2 = (uint16*) (psz + i);

		if( pBuf1[0] != pBuf2[0] )
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::IsBinaryEqual(const String& str) const
{
	// Fast test
	const uint16* pBuf1 = m_pszBuffer;
	const uint16* pBuf2 = str.m_pszBuffer;

	if( pBuf1 == pBuf2 )
		return true;

	const int length1 = GetData()->GetLength();
	const int length2 = str.GetData()->GetLength();

	if( length1 != length2 )
		return false;
	
	// Comparison one chunk (which contains several characters) per iteration
	const int chunkCount = StringData::GetUsedChunkCount(length1);

	// Note : we do not loop up to the final uint64 because the unused chars may be
	// different from the two strings
	// Example : 
	// String1 : ABCDEFGH IJ0ZZZZZ
	// String2 : ABCDEFGH KL0WWWWW
	// The two strings are equal but the unused chars (Z and W) are different
	
	const int inc = 8 / sizeof(uint16);

	for(int i = 0; i < chunkCount - 1; ++i)
	{
		const uint64 n1 = ((const uint64*) pBuf1)[0];
		const uint64 n2 = ((const uint64*) pBuf2)[0];
		if( n1 != n2 )
		{
			return false;
		}	
		pBuf1 += inc;
		pBuf2 += inc;
	}

	// Final characters
	while( pBuf1[0] != 0 )
	{
		if( pBuf1[0] != pBuf2[0] )
		{
			return false;
		}
		pBuf1++;
		pBuf2++;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::IsBinaryEqualPartially(int startAt, const String& str) const
{
	// Fast test
	const uint16* pBuf1 = m_pszBuffer + startAt;
	const uint16* pBuf2 = str.m_pszBuffer;

	if( pBuf1 == pBuf2 )
	{
		return true;
	}

	const int length1 = GetData()->GetLength();
	const int length2 = str.GetData()->GetLength();

	if( startAt + length2 > length1 )
		return false;
	
	// TODO: Darken, please use your powerful mind to code 
	// a chunk optimized version of this one

	// Don't check \0 at end of str !
	for( int i = 0; i < length2; ++i )
	{
		if( pBuf1[0] != pBuf2[0] )
		{
			return false;
		}
		pBuf1++;
		pBuf2++;
	}

	return true;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::StartsWith(const String& str) const
{
	return IsBinaryEqualPartially(0, str);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::EndsWith(const String& str) const
{
	const int startAt = GetLength() - str.GetLength();
	return IsBinaryEqualPartially(startAt, str);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String String::Right(int count) const
{
	if( count < 0 )
	{
		count = 0;
	}

	const int length = GetData()->GetLength();
	if( count > length )
	{
		count = length;
	}

	uint16* pszNew = StringData::CreateInstance(count);
	const uint16* const pszSource = m_pszBuffer + (length - count);
	Memory::Copy16(pszNew, pszSource, count);

	String strOut;
	strOut.m_pszBuffer = pszNew;
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String String::Left(int count) const
{
	if( count < 0 )
	{
		count = 0;
	}

	const int length = GetData()->GetLength();
	if( count > length )
	{
		count = length;
	}

	uint16* pszNew = StringData::CreateInstance(count);
	const uint16* const pszSource = m_pszBuffer;
	
	Memory::Copy16(pszNew, pszSource, count);

	String strOut;
	strOut.m_pszBuffer = pszNew;
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String String::Mid(int first, int count) const
{
	// The first position can be equal to or greater than the length of the string
	// In that case you get an empty string
	if( first >= GetLength() )
	{
		return String();
	}

	if( count <= 0 )
	{
		return String();
	}

	const int length = GetData()->GetLength();
	if( first + count > length )
	{
		count = length - first;
	}

	uint16* pszNew = StringData::CreateInstance(count);
	const uint16* const pszSource = m_pszBuffer + first;
	
	Memory::Copy16(pszNew, pszSource, count);

	String strOut;
	strOut.m_pszBuffer = pszNew;
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Converts the string to an integer.
//
// [out] val : Result. Contains 0 in case of error.
// [in] cArg : Conversion option.
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::ToInt(int& val, char cArg) const
{
	val = 0;
	if( IsEmpty() )
	{
		return false;
	}
	const uint16* pszThis = GetData()->GetBuffer();
	int advance = 0;
	bool bOk = ScanType( (wchar*)pszThis, advance, cArg, &val);
	if( !bOk )
	{
		return false;
	}
	if( advance < GetData()->GetLength() )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Converts the string to a boolean.
//
// [out] val : Result. Contains false in case of error.
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::ToBool(bool& val) const
{
	int valInt = 0;
	bool ret = ToInt(valInt);
	if( ret )
	{
		val = valInt != 0;
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::ToFloat(float32& fVal, bool bStrict) const
{
	float64 fVal64 = 0;
	bool bRes = ToFloat(fVal64, bStrict);
	fVal = float32(fVal64);
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool String::ToFloat(float64& fVal, bool bStrict) const
{
	if( IsEmpty() )
		return false;

	const uint16* pszThis = GetData()->GetBuffer();
	int advance = 0;
	bool bOk = ScanType( (wchar*)pszThis, advance, 0, &fVal);
	if( !bOk )
		return false;

	if( bStrict && advance < GetData()->GetLength() )
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
StringArray String::SplitByEndlines() const
{
	StringArray astrResults;

	Int32Array anResults;
	const int length = GetData()->GetLength();
	if( length == 0 )
		return astrResults;

	const uint16* pszThis = GetData()->GetBuffer();
	
	int cutStartAt = 0; // Next string first character index
	int iCutStopAt = 0;  // Current character index

	// Prepare slot for the first string
	anResults.Add(0);

	for(; iCutStopAt < length; iCutStopAt++)
	{
		uint16 c = pszThis[iCutStopAt];
		uint16 cNext = pszThis[iCutStopAt + 1];

		if( (c == '\n') || (c == '\r' && cNext != '\n') )
		{
			// Carriage return made of only one character : \n or \r
			anResults.Add(iCutStopAt);
			
			cutStartAt = iCutStopAt + 1;
			anResults.Add(cutStartAt);

		}
		else if( c == '\r' && cNext == '\n' )
		{
			// Carriage return made of two characters : \r\n
			anResults.Add(iCutStopAt);

			iCutStopAt++; // Jump over '\n' in the loop
			cutStartAt = iCutStopAt + 1;
			anResults.Add(cutStartAt);
		}
	}

	anResults.Add(iCutStopAt);

	const int rangeCount = anResults.GetSize();
	ASSERT( (rangeCount & 1) == 0); // Should be even

	astrResults.SetSize(rangeCount / 2);
	for(int i = 0; i < rangeCount; i += 2)
	{
		int start = anResults[i];
		int stop = anResults[i + 1];

		astrResults[i / 2].SetBuffer( (wchar*)(pszThis + start), stop - start);
	}

	return astrResults;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
StringArray String::Split(int codePoint) const
{
	StringArray astrResults;

	const int length = GetData()->GetLength();
	if( length == 0 )
		return astrResults;

	uint16 first, next;
	int codeUnitCount = CodePointToCodeUnits(codePoint, first, next);
	
	if( codeUnitCount == 0 )
		return astrResults;
	
	const uint16* pszThis = GetData()->GetBuffer();
	int previousCut = 0;

	for(int i = 0; i < length; ++i)
	{
		uint16 value = pszThis[i];
		if( value == first )
		{
			// Found a unit !
			int cutIndex = i;
			if( next != 0 )
			{
				if( i < (length - 1) && pszThis[i + 1] == next )
				{
					// Ok
					++i; // Jump over to count the second code unit
				}
				else
				{
					cutIndex = -1; // Do not cut
				}
			}

			if( cutIndex >= 0 )
			{
				int pieceSize = cutIndex - previousCut;
				String strPiece = Mid(previousCut, pieceSize);
				astrResults.Add(strPiece);

				previousCut = i + 1;
			}
		}
	}

	// Get the last piece
	int cutIndex = length;
	int pieceSize = cutIndex - previousCut;
	String strPiece = Mid(previousCut, pieceSize);
	astrResults.Add(strPiece);

	return astrResults;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
StringArray String::Split(const String& str) const
{
	StringArray astrResults;

	const int length = GetData()->GetLength();
	if( length == 0 )
		return astrResults;

	int startAt = 0;
	
	while(true)
	{
		int pos = Find(str, startAt);
		if( pos < 0 )
		{
			break;
		}

		String strLeft = Mid(startAt, pos - startAt);
		astrResults.Add(strLeft);

		startAt = pos + str.GetLength();
	}

	// Get the last piece
	if( startAt <= length )
	{
		String strLast = Mid(startAt);
		astrResults.Add(strLast);
	}

	return astrResults;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ByteArray String::ToBytes(const chustd::TextEncoding& te, bool addEndingZero) const
{
	ByteArray bytes = te.StringToBytes(*this);

	if( addEndingZero )
	{
		bytes.Add(0);
	}
	return bytes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// static
String String::FromBytes(const ByteArray& bytes, const chustd::TextEncoding& te)
{
	return te.BytesToString(bytes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Finds a substring in the current string
// Returns >= 0 if found, -1 otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
int String::Find(const String& strToFind, int startAt) const
{
	ASSERT(startAt >= 0);

	const int length = GetData()->GetLength();

	if( startAt >= length )
		return -1;

	const int nToFindLength = strToFind.GetLength();
	if( nToFindLength > length || nToFindLength == 0 )
		return -1;
	
	const int lastPos = length - nToFindLength + 1;
	
	const uint16* const pszToFind = strToFind.m_pszBuffer;
	const uint16* const pszBuffer = m_pszBuffer;
	for(int i = startAt; i < lastPos; i++)
	{
		int j = 0;
		for(; j < nToFindLength; j++)
		{
			if( pszToFind[j] != pszBuffer[i + j] )
				break;
		}
		if( j == nToFindLength )
		{
			// Found !
			return i;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the index of the first occurrence of a one of the characters defined in strChars
// Returns >= 0 if found, -1 otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
int String::FindOneOf(const String& strChars, int startAt = 0) const
{
	const int length = GetData()->GetLength();
	const int charCount = strChars.GetLength();
	if( charCount == 0 )
	{
		return -1;
	}
	
	const uint16* const pszBuffer = m_pszBuffer;
	const uint16* const pszChars = strChars.m_pszBuffer;

	for(int i = startAt; i < length; ++i)
	{
		uint16 src = pszBuffer[i];
		for(int iChar = 0; iChar < charCount; ++iChar)
		{
			if( src == pszChars[iChar] )
			{
				return i;
			}
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Replaces every occurrence of strSrc with strNew
// TODO: can be optimized by removing too many memory allocations
String String::ReplaceAll(const String& strWhat, const String& strWith) const
{
	Int32Array aPositions;

	const int lengthWhat = strWhat.GetLength();
	const int lengthWith = strWith.GetLength();

	int startAt = 0;
	while(true)
	{			
		// Find the string to replace
		int foundAt = Find( strWhat, startAt);
		if( foundAt < 0 )
		{
			break;
		}
		aPositions.Add(foundAt);

		startAt = foundAt + lengthWhat;
	}
	const int replaceCount = aPositions.GetSize();
	if( replaceCount == 0 )
	{
		return *this;
	}

	const int oldLength = GetLength();
	const int newLength = GetLength() - (replaceCount * (lengthWhat - lengthWith));
	
	// Create a new StringData for our own use
	uint16* const pszNewDataBuffer = StringData::CreateInstance(newLength);
	
	uint16* pSrc = m_pszBuffer;
	uint16* pDst = pszNewDataBuffer;

	const uint16* pszWith = strWith.GetData()->GetBuffer();

	int srcStart = 0;
	for(int i = 0; i < replaceCount; ++i)
	{
		int srcStop = aPositions[i];
		int segmentSize = srcStop - srcStart;

		Memory::Copy(pDst, pSrc + srcStart, segmentSize * sizeof(uint16));
		pDst += segmentSize;

		Memory::Copy(pDst, pszWith, lengthWith * sizeof(uint16));
		pDst += lengthWith;

		srcStart = srcStop + lengthWhat;
	}

	// Copy last segment
	int segmentSize = oldLength - srcStart;
	Memory::Copy(pDst, pSrc + srcStart, segmentSize * sizeof(uint16));
	
	String strOut;
	strOut.m_pszBuffer = pszNewDataBuffer;
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns a copy of the string without both leading and trailing blank chars
String String::Trim() const
{
	const int length = GetLength();
	const uint16* pszThis = GetData()->GetBuffer();

	int iStart = 0;
	for(; iStart < length; ++iStart)
	{
		uint16 char16 = uint16(pszThis[iStart]);
		if( !CodePoint::IsWhitespace(char16) )
			break;
	}

	int iStop = length - 1;
	for(; iStop >= iStart; --iStop)
	{
		uint16 char16 = uint16(pszThis[iStop]);
		if( !CodePoint::IsWhitespace(char16) )
			break;
	}

	if( iStart == 0 && iStop == length - 1 )
	{
		// Just returns a copy
		return *this;
	}

	int newLength = (iStop - iStart) + 1;

	uint16* pszNew = StringData::CreateInstance(newLength);
	Memory::Copy16(pszNew, pszThis + iStart, newLength);

	String strOut;
	strOut.m_pszBuffer = pszNew;
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the number of code units assigned
// Returns 0 if codePoint is out of range
int String::CodePointToCodeUnits(int codePoint, uint16& utf16First, uint16& utf16Next)
{
	if( codePoint < 0x0000FFFF )
	{
		utf16First = uint16(codePoint);
		utf16Next = 0;
		return 1;
	}
	else if( codePoint <= 0x0010FFFF )
	{
		// U+10000..U+10FFFF stored in 
		// U+D800..U+DBFF (high surrogate)
		// U+DC00..U+DFFF (low surrogate)
		uint16 nHi = uint16( (codePoint - 0x10000) / 0x0400 + 0xD800);
		uint16 nLo = uint16( (codePoint - 0x10000) % 0x0400 + 0xDC00);

		utf16First = nHi;
		utf16Next = nLo;
		return 2;
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns -1 upon error
int String::GetCodePoint(int& start) const
{
	const int length = GetLength();

	int invalidCodePoint = -1;

	ASSERT(start >= 0);
	
	if( !(0 <= start && start < length) )
	{
		return invalidCodePoint;
	}

	uint16 nHi = m_pszBuffer[start];
	start += 1;

	// A surrogate ?
	if( nHi < 0xD800 || nHi > 0xDFFF )
	{
		// Nop, standard code point
		return nHi;
	}

	if( 0xDBFF < nHi )
	{
		// Error ! Low surrogate used first, this is an error
		return invalidCodePoint;
	}

	/////////////////////////////////////////////
	// Get the second surrogate (low)
	if( start >= length )
	{
		// Error ! Unexpected end of string
		return invalidCodePoint;
	}
	uint16 nLo = m_pszBuffer[start];
	start += 1;

	// A low surrogate ?
	if( !(0xDBFF < nLo && nLo <= 0xDFFF) )
	{
		// Error ! We expected a low surrogate
		return invalidCodePoint;
	}

	int codePoint = 0x10000 + ((nHi - 0xD800) << 10) + (nLo - 0xDC00);
	return codePoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Formats an int32 to a new string
// [in] val     Value to format
// [in] base    Formatting base: 0 or 'd' for decimal, 'x' or 'X' for hexadecimal, 'b' for binary
// Returns the formatted string
///////////////////////////////////////////////////////////////////////////////////////////////////
String String::FromInt(int val, char base)
{
	wchar szBuffer[40];
	if( base == 'x' )
	{
		FormatUInt32Hex( uint32(val), szBuffer, false);
	}
	else if( base == 'X' )
	{
		FormatUInt32Hex( uint32(val), szBuffer, true);
	}
	else if( base == 'b' )
	{
		FormatUInt32Bin( uint32(val), szBuffer);
	}
	else
	{
		FormatInt32(val, szBuffer);
	}
	return String(szBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Formats an int32 to a new string
// [in] val     Value to format
// [in] base    Formatting base: 0 or 'd' for decimal, 'x' or 'X' for hexadecimal, 'b' for binary
// [in] width   Minimal width of the result. -1 if not set
// [in] fill    Fill code point if minimal width not reached
// Returns the formatted string
///////////////////////////////////////////////////////////////////////////////////////////////////
String String::FromInt(int val, char base, int8 width, wchar cFill)
{
	wchar szBuffer[40];
	if( base == 'x' )
	{
		FormatUInt32Hex( uint32(val), szBuffer, width, cFill, false);
	}
	else if( base == 'X' )
	{
		FormatUInt32Hex( uint32(val), szBuffer, width, cFill, true);
	}
	else if( base == 'b' )
	{
		FormatUInt32Bin( uint32(val), szBuffer, width, cFill);
	}
	else
	{
		FormatInt32(val, szBuffer, width, cFill);
	}
	return String(szBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Formats an int64 to a new string
// [in] val     Value to format
// [in] base    Formatting base: 0 or 'd' for decimal, 'x' or 'X' for hexadecimal, 'b' for binary
// [in] width   Minimal width of the result. -1 if not set
// [in] fill    Fill code point if minimal width not reached
// Returns the formatted string
///////////////////////////////////////////////////////////////////////////////////////////////////
String String::FromInt64(int64 val, char base, int8 width, wchar fill)
{
	wchar szBuffer[72];
	if( base == 'x' )
	{
		FormatUInt64Hex( uint64(val), szBuffer, width, fill, false);
	}
	else if( base == 'X' )
	{
		FormatUInt64Hex( uint64(val), szBuffer, width, fill, true);
	}
	else if( base == 'b' )
	{
		FormatUInt64Bin( uint64(val), szBuffer, width, fill);
	}
	else
	{
		FormatInt64(val, szBuffer, width, fill);
	}
	return String(szBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String String::FromAsciiSZ(const char* psz)
{
	return String(psz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
wchar* String::GetUnsafeBuffer(int wantedLength)
{
	Unref();

	uint16* pszNew = StringData::CreateInstance(wantedLength);
	if( pszNew == null )
	{
		m_pszBuffer = StringData::GetNullInstance()->GetBuffer();
		return null;
	}
	
	m_pszBuffer = pszNew;
	return (wchar*)pszNew;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String String::InsertAt(int index, const String& str) const
{
	if( index < 0 )
	{
		ASSERT(0);
		return String();
	}

	const int length = GetLength();

	if( index > length )
	{
		ASSERT(0);
		return String();
	}

	// index == length is allowed and is equivalent to an append

	const uint16* pszThis = GetData()->GetBuffer();
		
	const uint16* pszOther = str.GetData()->GetBuffer();
	const int otherLength = str.GetLength();

	uint16* pszNew = StringData::CreateInstance(length + otherLength);
	if( pszNew == null )
	{
		return String();
	}
	
	Memory::Copy16(pszNew, pszThis, index);
	Memory::Copy16(pszNew + index, pszOther, otherLength);
	Memory::Copy16(pszNew + index + otherLength, pszThis + index, length - index);

	String strOut;
	strOut.m_pszBuffer = pszNew;
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String String::Pad(int length, int codePoint) const
{
	const int thisLength = GetLength();
	if( length <= thisLength )
		return *this;

	uint16 utf16First, utf16Next;
	int codeUnitCount = CodePointToCodeUnits(codePoint, utf16First, utf16Next);
	if( codeUnitCount == 0 )
	{
		return String(); // Failed
	}

	// TODO: should we increase length to avoid cutting a surrogate pair ?
	// TODO: should length be the code unit length or the code point length ?

	uint16* pszThis = m_pszBuffer;
	uint16* pszNew = StringData::CreateInstance(length);
	if( pszNew == null )
	{
		return String(); // Failed
	}
	
	Memory::Copy16(pszNew, pszThis, thisLength);
	if( codeUnitCount == 1 )
	{
		Memory::Set16(pszNew + thisLength, utf16First, length - thisLength);
	}
	else
	{
		for(int i = 0; i < (length - thisLength); ++i)
		{
			if( (i & 0x01) == 0 )
			{
				pszNew[length + i] = utf16First;
			}
			else
			{
				pszNew[length + i] = utf16Next;
			}
		}
	}

	String strOut;
	strOut.m_pszBuffer = pszNew;
	return strOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Unifies newlines contained in this string.
// [in] str     String to check for newlines
// [in] nt      Type of newline to use
// Returns the new string with unified newlines
///////////////////////////////////////////////////////////////////////////////////////////////////
String String::UnifyNewlines(NewlineType nt) const
{
	const int length = GetData()->GetLength();
	if( length == 0 )
	{
		return *this;
	}

	///////////////////////////////////////////////////
	// Count newlines
	enum State { Idle, DosBegin } state = Idle;
	int unixCount = 0;
	int dosCount = 0;
	for(int i = 0; i < length; ++i)
	{
		uint16 cu = m_pszBuffer[i];
		if( state == Idle )
		{
			if( cu == '\r' )
			{
				state = DosBegin;
			}
			else if( cu == '\n' )
			{
				unixCount++;
			}
		}
		else if( state == DosBegin )
		{
			if( cu == '\n' )
			{
				dosCount++;
			}
			state = Idle;
		}
	}

	int newLength = 0;
	if( nt == NT_Unix )
	{
		if( dosCount == 0 )
		{
			// Nothing to convert
			return *this;
		}
		newLength = length - dosCount;
	}
	else // NT_Dos
	{
		if( unixCount == 0 )
		{
			// Nothing to convert
			return *this;
		}
		newLength = length + unixCount;
	}

	///////////////////////////////////////////////////
	uint16* pszNew = StringData::CreateInstance(newLength);
	String ret;
	ret.m_pszBuffer = pszNew;
	int dstIndex = 0;
	
	state = Idle;

	if( nt == NT_Unix )
	{
		for(int i = 0; i < length; ++i)
		{
			uint16 cu = m_pszBuffer[i];
			if( state == Idle )
			{
				if( cu == '\r' )
				{
					state = DosBegin;
				}
				else
				{
					pszNew[dstIndex] = cu;
					dstIndex++;
				}
			}
			else // DosBegin
			{
				pszNew[dstIndex] = cu;
				dstIndex++;
				state = Idle;
			}
		}
	}
	else // NT_Dos
	{
		for(int i = 0; i < length; ++i)
		{
			uint16 cu = m_pszBuffer[i];
			if( state == Idle )
			{
				if( cu == '\r' )
				{
					state = DosBegin;
				}
				else
				{
					if( cu == '\n' )
					{
						pszNew[dstIndex] = '\r';
						dstIndex++;
					}
					pszNew[dstIndex] = cu;
					dstIndex++;
				}
			}
			else if( state == DosBegin )
			{
				if( cu == '\n' )
				{
					pszNew[dstIndex] = '\r';
					dstIndex++;
				}
				pszNew[dstIndex] = cu;
				dstIndex++;
				state = Idle;
			}
		}

	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Converts this string to UTF-8 bytes and a zero byte. No memory allocation
// is done.
//
// [out] buf    Buffer where the UTF-8 bytes are written to
// [in]  size   Size in bytes of the buffer pointer by buf
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////
bool String::ToUtf8Z(char* buf, int size) const
{
	uint8* dstBytes = reinterpret_cast<uint8*>(buf);
	int index = 0;
	for(;;)
	{
		int codePoint = GetCodePoint(index);
		if( codePoint == -1 )
		{
			break;
		}
		// Use the no-malloc version
		int seqLen = TextEncodingUtf8::InsertCodePoint(codePoint, dstBytes, size);
		if( seqLen <= 0 )
		{
			return false;
		}
		dstBytes += seqLen;
		size -= seqLen;
	}
	// Put final zero byte
	if( size < 1 )
	{
		return false;
	}
	dstBytes[0] = 0;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Builds a string from a UTF-8 buffer.
///////////////////////////////////////////////////////////////////////////////
String String::FromUtf8(const char* buf, int len)
{
	const uint8* buf8 = reinterpret_cast<const uint8*>(buf);
	return TextEncodingUtf8::StringFromBytes(buf8, len);
}

///////////////////////////////////////////////////////////////////////////////
// Builds a string from a UTF-8 buffer terminated with 0.
///////////////////////////////////////////////////////////////////////////////
String String::FromUtf8Z(const char* buf)
{
	return FromUtf8(buf, (int)strlen(buf));
}

///////////////////////////////////////////////////////////////////////////////
// Compares the binary content with another string.
//
// Returns -1 if this string is less than the other string
//         +1 if this string is greater than the other string
//          0 if the strings are equal
///////////////////////////////////////////////////////////////////////////////
int String::CompareBin(const String& other) const
{
	const String& left = *this;
	const String& right = other;

	int leftLength = left.GetLength();
	int rightLength = right.GetLength();

	int len = MIN(leftLength, rightLength);
	const wchar* p1 = left.GetPtr();
	const wchar* p2 = right.GetPtr();
	for(int i = 0; i < len; ++i)
	{
		if( p1[i] < p2[i] )
		{
			return -1;
		}
		else if( p1[i] > p2[i] )
		{
			return +1;
		}
	}
	if( leftLength < rightLength )
	{
		return -1;
	}
	if( leftLength > rightLength )
	{
		return +1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
}
