///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringBuilder.h"

using namespace chustd;
//////////////////////////////////////////////////////////////////////

StringBuilder::StringBuilder()
{
	m_nNext = 0;
	m_bSzBufferDone = false;
}

StringBuilder::~StringBuilder()
{

}

const StringBuilder& StringBuilder::operator+=(int codePoint)
{
	uint16 szCat[4];

	int32 codeUnitCount = String::CodePointToCodeUnits(codePoint, szCat[0], szCat[1]);
	szCat[codeUnitCount] = 0;

	if( !m_bSzBufferDone )
	{
		if( (m_nNext + codeUnitCount) < 64 )
		{
			if( codeUnitCount == 1 )
			{
				m_szBuffer[m_nNext] = szCat[0];
			}
			else
			{
				m_szBuffer[m_nNext] = szCat[0];
				m_szBuffer[m_nNext + 1] = szCat[1];
			}
			m_nNext += codeUnitCount;
			return *this;
		}

		m_bSzBufferDone = true;
	}
	
	// TODO : decide if we use uint16 or wchar for code units
	m_strBuffer = m_strBuffer + String( (wchar*) szCat, codeUnitCount);

	return *this;
}

const StringBuilder& StringBuilder::operator+=(const String& str)
{
	m_bSzBufferDone = true;
	m_strBuffer = m_strBuffer + str;

	return *this;
}

String StringBuilder::ToString() const
{
	if( !m_bSzBufferDone )
	{
		// Only chars in the fixed buffer
		return String(m_szBuffer, m_nNext);
	}
	else if( m_nNext == 0 )
	{
		// Only chars in the String buffer
		return m_strBuffer;
	}

	// Note : can optimize here by creating directly a StringData (would need to be friend with String)
	return String(m_szBuffer, m_nNext) + m_strBuffer;
}

int32 StringBuilder::GetLength() const
{
	return m_strBuffer.GetLength();
}

bool StringBuilder::IsEmpty() const
{
	if( !m_bSzBufferDone )
	{
		return (m_nNext == 0);
	}

	return m_strBuffer.IsEmpty();
}

void StringBuilder::Empty()
{
	m_strBuffer.Empty();
	m_nNext = 0;
	m_bSzBufferDone = false;
}