///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Registry.h"

//////////////////////////////////////////////////////////////////////
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////

Registry::Registry()
{
	m_hKey = NULL;
}

Registry::~Registry()
{
	Close();
}


bool Registry::Open(HKEY hKeyRoot, const chustd::String& strPath, bool bCreateIfDoesntExist)
{
	DWORD nOptions = 0;
	REGSAM samDesired = KEY_ALL_ACCESS;

	LONG nRet = RegOpenKeyExW(
		hKeyRoot,        // handle to open key
		strPath.GetBuffer(), // name of subkey to open
		nOptions,
		samDesired,
		&m_hKey   // handle to open key
		);

	if( nRet != ERROR_SUCCESS )
	{
		if( bCreateIfDoesntExist )
		{
			// La création est peut-être nécessaire
			DWORD dw;
			nRet = RegCreateKeyExW(hKeyRoot, strPath.GetBuffer(), 0L, NULL, REG_OPTION_NON_VOLATILE, 
				KEY_ALL_ACCESS, NULL, &m_hKey, &dw);
		
			if( nRet != ERROR_SUCCESS )
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		if( m_hKey == NULL )
			return false;
	}

	return true;
}

void Registry::Close()
{
	if( m_hKey != NULL )
	{
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
}


int32 Registry::ReadInt32(const chustd::String& strValueNameW, int32 nDefault)
{
	if( !m_hKey )
		return nDefault;

	DWORD dwType = REG_DWORD;
	DWORD dwSize = 4;
	DWORD nDest;

	LONG nRet = RegQueryValueExW(m_hKey, strValueNameW.GetBuffer(), NULL, &dwType, (BYTE*) &nDest, &dwSize);
	if( nRet == ERROR_SUCCESS )
	{
		return nDest;
	}

	return nDefault;
}

bool Registry::WriteInt32(const chustd::String& strValueName, int32 nValue)
{
	if( !m_hKey )
		return false;

	LONG nRet = RegSetValueExW(m_hKey, strValueName.GetBuffer(), 0, REG_DWORD, (BYTE*)&nValue, 4);
	return nRet == ERROR_SUCCESS;
}

bool Registry::ReadBool(const chustd::String& strValueName, bool bDefault)
{
	if( !m_hKey )
		return bDefault;

	DWORD dwType;
	DWORD dwSize = 1;
	BYTE  nDest;

	LONG nRet = RegQueryValueExW(m_hKey, strValueName.GetBuffer(), NULL, &dwType, (BYTE*) &nDest, &dwSize);

	if( nRet == ERROR_SUCCESS )
	{
		return (nDest != 0);
	}

	return bDefault;
}

bool Registry::WriteBool(const chustd::String& strValueName, bool bValue)
{
	if( !m_hKey )
		return false;

	const BYTE nValue = bValue ? 1 : 0;

	LONG nRet = RegSetValueExW(m_hKey, strValueName.GetBuffer(), 0, REG_BINARY, &nValue, 1);
	return nRet == ERROR_SUCCESS;
}

chustd::String Registry::ReadString(const chustd::String& strValueNameW, const chustd::String& strDefault)
{
	if( !m_hKey )
		return strDefault;

	chustd::String strW;

	DWORD dwType = REG_SZ;
	DWORD dwSize = 0;

	// Détermine la taille des données en octet
	LONG nRet = RegQueryValueExW(m_hKey, strValueNameW.GetBuffer(), NULL, &dwType, NULL, &dwSize);
	if( nRet != ERROR_SUCCESS )
	{
		return strDefault;
	}

	const int nStringLength = chustd::Math::DivCeil(dwSize, 2) - 1;
	
	nRet = RegQueryValueExW(m_hKey, strValueNameW.GetBuffer(), NULL, 
		&dwType, (LPBYTE) strW.GetUnsafeBuffer(nStringLength), &dwSize);
	if( nRet != ERROR_SUCCESS )
	{
		return strDefault;
	}

	return strW;
}

bool Registry::WriteString(const chustd::String& strValueNameW, const chustd::String& strValue)
{
	if( !m_hKey )
		return false;

	int nStringBufferSize = (strValue.GetLength() + 1) * 2;

	LONG nRet = RegSetValueExW(m_hKey, strValueNameW.GetBuffer(), 0, REG_SZ, 
		(LPBYTE) strValue.GetBuffer(), nStringBufferSize);

	return nRet == ERROR_SUCCESS;
}


chustd::StringArray Registry::GetKeyNamesW(LPCSTR pszJoker)
{	
	chustd::StringArray astr;

	wchar szBuffer[256];
	DWORD nBufferSize = 0;
	FILETIME ftLastWriteTime;

	chustd::String strJoker = chustd::String::FromAsciiSZ(pszJoker);

	DWORD index = 0;
	for(;;)
	{
		nBufferSize = sizeof(szBuffer) / sizeof(szBuffer[0]);

		LONG nEnumRet = RegEnumKeyExW(
			m_hKey,  // handle to key to enumerate
			index,              // subkey index
			szBuffer,              // subkey name
			&nBufferSize,            // size of subkey buffer
			NULL,         // reserved
			NULL,             // class string buffer
			NULL,           // size of class string buffer
			&ftLastWriteTime // last write time
		);
		
		if( nEnumRet == ERROR_SUCCESS )
		{
			chustd::String strName = szBuffer;
			if( StringMatch(strName, strJoker) )
			{
				astr.Add(strName);	
			}
		}
		else
		{
			break;
		}
		index++;
	}
	return astr;
}

// a*b*c
bool Registry::StringMatch(const chustd::String& strTest, const chustd::String& strJoker)
{
	chustd::StringArray astrJokerTokens = strJoker.Split(L'*');

	const int nTokenCount = astrJokerTokens.GetSize();
	if( nTokenCount == 0 )
		return false;

	int32 nFoundAt = 0;
	int32 nStartAt = 0;
	for(int iToken = 0; iToken < nTokenCount; ++iToken)
	{
		chustd::String strToken = astrJokerTokens[iToken];
		if( strToken.IsEmpty() )
		{
			// Possible en début et fin
			continue;
		}

		nFoundAt = strTest.Find(strToken, nStartAt);
		if( nFoundAt < 0 )
		{
			return false;
		}

		nStartAt = nFoundAt + strToken.GetLength();
	}
	
	return true;
}


bool Registry::Flush()
{
	if( !m_hKey )
		return false;

	LONG nRet = RegFlushKey(m_hKey);
	return nRet == ERROR_SUCCESS;
}
