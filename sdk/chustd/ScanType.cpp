///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScanType.h"

#include "String.h"
#include "CodePoint.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
namespace chustd {

/////////////////////////////////////////////////////////////////////////////////////////////////
// ScanType for int32
bool IsBaseDigit(wchar c, int base, int& digit)
{
	if( base > 16 )
		return false;

	if( base <= 10 )
	{
		const wchar maxDigit = wchar('0' + (base - 1));
		if( '0' <= c && c <= maxDigit )
		{
			digit = c - '0';
			return true;
		}
		return false;
	}

	if( '0' <= c && c <= '9' )
	{
		digit = c - '0';
		return true;
	}

	if( 'a' <= c && c <= 'f' )
	{
		digit = (c - 'a') + 10;
		return true;
	}

	if( 'A' <= c && c <= 'F' )
	{
		digit = (c - 'A') + 10;
		return true;
	}
	return false;
}

bool ScanType(const wchar* psz, int& advance, char cScanArg, int32* pResult)
{
	const wchar* pszStart = psz + advance;
	
	int base = 10;
	if( cScanArg == 'x' || cScanArg == 'X' )
	{
		base = 16;
	}
	else if( cScanArg == 'b' || cScanArg == 'B' )
	{
		base = 2;
	}

	enum { stateWhitespace, stateSignOrDigit, stateDigit };
	int state = stateWhitespace;

	int32 result = 0;
	bool bPositive = true;
	int32 digitCount = 0;

	int32 iChar = 0;
	while(true)
	{
		wchar c = pszStart[iChar];
		if( state == stateWhitespace )
		{
			if( !CodePoint::IsWhitespace(c) )
			{
				state = stateSignOrDigit;
			}
		}

		if( state == stateSignOrDigit )
		{
			if( c == '+' )
			{
				// Ok
			}
			else if( c == '-' || c == 0x2212 )
			{
				bPositive = false;
			}
			else
			{
				state = stateDigit;
			}
		}

		if( state == stateDigit )
		{
			int digit;
			if( IsBaseDigit(c, base, digit) )
			{
				result = result * base + digit;
				digitCount++;
			}
			else
			{
				// End of parsing
				break;
			}
		}
		iChar++;
	}

	if( digitCount == 0 )
	{
		// Error
		return false;
	}
	
	if( !bPositive )
	{
		result = -result;
	}

	advance += iChar;
	*pResult = result;

	return true;
}

// ScanType for int16
bool ScanType(const wchar* psz, int& advance, char cScanArg, int16* pResult)
{
	// Use the int32 version of ScanType
	int32 preResult = 0;
	if( !ScanType(psz, advance, cScanArg, &preResult) )
		return false;

	*pResult = int16(preResult);
	return true;
}

// ScanType for int8
bool ScanType(const wchar* psz, int& advance, char cScanArg, int8* pResult)
{
	// Use the int32 version of ScanType
	int32 preResult = 0;
	if( !ScanType(psz, advance, cScanArg, &preResult) )
		return false;

	*pResult = int8(preResult);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// ScanType for uint32
bool ScanType(const wchar* psz, int& advance, char cScanArg, uint32* pResult)
{
	const wchar* pszStart = psz + advance;
	
	int32 base = 10;
	if( cScanArg == 'x' || cScanArg == 'X' )
	{
		base = 16;
	}
	else if( cScanArg == 'b' || cScanArg == 'B' )
	{
		base = 2;
	}

	enum { stateWhitespace, statePlusOrDigit, stateDigit };
	int state = stateWhitespace;

	int32 result = 0;
	int32 digitCount = 0;

	int32 iChar = 0;
	while(true)
	{
		wchar c = pszStart[iChar];
		if( state == stateWhitespace )
		{
			if( !CodePoint::IsWhitespace(c) )
			{
				state = statePlusOrDigit;
			}
		}

		if( state == statePlusOrDigit )
		{
			if( c == '+' )
			{
				// Ok
			}
			else
			{
				state = stateDigit;
			}
		}

		if( state == stateDigit )
		{
			int digit;
			if( IsBaseDigit(c, base, digit) )
			{
				result = result * base + digit;
				digitCount++;
			}
			else
			{
				// End of parsing
				break;
			}
		}
		iChar++;
	}

	if( digitCount == 0 )
	{
		// Error
		return false;
	}
	
	advance += iChar;
	*pResult = result;

	return true;
}

// ScanType for uint16
bool ScanType(const wchar* psz, int& advance, char cScanArg, uint16* pResult)
{
	// Use the uint32 version of ScanType
	uint32 preResult = 0;
	if( !ScanType(psz, advance, cScanArg, &preResult) )
		return false;

	*pResult = uint16(preResult);
	return true;
}

// ScanType for uint8
bool ScanType(const wchar* psz, int& advance, char cScanArg, uint8* pResult)
{
	// Use the uint32 version of ScanType
	uint32 preResult = 0;
	if( !ScanType(psz, advance, cScanArg, &preResult) )
		return false;

	*pResult = uint8(preResult);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// ScanType for float 64
// TODO: Lame, lame code, we need something better here
bool ScanType(const wchar* psz, int& advance, char /*cScanArg*/, float64* pResult)
{
	int32 intPart = 0;
	if( !ScanType(psz, advance, 0, &intPart) )
		return false;
		
	float64 fFrac = 0;
	if( psz[advance] == L'.' )
	{
		uint32 fracPart = 0;
		int32 fracPartLength = 0;

		advance++;
		fracPartLength = advance;

		if( !ScanType(psz, advance, 0, &fracPart) )
			return false;

		fracPartLength = advance - fracPartLength;

		int32 fracDiv = 1;
		for(int i = 0; i < fracPartLength; ++i)
		{
			fracDiv *= 10;
		}
		fFrac = float64(fracPart);
		fFrac /= float64(fracDiv);
	}
		
	float64 fResult = float64(intPart);
	fResult += fFrac;

	*pResult = fResult;

	return true;
}

// ScanType for float32
bool ScanType(const wchar* psz, int& advance, char cScanArg, float32* pResult)
{
	// Use the double version of ScanType
	float64 dResult = 0;
	if( !ScanType(psz, advance, cScanArg, &dResult) )
		return false;

	*pResult = float32(dResult);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ScanType for bool
bool ScanType(const wchar* psz, int& advance, char cScanArg, bool* pResult)
{
	// Use the uint8 version of ScanType
	uint8 preResult = 0;
	if( !ScanType(psz, advance, cScanArg, &preResult) )
		return false;

	*pResult = preResult == 0 ? false : true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanType(const wchar* psz, int& advance, char /*cScanArg*/, String* pResult)
{
	int32 iChar = advance;
	while(true)
	{
		wchar c = psz[iChar];
		if( c == 0 || c  == ' ' || c == '\t' || c == '\r' || c == '\n' )
			break;
		++iChar;
	}

	const int32 charCount = iChar - advance;
	pResult->SetBuffer(psz + advance, charCount);

	advance += charCount;

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace chustd

