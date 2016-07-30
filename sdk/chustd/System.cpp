///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "System.h"

#include "String.h"
#include "File.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
// Gets a system timer time in milliseconds
uint32 System::GetTime()
{
#if defined(_WIN32)
	return GetTickCount();

#elif defined(__linux__)
	timespec ts = {};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	int64 ret = ts.tv_sec;
	ret *= 1000000000;
	ret += ts.tv_nsec;
	ret /= 1000000;
	return static_cast<uint32>(ret);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Gets a system timer time in microseconds
uint64 System::GetTime64()
{
#if defined(_WIN32)
	return GetTickCount()*1000;

#elif defined(__linux__)
	timespec ts = {};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	int64 ret = ts.tv_sec;
	ret *= 1000000000;
	ret += ts.tv_nsec;
	ret /= 1000;
	return static_cast<uint32>(ret);
#endif
}

///////////////////////////////////////////////////////////////////////////////
String System::GetFontsDirectory()
{
#if defined(_WIN32)
	wchar szPath[MAX_PATH];
	if( SHGetSpecialFolderPathW(NULL, szPath, CSIDL_FONTS, FALSE) )
	{
		return String(szPath);
	}
	return String();

#elif defined(__linux__)
	return String();
#endif
}

///////////////////////////////////////////////////////////////////////////////
String System::GetUserConfigDirectory()
{
#if defined(_WIN32)
	wchar szPath[MAX_PATH];
	if( SHGetSpecialFolderPathW(NULL, szPath, CSIDL_APPDATA, TRUE) )
	{
		return String(szPath);
	}
	return String();

#elif defined(__linux__)
	struct passwd* pw = getpwuid(getuid());
	const char* homedir = pw->pw_dir;
	return FilePath::Combine(String(homedir), ".config");
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
