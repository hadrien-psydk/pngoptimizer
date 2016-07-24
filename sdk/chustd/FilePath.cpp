///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FilePath.h"
#include "StringBuilder.h"

namespace chustd {

///////////////////////////////////////////////////////////////////////////////////////////////////
String FilePath::AddSeparator(const String& dir, wchar cModel /*='/'*/)
{
	const int32 length = dir.GetLength();
	if( length > 0 )
	{
		const wchar c = dir.GetAt(length - 1); // Last character
		if( (c != '\\') && (c != '/') )
		{
			return dir + String(&cModel, 1);
		}
	}
	return dir;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String FilePath::RemoveSeparator(const String& dir)
{
	int length = dir.GetLength();
	if( length >= 1 )
	{
		wchar last = dir.GetAt(length - 1);
		if( IsSeparator(last) )
		{
			return dir.Left(length - 1);
		}
	}
	return dir;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String FilePath::Combine(const String& part1, const String& part2)
{
	return AddSeparator(part1) + part2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String FilePath::AddFileNamePrefix(const String& path, const String& prefix)
{
	String strDir, strFileName;
	Split(path, strDir, strFileName);

	String strNewPath = strDir + prefix + strFileName;
	return strNewPath;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Removes the extension of a file name in a file path. Returns the file path without the file name extension.
String FilePath::RemoveExtension(const String& path)
{
	const int32 length = path.GetLength();
	if( length > 0 )
	{
		for(int i = length - 1; i >= 0; i--)
		{
			if( path.GetAt(i) == L'.' )
			{
				return path.Left(i);
			}
		}
	}
	return path;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilePath::Split(const String& path, String& dir, String& fileName)
{
	const int32 length = path.GetLength();
	if( length == 0 )
	{
		dir.Empty();
		fileName.Empty();
		return;
	}
	
	int32 iSeparator = length - 1;
	for(; iSeparator >= 0; --iSeparator)
	{
		uint16 c = path.GetAt(iSeparator);
		if( c == '\\' || c == '/' || c == ':' )
		{
			break;
		}
	}

	if( iSeparator < 0 )
	{
		// No separator found
		dir.Empty();
		fileName = path;
		return;
	}

	dir = path.Left(iSeparator + 1); // +1 in order to include the separator
	fileName = path.Right( (length - iSeparator) - 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String FilePath::GetExtension(const String& filePath)
{
	const int32 length = filePath.GetLength();
	String strExt;

	for(int i = length - 1; i >= 0; i--)
	{
		wchar c = filePath.GetAt(i);
		if( c == L'.' )
		{
			const int count = length - 1 - i;
			strExt = filePath.Right(count);
			break;
		}
	}
	return strExt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
String FilePath::GetName(const String& path)
{
	String dir, name;
	Split(path, dir, name);
	return name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace chustd
