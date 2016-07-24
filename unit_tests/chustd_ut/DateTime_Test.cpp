#include "stdafx.h"

int64 OS_GetEpochOffset()
{
#if defined(_WIN32)
	return 0; // Windows starts in 1601 too

#elif defined(__linux__)
	struct tm tm;
	tm.tm_sec = 0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	tm.tm_mday = 1;
	tm.tm_mon = 0;
	tm.tm_year = 1601 - 1900;
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_isdst = 0;
	tm.tm_zone = nullptr;
	tm.tm_gmtoff = 0;
	int64 offset = timegm(&tm);
	return offset;
#else
	#error oops
#endif
}

int64 OS_DateFieldsToChuStamp(int year, int month, int day)
{
	int64 chuts = 0;

#if defined(_WIN32)
	SYSTEMTIME st;
	st.wYear = (uint16) year;
	st.wMonth = (uint16) month;
	st.wDayOfWeek = 0; // Ignored by SystemTimeToFileTime
	st.wDay = (uint16) day;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0; // TODO: could be cool to have this

	FILETIME ft;
	BOOL bOk = SystemTimeToFileTime(&st, &ft);
	
	chuts = ft.dwHighDateTime;
	chuts <<= 32;
	chuts |= ft.dwLowDateTime;
	chuts /= 10000;

#elif defined(__linux__)
	struct tm tm;
	tm.tm_sec = 0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	tm.tm_mday = day;
	tm.tm_mon = month - 1;
	tm.tm_year = year - 1900;
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_isdst = 0;
	tm.tm_zone = nullptr;
	tm.tm_gmtoff = 0;
	time_t unixts = timegm(&tm);
	chuts = unixts - OS_GetEpochOffset();
	chuts *= 1000;
#endif

	return chuts;
}

// Check conversion from year-month-day to timestamp
// Chu timestamps are int64 numbers representing the number of milliseconds since 1601-01-01 00:00:00.000
// We use the operating system date functions (and assume they are correct)
// to check that chu functions are ok.
TEST(DateTime, FieldsToChuStamp)
{
	DateTime date;
	for(int16 iYear = 1601; iYear <= 2100; ++iYear)
	{
		for(uint8 iMonth = 1; iMonth <= 12; ++iMonth)
		{
			for(uint8 iDay = 1; iDay < 5; ++iDay)
			{
				date.Set(iYear, iMonth, iDay);
				int64 chuTimeStamp = date.GetChuTimeStamp();
				int64 osChuTimeStamp = OS_DateFieldsToChuStamp(iYear, iMonth, iDay);

				ASSERT_TRUE( chuTimeStamp == osChuTimeStamp );
			}
		}
	}
}


void OS_Date_Split(int64 chuts, int& year, int& month, int& day)
{
#if defined(_WIN32)
	int hour, minute, second;

	chuts *= 10000;

	FILETIME ft;
	ft.dwHighDateTime = DWORD(chuts >> 32);
	ft.dwLowDateTime = DWORD(chuts & 0x00000000ffffffff);

	SYSTEMTIME st;
	if( !FileTimeToSystemTime(&ft, &st))
	{
		year = 0;
		month = 0;
		day = 0;
		hour = 0;
		minute = 0;
		second = 0;
	}
	else
	{
		year = int16( st.wYear);
		month = uint8( st.wMonth); // January = 1
		day = uint8( st.wDay);
		hour = uint8( st.wHour);
		minute = uint8( st.wMinute);
		second = uint8( st.wSecond);
	}

#elif defined(__linux__)
	time_t unixts = chuts/1000;
	unixts += OS_GetEpochOffset();
	struct tm tm = { 0 };
	gmtime_r(&unixts, &tm);

	year = tm.tm_year + 1900;
	month = tm.tm_mon + 1;
	day = tm.tm_mday;

#else
	#error oops
#endif
}

TEST(DateTime, ChuStampToFields)
{
	// 146097 = DaysPer400Years
	int64 oneDay = 24 * 60 * 60 * 1000;
	int64 chutsMax = 2 * 146097;
	chutsMax *= oneDay;

	int64 chutsMin = 146097;
	chutsMin *= oneDay;
	chutsMin -= oneDay;

	chutsMin = oneDay * 365;

	for(int64 iStamp = 0; iStamp <= chutsMax; iStamp += oneDay)
	{
		DateTime date = DateTime::FromChuTimeStamp(iStamp);

		int year = date.GetYear();
		int month = date.GetMonth();
		int day = date.GetDay();

		int32 yearOS, monthOS, dayOS;
		OS_Date_Split(iStamp, yearOS, monthOS, dayOS);

		//Console::WriteLine( String::FromInt(year) + "-" + String::FromInt(month) + "-" + String::FromInt(day));

		ASSERT_TRUE(year == yearOS);
		ASSERT_TRUE(month == monthOS);
		ASSERT_TRUE(day == dayOS);
	}
}
