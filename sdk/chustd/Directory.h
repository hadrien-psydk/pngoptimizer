///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_DIRECTORY_H
#define CHUSTD_DIRECTORY_H

#include "String.h"

namespace chustd {\

class StringArray;

class Directory
{
public:
	static StringArray GetFileNames(const String& dirPath, const String& joker,
	                                bool fullPaths = false);
	
	static bool Create(const String& dirPath);
	static bool Exists(const String& dirPath);
	static bool Delete(const String& dirPath);
};

class Joker
{
public:
	Joker(const String& str);
	bool Matches(const String& fileName) const;

private:
	String m_str;
	String m_left;
	String m_right;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace chustd

#endif // ndef CHUSTD_DIRECTORY_H
