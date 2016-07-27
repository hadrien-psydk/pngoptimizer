///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Directory.h"

#include "String.h"
#include "Array.h"
#include "File.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
Joker::Joker(const String& str)
{
	m_str = str;
	int at = str.Find(String("*"), 0);
	if( at >= 0 )
	{
		m_left = str.Mid(0, at);
		m_right = str.Mid(at+1);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Checks if a file name matches this joker.
///////////////////////////////////////////////////////////////////////////////
bool Joker::Matches(const String& fileName) const
{
	int empty = 0;

	int leftIndex = 0;
	if( m_left.GetLength() > 0 )
	{
		leftIndex = fileName.Find(m_left, 0);
		if( leftIndex != 0 )
		{
			return false;
		}
	}
	else
	{
		empty = 1;
	}

	int rightIndex = 0;
	if( m_right.GetLength() > 0 )
	{
		rightIndex = fileName.Find(m_right, leftIndex + m_left.GetLength());
		if( rightIndex < 0 )
		{
			return false;
		}
		int wantedIndex = fileName.GetLength() - m_right.GetLength();
		if( rightIndex != wantedIndex )
		{
			return false;
		}
	}
	else
	{
		empty |= 2;
	}

	if( empty == 3 && m_str != "*" )
	{
		// Exact match
		return m_str == fileName;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// [in] fullPath : true to get paths as dir+name instead of name only
StringArray Directory::GetFileNames(const String& dirPath, const String& joker,
                                    bool fullPaths)
{
	StringArray filePaths;
	StringArray jokerPatterns;

	Array<Joker> jokers;

	if( !joker.IsEmpty() )
	{
		jokerPatterns = joker.Split('|');
	}
	else
	{
		jokerPatterns.Add("*");
	}

	jokers.EnsureCapacity(jokerPatterns.GetSize());
	foreach(jokerPatterns, i)
	{
		jokers.Add( Joker(jokerPatterns[i]) );
	}

	// We bypass the filter mechanism of FindFirstFile and use always use *
	// Instead we use our own filter mechanism to get a consistent behavior
	// on both Windows and Linux.

#if defined(_WIN32)
	WIN32_FIND_DATAW fdw;
	String fffPath = FilePath::Combine(dirPath, "*");
	HANDLE hFind = FindFirstFileW(fffPath.GetBuffer(), &fdw);
	if( hFind == INVALID_HANDLE_VALUE )
	{
		return filePaths;
	}

#elif defined(__linux__)
	// Linux wants ./ for local paths
	char buf[2+260]; // TODO: create a class to be able to hold long paths
	char* dirPath8 = buf+2; // Save space for "./"

	if( !dirPath.ToUtf8Z(dirPath8, sizeof(buf)-2) )
	{
		return filePaths;
	}
	if( dirPath8[0] != '/' )
	{
		// Relative path
		dirPath8 = buf;
		dirPath8[0] = '.';
		dirPath8[1] = '/';
	}

	DIR* pDir = opendir(dirPath8);
	if( !pDir )
	{
		return filePaths;
	}
	struct dirent* pEntry = readdir(pDir);
	if( !pEntry )
	{
		return filePaths;
	}
#endif

	String fileName;

	for(;;)
	{

#if defined(_WIN32)
		const wchar* entryName = fdw.cFileName;
#elif defined(__linux__)
		const char* entryName = pEntry->d_name;
#endif

		bool keepIt = false;

		// We do not want the "." nor the ".." directories
		bool dot = (entryName[0] == '.' && entryName[1] == 0);
		bool dotDot = (entryName[0] == '.' && entryName[1] == '.' && entryName[2] == 0);
		if( !(dot || dotDot) )
		{
#if defined(_WIN32)
			fileName = entryName;
#elif defined(__linux__)
			fileName = String::FromUtf8Z(entryName);
#endif
			foreach(jokers, i)
			{
				if( jokers[i].Matches(fileName) )
				{
					keepIt = true;
					break;
				}
			}
		}

		if( keepIt )
		{
			if( fullPaths )
			{
				filePaths.Add( FilePath::Combine(dirPath, fileName) );
			}
			else
			{
				filePaths.Add(fileName);
			}
		}

#if defined(_WIN32)
		if( !FindNextFileW(hFind, &fdw) )
		{
			break;
		}

#elif defined(__linux__)
		pEntry = readdir(pDir);
		if( !pEntry )
		{
			break;
		}
#endif
	}

#if defined(_WIN32)
	FindClose(hFind);

#elif defined(__linux__)
	closedir(pDir);
#endif

	return filePaths;
}

///////////////////////////////////////////////////////////////////////////////
// Creates a directory.
//
// [in] dirPath  Path of the directory to create
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////
bool Directory::Create(const String& dirPath)
{
#if defined(_WIN32)
	return CreateDirectoryW(dirPath.GetBuffer(), nullptr) != FALSE;

#elif defined(__linux__)
	char dirPath8[260];
	if( !dirPath.ToUtf8Z(dirPath8) )
	{
		return false;
	}
	return mkdir(dirPath8, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Checks if a directory exists
//
// [in] dirPath  Path of the directory to create
//
// Returns true if the directory exists
///////////////////////////////////////////////////////////////////////////////
bool Directory::Exists(const String& dirPath)
{
	bool ret = false;

#if defined(_WIN32)
	String dirPath2 = FilePath::AddSeparator(dirPath, '\\') + ".";
	DWORD attributes = GetFileAttributes(dirPath2.GetBuffer());
	if( attributes != INVALID_FILE_ATTRIBUTES )
	{
		ret = (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
#elif defined(__linux__)
	char dirPath8[260];
	if( !dirPath.ToUtf8Z(dirPath8) )
	{
		return false;		
	}
	struct stat st;
	if( stat(dirPath8, &st) != 0 )
	{
		return false;
	}
	ret = S_ISDIR(st.st_mode);
#endif
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Removes a directory
//
// [in] dirPath  Path of the directory to create
//
// Returns true if the directory exists
///////////////////////////////////////////////////////////////////////////////
bool Directory::Delete(const String& dirPath)
{
#if defined(_WIN32)
	return RemoveDirectoryW(dirPath.GetBuffer()) != FALSE;

#elif defined(__linux__)
	char dirPath8[260];
	if( !dirPath.ToUtf8Z(dirPath8) )
	{
		return false;		
	}
	return rmdir(dirPath8) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
