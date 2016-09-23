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
		HANDLE handle;
		if( m_type == StdFileType::Stdout )
		{
			handle = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		else if( m_type == StdFileType::Stderr )
		{
			handle = GetStdHandle(STD_ERROR_HANDLE);
		}
		else
		{
			return;
		}

		if( col.r > th2 || col.g > th2 || col.b > th2 )
		{
			ccol |= FOREGROUND_INTENSITY;
		}

		if( !m_initialized )
		{
			CONSOLE_SCREEN_BUFFER_INFO ci;
			GetConsoleScreenBufferInfo(handle, &ci);
			m_original = ci.wAttributes;
			m_initialized = true;
		}
		int mask = ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		WORD att = static_cast<WORD>((m_original & mask) | ccol);
		SetConsoleTextAttribute(handle, att);

#elif defined(__linux__)
		int fd;
		if( m_type == StdFileType::Stdout )
		{
			fd = STDOUT_FILENO;
		}
		else if( m_type == StdFileType::Stderr )
		{
			fd = STDERR_FILENO;
		}
		else
		{
			return;
		}

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
		if( ::write(fd, buf, strlen(buf)) < 0 )
		{
			return;
		}
#endif
	}

	void SetNormalColor()
	{
		if( m_normalColor )
		{
			// Nothing to do
			return;
		}
		m_normalColor = true; // Back to normal color

#if defined(_WIN32)
		if( m_initialized )
		{
			HANDLE handle;
			if( m_type == StdFileType::Stdout )
			{
				handle = GetStdHandle(STD_OUTPUT_HANDLE);
			}
			else if( m_type == StdFileType::Stderr )
			{
				handle = GetStdHandle(STD_ERROR_HANDLE);
			}
			else
			{
				return;
			}
			SetConsoleTextAttribute(handle, static_cast<WORD>(m_original));
		}
#elif defined(__linux__)
		int fd;
		if( m_type == StdFileType::Stdout )
		{
			fd = STDOUT_FILENO;
		}
		else if( m_type == StdFileType::Stderr )
		{
			fd = STDERR_FILENO;
		}
		else
		{
			return;
		}
		static const char sz[] = "\033[0m";
		if( ::write(fd, sz, sizeof(sz)-1) < 0 )
		{
			return;
		}
#endif
	}

	ConsoleColorSetter(StdFileType type)
	{
		m_initialized = false;
		// By default we assume the current color when the program starts is the normal one
		m_normalColor = true;
		m_type = type;
		m_original = 0;
	}
	~ConsoleColorSetter()
	{
		SetNormalColor();
	}
private:
	bool m_initialized;
	bool m_normalColor; // true if current color changed back to normal
	StdFileType m_type;
	uint32 m_original;
};

static void ConsoleWrite(StdFileType ct, const String& str)
{
#if defined(_WIN32)
	HANDLE handle;
	if( ct == StdFileType::Stdout )
	{
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	else if( ct == StdFileType::Stderr )
	{
		handle = GetStdHandle(STD_ERROR_HANDLE);
	}

	DWORD written = 0;

	// Windows functions
	DWORD fileType = GetFileType(handle);
	if( fileType == FILE_TYPE_CHAR )
	{
		// Write to the window
		DWORD length = str.GetLength();
		BOOL bOk = WriteConsoleW(handle, str.GetBuffer(), length, &written, nullptr);
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
		WriteFile(handle, bytes.GetPtr(), bytes.GetSize(), &written, nullptr);
	}
#elif defined(__linux__)
	int fd;
	if( ct == StdFileType::Stdout )
	{
		fd = STDOUT_FILENO;
	}
	else if( ct == StdFileType::Stderr )
	{
		fd = STDERR_FILENO;
	}
	else
	{
		return;
	}

	String unified = str.UnifyNewlines(String::NT_Unix);
	ByteArray bytes = unified.ToBytes(TextEncoding::Utf8());
	if( ::write(fd, bytes.GetPtr(), bytes.GetSize()) < 0 )
	{
		return;
	}
#endif
}

static class Stdout : public IConsoleWriter
{
public:
	virtual void Write(const String& str)     { ConsoleWrite(StdFileType::Stdout, str); }
	virtual void WriteLine(const String& str) { ConsoleWrite(StdFileType::Stdout, str + "\n"); }

	virtual void SetTextColor(Color col) { m_colorSetter.SetColor(col); }
	virtual void ResetTextColor()        { m_colorSetter.SetNormalColor(); }

	Stdout() : m_colorSetter(StdFileType::Stdout) {}

private:
	ConsoleColorSetter m_colorSetter;
} g_stdout;

static class Stderr : public IConsoleWriter
{
public:
	virtual void Write(const String& str)     { ConsoleWrite(StdFileType::Stderr, str); }
	virtual void WriteLine(const String& str) { ConsoleWrite(StdFileType::Stderr, str + "\n"); }

	virtual void SetTextColor(Color col) { m_colorSetter.SetColor(col); }
	virtual void ResetTextColor()        { m_colorSetter.SetNormalColor(); }

	Stderr() : m_colorSetter(StdFileType::Stderr) {}

private:
	ConsoleColorSetter m_colorSetter;
} g_stderr;

///////////////////////////////////////////////////////////////////////////////
IConsoleWriter& Console::Stdout()
{
	return g_stdout;
}

IConsoleWriter& Console::Stderr()
{
	return g_stderr;
}

///////////////////////////////////////////////////////////////////////////////
// Waits for a string input by the user
//
// [in] prompt  Symbol to print before waiting for the string
// [in] input   Result
///////////////////////////////////////////////////////////////////////////////
bool Console::WaitForInput(const String& prompt, String& input)
{
	Stdout().Write(prompt);

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
bool Console::WaitForInput(String& str)
{
	static char szPrompt[] = ">";
	return WaitForInput(szPrompt, str);
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
