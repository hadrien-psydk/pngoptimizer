///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_DATETIME_H
#define CHUSTD_DATETIME_H

namespace chustd {\

class DateTime;

// Manages a duration between two dates
class Duration
{
public:
	void Split(int& days, int& hours, int& minutes, int& seconds, int& milliseconds) const;
	int64 GetTotalSeconds() const;

	Duration();
	Duration(int64 chuTimeStamp);
	Duration(int days, int hours, int minutes, int seconds, int milliseconds);

	void FormatFrench(class String& str) const;
	void FormatSmall(class String& str) const;

	static Duration FromSeconds(int64 seconds);

protected:
	int64 m_stamp;

	friend class DateTime;
};

// Used by the DateTime::Format method
enum class DateFormat
{
	Expanded,  // YYYY-MM-DD hh:mm:ss
	ExpandedT, // YYYY-MM-DDThh:mm:ss
	Compact    // YYYYMMDDThhmmss
};

bool operator == (const DateTime& dt1, const DateTime& dt2);

// Manages a date+time
class DateTime
{
public:
	DateTime();
	DateTime(int year, int month = 1, int day = 1,
	         int hour = 0, int minute = 0, int second = 0);

	int GetYear() const { return m_year; }
	int GetMonth() const { return m_month; }
	int GetDay() const { return m_day; }
	int GetHour() const { return m_hour; }
	int GetMinute() const { return m_minute; }
	int GetSecond() const { return m_second; }
	
	int64 GetStampInSec() const;
	int64 GetChuTimeStamp() const;

	void AddDays(int count);
	bool SetTime(int hour, int minute, int second);

	const Duration operator - (const DateTime& t2) const;
	bool operator > (const DateTime& t2) const;
	bool operator < (const DateTime& t2) const;
	bool operator >= (const DateTime& t2) const;
	bool operator <= (const DateTime& t2) const;
	
	const DateTime& operator += (Duration dur);

	static DateTime GetNow();
	static DateTime FromChuTimeStamp(int64 chuts);
	static DateTime FromUnixTimeStamp(int64 unixts, int nsec=0);

	int64 ToUnixTimeStamp(int* pnsec=nullptr) const;
	
	// Formats date and hour as YYYY-MM-DD hh:mm:ss (ISO 8601)
	String Format(DateFormat format = DateFormat::Expanded) const;

	// Formats date only as YYYY-MM-DD
	String FormatDate(DateFormat format = DateFormat::Expanded) const;

	// Formats time only as hh:mm:ss
	String FormatTime(DateFormat format = DateFormat::Expanded) const;

	// Gets date as YYYY-MM-DD hh:mm:ss
	// Works if last pars are missing, for instance : YYYY-MM 
	// Returns the number of read members
	int32 Scan(const String& strDate);

	bool Set(int year, int month = 1, int day = 1, int hour = 0, int minute = 0, int second = 0);

protected:
	int64 m_stamp;

	uint16 m_year;
	uint8 m_month;
	uint8 m_day;

	uint8 m_hour;
	uint8 m_minute;
	uint8 m_second;

	uint8 m_dummy;

protected:
	// Updates m_year, m_month, m_day, m_hour, m_minute, m_second
	void UpdateFieldsFromStamp();

	// Updates m_stamp
	void UpdateStampFromFields();

	friend bool operator == (const DateTime& dt1, const DateTime& dt2);
};

} // namespace chustd

#endif // ndef CHUSTD_DATE_H
