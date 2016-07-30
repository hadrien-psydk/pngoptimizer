///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Console.h"

#include "String.h"
#include "TextEncoding.h"

///////////////////////////////////////////////////////////////////////////////
namespace chustd {\

class ConsoleColorSetter
{
public:

#if defined(__linux__)
	enum
	{
		FOREGROUND_RED = 1,
		FOREGROUND_GREEN = 2,
		FOREGROUND_BLUE = 4
	};
#endif

	void SetColor(Color col)
	{
		int th1 = 64; // First threshold
		int th2 = 192; // Second threshold

		int ccol = 0;
		if( col.r > th1 )
		{
			ccol |= FOREGROUND_RED;
		}
		if( col.g > th1 )
		{
			ccol |= FOREGROUND_GREEN;
		}
		if( col.b > th1 )
		{
			ccol |= FOREGROUND_BLUE;
		}

#if defined(_WIN32)
		if( col.r > th2 || col.g > th2 || col.b > th2 )
		{
			ccol |= FOREGROUND_INTENSITY;
		}

		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if( !m_initialized )
		{
			CONSOLE_SCREEN_BUFFER_INFO ci;
			GetConsoleScreenBufferInfo(hStdOut, &ci);
			m_original = ci.wAttributes;
			m_initialized = true;
		}
		int mask = ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		WORD att = static_cast<WORD>((m_original & mask) | ccol);
		SetConsoleTextAttribute(hStdOut, att);

#elif defined(__linux__)
		static const int k_cis[] = {
			30, 31, 32, 33, 34, 35, 36, 37
		};

		int ci = k_cis[ccol];
		if( col.r > th2 || col.g > th2 || col.b > th2 )
		{
			ci += 60;
		}
		char buf[20];
		sprintf(buf, "\033[%dm", ci);
		if( ::write(1, buf, strlen(buf)) < 0 )
		{
			return;
		}
#endif
	}

	void SetNormalColor()
	{
#if defined(_WIN32)
		if( m_initialized )
		{
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hStdOut, static_cast<WORD>(m_original));
		}
#elif defined(__linux__)
		static const char sz[] = "\033[0m";
		if( ::write(1, sz, sizeof(sz)-1) < 0 )
		{
			return;
		}
#endif
	}

	Color GetColor()
	{
		Color ret;
#if defined(_WIN32)
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO ci;
		GetConsoleScreenBufferInfo(hStdOut, &ci);
		int mask = (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		WORD ccol = (ci.wAttributes & mask);

		uint8 th = 127;
		if( ccol & FOREGROUND_INTENSITY )
		{
			th = 255;
		}
		if( ccol & FOREGROUND_RED )
		{
			ret.r = th;
		}
		if( ccol & FOREGROUND_GREEN )
		{
			ret.g = th;
		}
		if( ccol & FOREGROUND_BLUE )
		{
			ret.b = th;
		}
#endif
		return ret;
	}

	ConsoleColorSetter()
	{
		m_initialized = false;
		m_original = 0;
	}
	~ConsoleColorSetter()
	{
		SetNormalColor();
	}
private:
	bool m_initialized;
	uint32 m_original;
};

static ConsoleColorSetter g_consoleColorSetter;

///////////////////////////////////////////////////////////////////////////////
Console::Console()
{

}

///////////////////////////////////////////////////////////////////////////////
Console::~Console()
{

}

///////////////////////////////////////////////////////////////////////////////
void Console::WriteLine(const String& str)
{
	Write(str + "\r\n");
}

///////////////////////////////////////////////////////////////////////////////
void Console::Write(const String& str)
{
#if defined(_WIN32)
	DWORD written = 0;

	// Windows functions
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD fileType = GetFileType(hStdOut);
	if( fileType == FILE_TYPE_CHAR )
	{
		// Write to the window
		DWORD length = str.GetLength();
		BOOL bOk = WriteConsoleW(hStdOut, str.GetBuffer(), length, &written, nullptr);
		bOk;
	}
	else
	{
		// Write to file.
		
		// Convert single \n to \r\n to make the output Notepad compatible (among others)
		String unified = str.UnifyNewlines(String::NT_Dos);

		// Write as UTF-8
		SetConsoleOutputCP(CP_UTF8); // Default in Windows 7, not sure for previous versions
		ByteArray bytes = unified.ToBytes(TextEncoding::Utf8());
		WriteFile(hStdOut, bytes.GetPtr(), bytes.GetSize(), &written, nullptr);
	}
#elif defined(__linux__)
	String unified = str.UnifyNewlines(String::NT_Dos);
	ByteArray bytes = unified.ToBytes(TextEncoding::Utf8());
	if( ::write(1, bytes.GetPtr(), bytes.GetSize()) < 0 )
	{
		return;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool Console::WaitForInput(String& str)
{
	static char szPrompt[] = ">";
	return WaitForInput(szPrompt, str);
}

///////////////////////////////////////////////////////////////////////////////
// Waits for a string input by the user
//
// [in] prompt  Symbol to print before waiting for the string
// [in] input   Result
///////////////////////////////////////////////////////////////////////////////
bool Console::WaitForInput(const String& prompt, String& input)
{
	Write(prompt);

	wchar szBuffer[128];

#ifdef WIN32
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

	DWORD numberOfCharsToRead = (sizeof(szBuffer) / 2) - 1;
	DWORD numberOfCharsRead = 0;

	BOOL bWinOk = ReadConsoleW(hStdIn, szBuffer, numberOfCharsToRead, &numberOfCharsRead, nullptr);

	if( !bWinOk )
		return false;
#else

	int32 numberOfCharsRead = 0;
#endif

	// Removes any carriage return or line feed
	int32 i = numberOfCharsRead - 1;
	for(; i >= 0; --i)
	{
		wchar c = szBuffer[i];
		if( c != 13 && c != 10 )
			break;
	}

	int32 realCharCount = i + 1;
	input.SetBuffer(szBuffer, realCharCount);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
uint32 Console::WaitForKey()
{
	uint32 char32 = 0;

#if defined(_WIN32)
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

	// Switch to raw mode (no line input, no echo input)
	DWORD oldMode = 0;
	GetConsoleMode(hStdIn, &oldMode);
	SetConsoleMode(hStdIn, 0);
	
	for(;;)
	{
		DWORD read = 0;
		INPUT_RECORD inputrec;
		if( !ReadConsoleInputW(hStdIn, &inputrec, 1, &read) || (read == 0) )
		{
			char32 = 0;
			break;
		}
		
		if( (inputrec.EventType == KEY_EVENT) && inputrec.Event.KeyEvent.bKeyDown )
		{
			char32 = inputrec.Event.KeyEvent.uChar.UnicodeChar;
			if( char32 != 0 )
			{
				break;
			}
		}
	}
	
	SetConsoleMode(hStdIn, oldMode);
#endif
	return char32;
}

///////////////////////////////////////////////////////////////////////////////
void Console::SetTextColor(Color col)
{
	g_consoleColorSetter.SetColor(col);
}

///////////////////////////////////////////////////////////////////////////////
void Console::ResetTextColor()
{
	g_consoleColorSetter.SetNormalColor();
}

///////////////////////////////////////////////////////////////////////////////
// Checks if the console was created by this process, and the process was run from an existing console
bool Console::IsOwned()
{
#if defined(_WIN32)
	HWND hConsoleWnd = GetConsoleWindow();
	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hConsoleWnd, &dwProcessId);
	return GetCurrentProcessId() == dwProcessId;
#else
	return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////
} // namespace chustd
