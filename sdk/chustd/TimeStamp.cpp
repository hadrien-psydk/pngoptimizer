///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TimeStamp.h"

namespace chustd {

///////////////////////////////////////////////////////////////////////////////
static const uint8 k_daysPerMonth[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

///////////////////////////////////////////////////////////////////////////////
inline bool IsLeapYear(int year)
{
	bool leapYear = (((year % 4) == 0) && !((year % 100) == 0))
	             || ((year % 400) == 0);
	return leapYear;
}

///////////////////////////////////////////////////////////////////////////////
void SplitChuTimeStamp(int64 stamp, uint16& yearOut, uint8& month, uint8& day, 
                       uint8& hour, uint8& minute, uint8& second, uint16& millisecond)
{
	int64 next = stamp;

	millisecond = uint16(next % 1000);
	next /= 1000;

	second = uint8(next % 60);
	next /= 60;

	minute = uint8(next % 60);
	next /= 60;

	hour = uint8(next % 24);
	next /= 24;

	int32 days = int32(next);

	/////////////////////////////////////////////
	int32 year = 1601;

	// Leap year ?
	// Multiple of 4 : yes
	// Multiple of 100 : no
	// Multiple of 400 : yes

	/////////////////////////////////////////////////////////////////////////////////////////////
	// The gregorian calendar repeats itself every 400 years
	// The calendar is divided into an infinite number of parts, each part is a quadcentury
	//
	// ---- QuadCenturies ----
	// Count : 146097  146097  146097  146097 ...
	// Index :    0       1       2       3
	// Year  :  1601    2001    2401    2801

	int32 daysPer400Years = 400 * 365 + 100 - 4 + 1; // 146097
	
	const int32 partIndex0 = days / daysPer400Years;
	const int32 dayIndex0 = days - partIndex0 * daysPer400Years;

	year += partIndex0 * 400;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// The quadcentury is divided into 4 parts, each part is a century
	//
	// ---- Centuries ----
	// Count : 36524  36524  36524  36525 <-- always
	// Index :   0      1      2      3

	int32 partIndex1 = dayIndex0 / 36524;
	if( partIndex1 > 3 )
	{
		// Reajust
		partIndex1 = 3;
	}
	const int32 dayIndex1 = dayIndex0 - partIndex1 * 36524;

	year += partIndex1 * 100;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// The century is divided into 25 parts, each part is a quadyear
	//
	// ---- QuadYears ----
	// Count : 1461  1461  1461... 1460 <-- regular last year of century (1700, 1800, 1900)
	// Count : 1461  1461  1461... 1461 <-- last year of quadcentury (2000, 2400, 2800)
	// Index :   0     1     2      24

	int32 partIndex2 = dayIndex1 / 1461;

	const int32 dayIndex2 = dayIndex1 - partIndex2 * 1461;

	year += partIndex2 * 4;
	
	/////////////////////////////////////////////////////////////////////////////////////////////
	// The quadyear is divided into 4 parts, each part is a year
	// 
	// ---- Years ----
	// Count : 365 365 365 366 <-- regular last quadyear (1704, 1708, 1712)
	// Count : 365 365 365 366 <-- last year of quadcentury (2000, 2400, 2800)
	// Count : 365 365 365 365 <-- last year of century and not last year of quadcentury (1700, 1800, 1900)
	// Index :  0   1   2   3

	int32 partIndex3 = dayIndex2 / 365;
	if( partIndex3 > 3 )
	{
		// Readjust
		partIndex3 = 3;
	}
	const int32 dayIndex3 = dayIndex2 - partIndex3 * 365;

	year += partIndex3;
	///////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Now that we got the year, let's compute the month
	bool leapYear = IsLeapYear(year);

	int32 sumMonth = 0;
	int32 iMonth = 0;

	for(; iMonth < 12; ++iMonth)
	{
		int32 dayCount = k_daysPerMonth[iMonth];
		if( leapYear && iMonth == 1 )
		{
			dayCount++;
		}

		if( sumMonth + dayCount > dayIndex3 )
		{
			break;
		}

		sumMonth += dayCount;
	}

	yearOut = uint16(year);
	month = uint8(1 + iMonth);
	day = uint8(1 + (dayIndex3 - sumMonth));
}

int64 MakeChuTimeStamp(uint16 year, uint8 month, uint8 day, 
                       uint8 hour, uint8 minute, uint8 second, uint16 millisecond)
{
	// The base date is 1601-01-01 00:00:00.000

	int32 yearRange = year - 1601;
	if( yearRange < 0 )
	{
		// TODO: thinks wether we stop and return an error or just use 0
		return 0;
	}

	// Leap year ?
	// Multiple of 4 : yes
	// Multiple of 100 : no
	// Multiple of 400 : yes

	int32 leapCount = yearRange / 4;
	leapCount -= yearRange / 100;
	leapCount += yearRange / 400;

	int64 stamp = yearRange;
	stamp *= 365; // 365 days
	stamp += leapCount;
		
	//////////////////////////////////////////////////////
	bool leapYear = IsLeapYear(year);

	int32 monthRange = month;
	if( ! (1 <= monthRange && monthRange <= 12) )
	{
		monthRange = 1;
	}
	monthRange -= 1;

	for(int iMonth = 0; iMonth < monthRange; ++iMonth)
	{
		uint8 dayCount = k_daysPerMonth[iMonth];
		if( iMonth == 1 && leapYear )
		{
			dayCount += 1;
		}
		stamp += dayCount;
	}
	
	/////////////////////////////////////////////////
	int32 dayRange = day;
	if( ! (1 <= dayRange && dayRange <= 31) )
	{
		dayRange = 1;
	}
	dayRange -= 1;

	stamp += dayRange;

	/////////////////////////////////////////////////
	stamp *= 24;
	stamp += hour;

	stamp *= 60;
	stamp += minute;

	stamp *= 60;
	stamp += second;

	stamp *= 1000;
	stamp += millisecond;

	return stamp;
}

} // namespace chustd