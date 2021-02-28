///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CppExtension.h"
#include "FormatType.h"
#include "Array.h"
#include "Memory.h"
#include "File.h"
#include "Console.h"

void CheckPlatformCompatibility()
{
	// Check type sizes
	if( sizeof(int16) != 2 )
	{
		ASSERTMSG(0, "Compatibility error: int16 size is not 16 bits!!!\n");
	}

	if( sizeof(int32) != 4 )
	{
		ASSERTMSG(0, "Compatibility error: int32 size is not 32 bits!!!\n");
	}

	// int64 is a bit weird so we do not test it here

	if( sizeof(float32) != 4 )
	{
		ASSERTMSG(0, "Compatibility error: float32 size is not 32 bits!!!\n");
	}

	if( sizeof(float64) != 8 )
	{
		ASSERTMSG(0, "Compatibility error: float64 size is not 64 bits!!!\n");
	}

	if( sizeof(wchar) != 2 )
	{
		ASSERTMSG(0, "Compatibility error: wchar size is not 16 bits!!!\n");
	}

	// Check endian mode
	bool bLittleEndian = false;
	union { uint8 bytes[4]; uint32 val32; } mixed;
	mixed.val32 = 0x12345678;
	if( mixed.bytes[0] != 0x12 )
	{
		// The CPU is little endian (yuck ! :-6)
		bLittleEndian = true;
	}
	if( chustd::k_ePlatformByteOrder != chustd::boLittleEndian && bLittleEndian )
	{
		ASSERTMSG(0, "Compatibility error: bad endian mode!\n"
			"comp_bLittleEndian value in Master.h is not correct!!!\n");
	}
}

#undef double
#undef float

#ifdef WIN32
#include <windows.h>
#pragma comment(lib, "user32.lib") // Used for MessageBoxA
#endif // WIN32

void ChustdSzCat(char* pszDst, const char* pszSrc, int sizeofBuffer)
{
	if( pszSrc == nullptr )
	{
		return;
	}

	int i = 0;
	for(; i < sizeofBuffer - 1; ++i)
	{
		if( pszDst[i] == 0 )
			break;
	}

	for(; i < sizeofBuffer - 1; ++i)
	{
		const char& c = pszSrc[0];
		pszDst[i] = c;
		if( c == 0 )
			break;
		pszSrc++;
	}
	pszDst[sizeofBuffer - 1] = 0;
}

void ChustdToAscii(char* sz, wchar* wsz)
{
	while(true)
	{
		const wchar& c = wsz[0];
		sz[0] = char(c);
		if( c == 0 )
			break;
		wsz++;
		sz++;
	}
}

void ChustdAssertMessage(const char* pszFileName, int32 line, const char* pszMsg)
{
	static const char szTitle[] = "ASSERT failure";
	
	char szBuffer[300];
	szBuffer[0] = 0;
	ChustdSzCat(szBuffer, pszFileName, 300);
	ChustdSzCat(szBuffer, " (", 300);
	
	wchar szLine[16];
	char szLine2[16];
	chustd::FormatInt32(line, szLine);
	ChustdToAscii(szLine2, szLine);
	ChustdSzCat(szBuffer, szLine2, 300);
	ChustdSzCat(szBuffer, ")", 300);

	ChustdSzCat(szBuffer, "\n\n", 300);
	ChustdSzCat(szBuffer, pszMsg, 300);

#ifdef WIN32
	typedef int (WINAPI *PFN_MESSAGEBOXA)(HWND, LPCSTR, LPCSTR, UINT);

	// Manually loads user32.dll as this is the only dependency in chustd
	HMODULE hMod = LoadLibraryW(L"user32.dll");
	if( hMod )
	{
		PFN_MESSAGEBOXA pfnMessageBoxA = (PFN_MESSAGEBOXA) GetProcAddress(hMod, "MessageBoxA");
		pfnMessageBoxA(nullptr, szBuffer, szTitle, MB_TASKMODAL | MB_OK | MB_TOPMOST);
		FreeLibrary(hMod);
	}
#elif CHUOS
	//chuos_SystemConsole_WriteLine(szBuffer);
#else
	fprintf(stderr, "%s - %s(%d) : %s", szTitle, pszFileName, line, pszMsg);
#endif // WIN32
}

