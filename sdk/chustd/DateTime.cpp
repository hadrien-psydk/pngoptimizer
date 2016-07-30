///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DateTime.h"
#include "String.h"
#include "StringBuilder.h"
#include "FormatType.h"
#include "FixArray.h"
#include "ScanType.h"
#include "TimeStamp.h"

namespace chustd {\

//////////////////////////////////////////////////////////////////////
Duration::Duration()
{
	m_stamp = 0;
}

//////////////////////////////////////////////////////////////////////
Duration::Duration(int64 chuTimeStamp) : m_stamp(chuTimeStamp)
{

}

//////////////////////////////////////////////////////////////////////
Duration::Duration(int days, int hours, int minutes, int seconds, int milliseconds)
{
	if( !(-23 <= hours && hours <= 23) )
	{
		hours = 0;
	}

	if( !(-59 <= minutes && minutes <= 59) )
	{
		minutes = 0;
	}

	if( !(-59 <= seconds && seconds <= 59) )
	{
		seconds = 0;
	}

	int64 stamp = days;

	stamp *= 24;
	stamp += hours;

	stamp *= 60;
	stamp += minutes;

	stamp *= 60;
	stamp += seconds;

	stamp *= 1000;
	stamp += milliseconds;

	m_stamp = stamp;
}

//////////////////////////////////////////////////////////////////////
// Returns the total number of seconds
int64 Duration::GetTotalSeconds() const
{
	return m_stamp / 1000;
}

//////////////////////////////////////////////////////////////////////
Duration Duration::FromSeconds(int64 seconds)
{
	Duration dur;
	dur.m_stamp = 	1000;
	dur.m_stamp *= seconds;
	return dur;
}

//////////////////////////////////////////////////////////////////////
void Duration::Split(int& days, int& hours, int& minutes, int& seconds,
                     int& milliseconds) const
{
	int64 next = m_stamp;

	milliseconds = int16(next % 1000);
	next /= 1000;

	seconds = int8(next % 60);
	next /= 60;

	minutes = int8(next % 60);
	next /= 60;

	hours = int8(next % 24);
	next /= 24;

	days = int32(next);
}

//////////////////////////////////////////////////////////////////////
void Duration::FormatFrench(String& str) const
{
	int days, hours, minutes, seconds, milliseconds;
	Split(days, hours, minutes, seconds, milliseconds);

	String strDays, strHours, strMinutes, strSeconds;
	wchar szBuffer[12];

	if( days > 0 )
	{
		if( days == 1 )
		{
			strDays = "1 jour ";
		}
		else
		{
			FormatInt32(days, szBuffer);
			strDays = String(szBuffer) + " jours";
		}
	}

	if( hours > 0 )
	{
		if( hours == 1 )
		{
			strHours = "1 heure ";
		}
		else
		{
			FormatInt32(hours, szBuffer);
			strHours = String(szBuffer) + " heures";
		}
	}

	if( minutes > 0 )
	{
		if( minutes == 1 )
		{
			strMinutes = "1 minute ";
		}
		else
		{
			FormatInt32(minutes, szBuffer);
			strMinutes = String(szBuffer) + " minutes";
		}
	}
	
	if( seconds > 0 )
	{
		if( seconds == 1 )
		{
			strSeconds = "1 seconde";
		}
		else
		{
			FormatInt32(seconds, szBuffer);
			strSeconds = String(szBuffer) + " secondes";
		}
	}

	str = strDays + strHours + strMinutes + strSeconds;
}

//////////////////////////////////////////////////////////////////////
void Duration::FormatSmall(class String& str) const
{
	int days;
	int hours;
	int minutes;
	int seconds;
	int milliseconds;
	Split(days, hours, minutes, seconds, milliseconds);

	String strDays, strHours, strMinutes, strSeconds;
	wchar szBuffer[12];

	if( days > 0 )
	{
		FormatInt32(days, szBuffer);
		strDays = String(szBuffer) + " j";
	}

	if( hours > 0 )
	{
		FormatInt32(hours, szBuffer);
		strHours = String(szBuffer) + " h";
	}

	if( minutes > 0 )
	{
		FormatInt32(minutes, szBuffer);
		strMinutes = String(szBuffer) + " m";
	}
	
	if( seconds > 0 )
	{
		FormatInt32(seconds, szBuffer);
		strSeconds = String(szBuffer) + " s";
	}

	str = strDays + strHours + strMinutes + strSeconds;
}

//////////////////////////////////////////////////////////////////////
DateTime::DateTime()
{
	m_year = 0;
	m_month = 0;
	m_day = 0;
	m_hour = 0;
	m_minute = 0;
	m_second = 0;
	m_stamp = 0;
}

//////////////////////////////////////////////////////////////////////
DateTime::DateTime(int year, int month, int day,
                   int hour, int minute, int second)
{
	if( !Set(year, month, day, hour, minute, second) )
	{
		m_year = 0;
		m_month = 0;
		m_day = 0;
		m_hour = 0;
		m_minute = 0;
		m_second = 0;
		m_stamp = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// static
DateTime DateTime::GetNow()
{
#if defined(_WIN32)
	SYSTEMTIME st;
	::GetLocalTime(&st); // OS Dependent
	
	// The FILETIME structure is a 64-bit value representing the number 
	// of 100-nanosecond intervals since January 1, 1601.
	FILETIME ft;
	::SystemTimeToFileTime(&st, &ft);

	int64 stamp = ft.dwHighDateTime;
	stamp <<= 32;
	stamp |= ft.dwLowDateTime;

	DateTime date;
	date.m_stamp = stamp / 10000;
	date.UpdateFieldsFromStamp();
	return date;

#elif defined(__linux__)
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return FromUnixTimeStamp(tv.tv_sec, tv.tv_usec*1000);

#endif
}

//////////////////////////////////////////////////////////////////////
void DateTime::UpdateFieldsFromStamp()
{
	uint16 millisecond = 0;
	chustd::SplitChuTimeStamp(m_stamp, m_year, m_month, m_day, m_hour, m_minute, m_second, millisecond);
}

//////////////////////////////////////////////////////////////////////
// This function computes a timestamp from the splitted fields
void DateTime::UpdateStampFromFields()
{
	uint16 millisecond = 0;
	m_stamp = chustd::MakeChuTimeStamp(m_year, m_month, m_day, m_hour, m_minute, m_second, millisecond);
}

//////////////////////////////////////////////////////////////////////
bool DateTime::Set(int year, int month, int day, int hour, int minute, int second)
{
	if( !(0 <= second && second <= 59) )
	{
		return false;
	}
	if( !(0 <= minute && minute <= 59) )
	{
		return false;
	}
	if( !(0 <= hour && hour <= 23) )
	{
		return false;
	}

	m_year = uint16(year);
	m_month = uint8(month);
	m_day = uint8(day);
	m_hour = uint8(hour);
	m_minute = uint8(minute);
	m_second = uint8(second);

	UpdateStampFromFields();
	return true;
}

//////////////////////////////////////////////////////////////////////
// Formats date only as YYYY-MM-DD (ISO 8601)
String DateTime::FormatDate(DateFormat format) const
{
	StringBuilder sbFinal;
	wchar szBuffer[12];

	FormatInt32(m_year, szBuffer, 4, '0');
	sbFinal += szBuffer;

	if( format != DateFormat::Compact )
	{
		sbFinal += "-";
	}
	FormatInt32(m_month, szBuffer, 2, '0');
	sbFinal += szBuffer;

	if( format != DateFormat::Compact )
	{
		sbFinal += "-";
	}
	FormatInt32(m_day, szBuffer, 2, '0');
	sbFinal += szBuffer;

	return sbFinal.ToString();
}

//////////////////////////////////////////////////////////////////////
String DateTime::FormatTime(DateFormat format) const
{
	StringBuilder sbFinal;
	wchar szBuffer[12];

	FormatInt32(m_hour, szBuffer, 2, '0');
	sbFinal += szBuffer;

	if( format != DateFormat::Compact )
	{
		sbFinal += ":";
	}
	FormatInt32(m_minute, szBuffer, 2, '0');
	sbFinal += szBuffer;


	if( format != DateFormat::Compact )
	{
		sbFinal += ":";
	}
	FormatInt32(m_second, szBuffer, 2, '0');
	sbFinal += szBuffer;

	return sbFinal.ToString();
}

//////////////////////////////////////////////////////////////////////
String DateTime::Format(DateFormat format) const
{
	String strDate = FormatDate(format);
	String strTime = FormatTime(format);

	if( format == DateFormat::ExpandedT || format == DateFormat::Compact )
	{
		return strDate + "T" + strTime;
	}

	return strDate + " " + strTime;
}

//////////////////////////////////////////////////////////////////////
// Gets date as YYYY-MM-DD hh:mm:ss
// Works if last parts are missing, for instance : YYYY-MM
// Returns the number of read members
int32 DateTime::Scan(const String& strDate)
{
	//int count = strDate.Scan("%?-%?-%? %?:%?:%?", m_year, m_month, m_day, m_hour, m_minute, m_second);
	
	const wchar* psz = strDate.GetBuffer();
	int32 advance = 0;
	
	FixArray<bool, 12> abParseStatus;

	abParseStatus[0] = ScanType(psz, advance, L' ', &m_year);
	abParseStatus[1] = psz[advance] == L'-';
	advance++;
	
	abParseStatus[2] = ScanType(psz, advance, L' ', &m_month);
	abParseStatus[3] = psz[advance] == L'-';
	advance++;

	abParseStatus[4] = ScanType(psz, advance, L' ', &m_day);
	abParseStatus[5] = psz[advance] == L' ';
	advance++;

	abParseStatus[6] = ScanType(psz, advance, L' ', &m_hour);
	abParseStatus[7] = psz[advance] == L':';
	advance++;

	abParseStatus[8] = ScanType(psz, advance, L' ', &m_minute);
	abParseStatus[9] = psz[advance] == L':';
	advance++;

	abParseStatus[10] = ScanType(psz, advance, L' ', &m_second);
	abParseStatus[11] = true;
	
	int count = 0;
	for(int i = 0; i < abParseStatus.GetSize(); i += 2)
	{
		if( !abParseStatus[i] )
			break;

		count++;
	}

	/////////////////////////////////////////////////////////////////
	// Check fields integrity
	if( count >= 6 && !( m_second <= 59) )
	{
		count = 5;
	}
	if( count >= 5 && !( m_minute <= 59) )
	{
		count = 4;
	}
	if( count >= 4 && !( m_hour <= 23) )
	{
		// Just take the year, the month and the day as the hour is invalid
		count = 3;
	}
	if( count >= 3 && !( 1 <= m_day && m_day <= 31) )
	{
		// Just take the year and the month as the day is invalid
		count = 2;
	}
	if( count >= 2 && !( 1 <= m_month && m_month <= 12) )
	{
		// Just take the year as the month is invalid
		count = 1;
	}
	/////////////////////////////////////////////////////////////////

	uint8* const apn[] = { nullptr, &m_month, &m_day, &m_hour, &m_minute, &m_second };
	int32 iClear = count;

	for(; iClear < 3; ++iClear)
	{
		apn[iClear][0] = 1;
	}

	for(; iClear < 6; ++iClear)
	{
		apn[iClear][0] = 0;
	}

	UpdateStampFromFields();

	return count;
}

//////////////////////////////////////////////////////////////////////
void DateTime::AddDays(int count)
{
	int64 oneDay = 24 * 60 * 60;
	oneDay *= 1000;
	m_stamp += count * oneDay;

	UpdateFieldsFromStamp();
}

//////////////////////////////////////////////////////////////////////
bool DateTime::SetTime(int hour, int minute, int second)
{
	if( !(0 <= hour && hour <= 23) )
	{
		return false;
	}

	if( !(0 <= minute && minute <= 59) )
	{
		return false;
	}

	if( !(0 <= second && second <= 59) )
	{
		return false;
	}

	m_hour = uint8(hour);
	m_minute = uint8(minute);
	m_second = uint8(second);

	UpdateStampFromFields();
	return false;
}

//////////////////////////////////////////////////////////////////////
const Duration DateTime::operator - (const DateTime& t2) const
{
	Duration dur;
	dur.m_stamp = m_stamp - t2.m_stamp;

	return dur;
}

//////////////////////////////////////////////////////////////////////
bool DateTime::operator > (const DateTime& t2) const
{
	return m_stamp > t2.m_stamp;
}

//////////////////////////////////////////////////////////////////////
bool DateTime::operator < (const DateTime& t2) const
{
	return m_stamp < t2.m_stamp;
}

//////////////////////////////////////////////////////////////////////
bool DateTime::operator >= (const DateTime& t2) const
{
	return m_stamp >= t2.m_stamp;
}

//////////////////////////////////////////////////////////////////////
bool DateTime::operator <= (const DateTime& t2) const
{
	return m_stamp <= t2.m_stamp;
}

//////////////////////////////////////////////////////////////////////
// Returns the time stamp in seconds
int64 DateTime::GetStampInSec() const
{
	return m_stamp / 1000;
}

//////////////////////////////////////////////////////////////////////
const DateTime& DateTime::operator += (Duration dur)
{
	m_stamp += dur.m_stamp;
	UpdateFieldsFromStamp();

	return *this;
}

//////////////////////////////////////////////////////////////////////
DateTime DateTime::FromUnixTimeStamp(int64 unixts, int nsec)
{
	int msec = nsec / 1000000;
	// 0 == 1970-01-01
	int64 chuts = unixts;
	chuts *= 1000;
	chuts += msec;

	DateTime date;
	date.Set(1970);
	date.m_stamp += chuts;

	date.UpdateFieldsFromStamp();
	return date;
}

//////////////////////////////////////////////////////////////////////
int64 DateTime::ToUnixTimeStamp(int* pnsec) const
{
	DateTime dateBase;
	dateBase.Set(1970);

	int64 dif = m_stamp - dateBase.m_stamp;
	if( pnsec )
	{
		*pnsec = (dif % 1000) * 1000000;
	}
	dif /= 1000;

	if( dif < 0 )
		dif = 0;

	return dif;
}

//////////////////////////////////////////////////////////////////////
int64 DateTime::GetChuTimeStamp() const
{
	return m_stamp;
}

//////////////////////////////////////////////////////////////////////
DateTime DateTime::FromChuTimeStamp(int64 chuts)
{
	DateTime date;
	date.m_stamp = chuts;
	date.UpdateFieldsFromStamp();
	return date;
}

//////////////////////////////////////////////////////////////////////
bool operator == (const DateTime& dt1, const DateTime& dt2)
{
	return dt1.m_stamp == dt2.m_stamp;
}

//////////////////////////////////////////////////////////////////////
}
