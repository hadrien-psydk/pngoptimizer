///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"
#include "String.h"
#include "Array.h"
#include "File.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
String Process::GetCurrentDirectory()
{
#if defined(_WIN32)
	// Includes ending 0
	const int sizeofBuffer = ::GetCurrentDirectory(0, nullptr);

	wchar* pBuffer = new wchar[sizeofBuffer];
	::GetCurrentDirectory(sizeofBuffer, pBuffer);

	String strDirectory(pBuffer);

	delete[] pBuffer;
	return strDirectory;
	
#elif defined(__linux__)
	char* cdn = get_current_dir_name();
	String ret = String::FromUtf8Z(cdn);
	free(cdn);
	return ret;
#endif
}

///////////////////////////////////////////////////////////////////////////////
void Process::SetCurrentDirectory(const String& dirPath)
{
#if defined(_WIN32)
	::SetCurrentDirectory(dirPath.GetBuffer());
#elif defined(__linux__)
	// TODO
	ASSERT(0);
	(void)dirPath;
#endif
}

///////////////////////////////////////////////////////////////////////////////
uint16 Process::GetCurrentId()
{
#if defined(_WIN32)
	uint32 nId = ::GetCurrentProcessId();
	return uint16(nId);
#elif defined(__linux__)
	return 0;	
#endif
}

///////////////////////////////////////////////////////////////////////////////
String Process::GetCommandLine()
{
#if defined(_WIN32)
	LPWSTR psz = ::GetCommandLineW();
	return String(psz);

#elif defined(__linux__)
	String ret;
	int fd = open("/proc/self/cmdline", O_RDONLY);
	if( fd >= 0 )
	{
		char buf[400];
		int len = read(fd, buf, sizeof(buf));
		ret = String::FromUtf8(buf, len);
		close(fd);
	}
	return ret;
#endif
}

///////////////////////////////////////////////////////////////////////////////
chustd::StringArray Process::CommandLineToArgv(const String& cmdLine)
{
	StringArray astrResult;
	
	enum { stateInsideArg, stateInsideQuotedArg, stateCalm};
	
	int state = stateCalm;
	int32 argStart = -1;

	const int32 length = cmdLine.GetLength();
	for(int iChar = 0; iChar < length; ++iChar)
	{
		wchar c = cmdLine.GetAt(iChar);
		if( state == stateCalm )
		{
			if( c == L'"' )
			{
				state = stateInsideQuotedArg;
				argStart = iChar + 1;
			}
			else if( c != L' ' )
			{
				state = stateInsideArg;
				argStart = iChar;
			}
		}
		else if( state == stateInsideQuotedArg )
		{
			if( c == L'"' )
			{
				// End
				ASSERT(argStart >= 0);
				String strArg = cmdLine.Mid(argStart, iChar - argStart);
				astrResult.Add(strArg);
				state = stateCalm;
			}
		}
		else if( state == stateInsideArg )
		{
			if( c == L' ' )
			{
				ASSERT(argStart >= 0);
				String strArg = cmdLine.Mid(argStart, iChar - argStart);
				astrResult.Add(strArg);
				state = stateCalm;
			}
		}
	}

	// Last argument
	if( state == stateInsideArg )
	{
		ASSERT(argStart >= 0);
		String strArg = cmdLine.Mid(argStart);
		astrResult.Add(strArg);
		state = stateCalm;
	}

	return astrResult;
}

///////////////////////////////////////////////////////////////////////////////
String Process::GetExecutablePath()
{
#if defined(_WIN32)
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	wchar szBuffer[MAX_PATH];
	::GetModuleFileName(hInstance, szBuffer, MAX_PATH);

	String strProgram(szBuffer);
	return strProgram;

#elif defined(__linux__)
	char buf[400];
	ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
	buf[len] = 0;
	return String::FromUtf8Z(buf);
#endif
}

///////////////////////////////////////////////////////////////////////////////
String Process::GetExecutableDirectory()
{
	String strPath = GetExecutablePath();
	
	String strDir, strFileName;
	FilePath::Split(strPath, strDir, strFileName);
	return strDir;
}

///////////////////////////////////////////////////////////////////////////////
}