void ChustdDebugMessage(const char* pszMsg)
{
#ifdef WIN32
	OutputDebugStringA(pszMsg);
	OutputDebugStringA("\n");
#elif CHUOS
	//chuos_SystemConsole_WriteLine(pszMsg);
#else
	fprintf(stderr, "%s\n", pszMsg);
#endif // WIN32
}

//////////////////////////////////////////////////////////////////////////////////////////////
void ChustdTraceLine(const chustd::String& str)
{
	static bool bCreated = false;

	uint32 openMode = 0;
	if( !bCreated )
	{
		openMode = chustd::IFile::modeWrite;
		bCreated = true;
	}
	else
	{
		openMode = chustd::IFile::modeAppend;
	}

	chustd::File file;
	if( file.Open("chustd-trace.txt", openMode) )
	{
		// Avoid code units conversion by using the platform byte order
		file.SetByteOrder(chustd::k_ePlatformByteOrder);

		if( file.GetSize() == 0 )
		{
			// Put the BOM
			file.Write16(uint16(0xFEFF));
		}
		file.WriteString(str);

#ifdef _WIN32 // Let's be kind with Windows
		file.Write16(uint16(0x000D));
#endif

		file.Write16(uint16(0x000A));
	}
}

//////////////////////////////////////////
void ChustdDebugPrintLeak(void* p, int size, const char* pszFileName, int32 line)
{
	wchar szPointer[20];
	chustd::FormatPtr(p, szPointer);
	
	wchar szSize[16];
	chustd::FormatInt32(size, szSize, 6, L' ');

	char szBuffer[512];
	szBuffer[0] = 0;

	char szTmp[16];
	
	ChustdToAscii(szTmp, szPointer);
	ChustdSzCat(szBuffer, szTmp, sizeof(szBuffer));
	ChustdSzCat(szBuffer, " / ", sizeof(szBuffer));
	
	ChustdToAscii(szTmp, szSize);
	ChustdSzCat(szBuffer, szTmp, sizeof(szBuffer));
	ChustdSzCat(szBuffer, " : ", sizeof(szBuffer));

	// Display the content of the buffer
	uint8* pData = (uint8*) p;
	
	int32 showSize = 16;
	
	// Values show
	for(int i = 0; i < showSize; ++i)
	{
		if( i < size )
		{
			wchar szData[16];
			chustd::FormatUInt8Hex(pData[i], szData, false);

			ChustdToAscii(szTmp, szData);
			ChustdSzCat(szBuffer, szTmp, sizeof(szBuffer));
			ChustdSzCat(szBuffer, " ", sizeof(szBuffer));
		}
		else
		{
			ChustdSzCat(szBuffer, "   ", sizeof(szBuffer));
		}
	}

	ChustdSzCat(szBuffer, "  ", sizeof(szBuffer));

	// Ascii show
	for(int i = 0; i < showSize; ++i)
	{
		if( i < size )
		{
			uint8 char8 = pData[i];
			if( char8 <= 31 )
			{
				char8 = '.';
			}
			szTmp[0] = char8;
			szTmp[1] = 0;
			ChustdSzCat(szBuffer, szTmp, sizeof(szBuffer));
		}
		else
		{
			ChustdSzCat(szBuffer, " ", sizeof(szBuffer));
		}
	}

	if( pszFileName && line > 0 )
	{
		ChustdSzCat(szBuffer, "   ", sizeof(szBuffer));
		ChustdSzCat(szBuffer, pszFileName, sizeof(szBuffer));

		wchar szData[16];
		chustd::FormatInt32(line, szData);
		ChustdToAscii(szTmp, szData);

		ChustdSzCat(szBuffer, " (", sizeof(szBuffer));
		ChustdSzCat(szBuffer, szTmp, sizeof(szBuffer));
		ChustdSzCat(szBuffer, ")", sizeof(szBuffer));
	}

	ChustdDebugMessage(szBuffer);
}
