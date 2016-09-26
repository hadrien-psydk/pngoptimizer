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
#include "FilePath.h"
#include "System.h"
#include "Directory.h"

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
// Gets the full path of the INI configuration file, depending on what exists
// already along the executable. This version is private to avoid confusion
// with the forWrite argument.
//
// [in] appName   Name of the application used for creating sub-directory and INI file.
// [in] forWrite  true to create the directory structure if it does not exist.
//
// Returns the full path or empty string if not found
///////////////////////////////////////////////////////////////////////////////
static String GetConfigPath(const String& appName, bool forWrite)
{
	String configFileName = appName + ".ini";

	// A settings file in the executable directory ?
	String exeDir = Process::GetExecutableDirectory();
	String filePath = FilePath::Combine(exeDir, configFileName);
	if( !File::Exists(filePath) )
	{
		// Nop, use a user profile location
		String configDir = System::GetUserConfigDirectory();
		if( !Directory::Exists(configDir) )
		{
			if( forWrite )
			{
				// A fresh OS install, the directory does not exist yet
				if( !Directory::Create(configDir) )
				{
					return "";
				}
			}
		}

		String appConfigDir = FilePath::Combine(configDir, appName);
		if( !Directory::Exists(appConfigDir) )
		{
			if( forWrite )
			{
				// First time the application is run, create profile directory
				if( !Directory::Create(appConfigDir) )
				{
					return "";
				}
			}
		}
		filePath = FilePath::Combine(appConfigDir, configFileName);
		if( !forWrite )
		{
			if( !File::Exists(filePath) )
			{
				// First run
				return "";
			}
		}
	}
	return filePath;
}

///////////////////////////////////////////////////////////////////////////////
// Gets the path of the configuration file or empty string if it does not exist
///////////////////////////////////////////////////////////////////////////////
String Process::GetConfigPathForRead(const String& appName)
{
	return GetConfigPath(appName, false);
}

///////////////////////////////////////////////////////////////////////////////
// Gets the path of the configuration file, creating the directory structure
// if it does not exist yet. Returns an empty string if creation fails.
///////////////////////////////////////////////////////////////////////////////
String Process::GetConfigPathForWrite(const String& appName)
{
	return GetConfigPath(appName, true);
}

///////////////////////////////////////////////////////////////////////////////
}
