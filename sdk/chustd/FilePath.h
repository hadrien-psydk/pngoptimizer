///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_FILEPATH_H
#define CHUSTD_FILEPATH_H

#include "String.h"

namespace chustd {

class FilePath
{
public:
	// Adds a separator to a path if necessary
	static String AddSeparator(const String& dir, wchar cModel = '/');

	// Removes a separator to a path if present
	static String RemoveSeparator(const String& dir);

	// Returns true if a character is a directory separator
	static bool IsSeparator(const wchar c) { return ( c == '\\' || c == '/'); }

	static String Combine(const String& part1, const String& part2);

	// Adds a prefix to a filename within a path : /foo/bar/filename.txt -> /foo/bar/prefix-filename.txt
	static String AddFileNamePrefix(const String& path, const String& prefix);

	// Removes the extension of a file name in a file path
	static String RemoveExtension(const String& path); // NOTE: Removes the '.'

	// Splits a path into its directory part and file name part
	static void Split(const String& path, String& dir, String& fileName);

	// Gets the extension of a file, in lower case and without the dot
	static String GetExtension(const String& path);

	// Gets the name of a file from its path
	static String GetName(const String& path);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace chustd

#endif
