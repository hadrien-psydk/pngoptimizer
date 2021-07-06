///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResourceVersion.h"

//////////////////////////////////////////////////////////////////////
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////
#pragma comment(lib, "Version.lib")

using namespace chustd;
//////////////////////////////////////////////////////////////////////
chustd::String ResourceVersion::GetVersion()
{
	int nA, nB, nC, nD;
	nA = nB = nC = nD = 0;

	if( GetVersion(nA, nB, nC, nD) )
	{
		String strA = String::FromInt(nA);
		String strB = String::FromInt(nB);
		String strC = String::FromInt(nC);
		String strD = String::FromInt(nD);

		if( nD == 0 )
		{
			if( nC == 0 )
			{
				return strA + L"." + strB;
			}
			return strA + L"." + strB + L"." + strC;
		}
		return strA + L"." + strB + L"." + strC + L"." + strD;
	}

	return String();
}

chustd::String ResourceVersion::GetVersion(const chustd::String& filePath)
{
	chustd::String str;

	int nA, nB, nC, nD;
	nA = nB = nC = nD = 0;

	if( GetVersion(filePath, nA, nB, nC, nD) )
	{
		String strA = String::FromInt(nA);
		String strB = String::FromInt(nB);
		String strC = String::FromInt(nC);
		String strD = String::FromInt(nD);

		if( nD == 0 )
		{
			if( nC == 0 )
			{
				return strA + L"." + strB;
			}
			return strA + L"." + strB + L"." + strC;
		}
		return strA + L"." + strB + L"." + strC + L"." + strD;
	}

	return String();
}


chustd::String ResourceVersion::GetVersionFull()
{
	chustd::String str;

	int nA, nB, nC, nD;
	nA = nB = nC = nD = 0;

	if( GetVersion(nA, nB, nC, nD) )
	{
		String strA = String::FromInt(nA);
		String strB = String::FromInt(nB);
		String strC = String::FromInt(nC);
		String strD = String::FromInt(nD);
		return strA + L"." + strB + L"." + strC + L"." + strD;
	}

	return str;
}

chustd::String ResourceVersion::GetVersionFullComa()
{
	chustd::String str;

	int nA, nB, nC, nD;
	nA = nB = nC = nD = 0;

	if( GetVersion(nA, nB, nC, nD) )
	{
		String strA = String::FromInt(nA);
		String strB = String::FromInt(nB);
		String strC = String::FromInt(nC);
		String strD = String::FromInt(nD);
		return strA + L"." + strB + L"." + strC + L"." + strD;
	}

	return str;
}

chustd::String ResourceVersion::GetVersionFullComa(const chustd::String& filePath)
{
	chustd::String str;

	int nA, nB, nC, nD;
	nA = nB = nC = nD = 0;

	if( GetVersion(filePath, nA, nB, nC, nD) )
	{
		String strA = String::FromInt(nA);
		String strB = String::FromInt(nB);
		String strC = String::FromInt(nC);
		String strD = String::FromInt(nD);
		return strA + L"," + strB + L"," + strC + L"," + strD;
	}

	return str;
}

bool ResourceVersion::GetVersion(int& nA, int& nB, int& nC, int& nD)
{
	wchar szFileName[MAX_PATH];

	DWORD dw = GetModuleFileNameW(
		nullptr,    // handle to module
		szFileName,  // file name of module
		MAX_PATH         // size of buffer
		);
	if( dw == 0 )
		return false;

	return GetVersion(szFileName, nA, nB, nC, nD);
}

bool ResourceVersion::GetVersion(const chustd::String& filePath, int& nA, int& nB, int& nC, int& nD)
{
	nA = nB = nC = nD = 0;

	wchar szFileName[260];
	chustd::Memory::Copy(szFileName, filePath.GetBuffer(), chustd::Math::Max(260, filePath.GetLength() + 1));

	DWORD dwFVISize = GetFileVersionInfoSizeW(
		szFileName,  // file name
		0      // set to zero
		);
	if( dwFVISize == 0 )
		return false;

	LPVOID pData = new __int8[dwFVISize];

	BOOL bOk = GetFileVersionInfoW(
		szFileName,  // file name
		0,         // ignored
		dwFVISize,            // size of buffer
		pData           // version information buffer
		);
	if( !bOk )
	{
		return false;
	}

	// If we were able to get the information, process it:
	char    szGetName[256];
	lstrcpyA(szGetName, "\\");
	
	LPVOID pVer = nullptr;
	UINT   nVerLength = 0;
	BOOL bRet = VerQueryValueA(pData, szGetName, &pVer, &nVerLength);
	
	chustd::String str;
	if( bRet && nVerLength && pVer )
	{
		// Replace dialog item text with version info
		VS_FIXEDFILEINFO* pFfi = (VS_FIXEDFILEINFO*) pVer;
		nA = HIWORD(pFfi->dwProductVersionMS);
		nB = LOWORD(pFfi->dwProductVersionMS);
		nC = HIWORD(pFfi->dwProductVersionLS);
		nD = LOWORD(pFfi->dwProductVersionLS);
	
		delete[] pData;

		return true;
	}
	
   	delete[] pData;

	return false;
}
