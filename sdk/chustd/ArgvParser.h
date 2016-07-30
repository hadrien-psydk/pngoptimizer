///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_ARGVPARSER_H
#define CHUSTD_ARGVPARSER_H

#include "String.h"
#include "Array.h"

namespace chustd {\

// Allows to parse arguments from the command line
class ArgvParser
{
public:
	ArgvParser(int argc, const char* const* argv);
	ArgvParser(int argc, const wchar* const* argv);

	// Returns true if a flag word is present in the command line
	// Example: compil -Verbose // GetFlag("verbose") returns true
	bool HasFlag(const String& flagName) const;

	// Returns the value assigned to a flag
	// Example: run -value:"100" // GetFlagString("value") returns "100"
	String GetFlagString(const String& flagName) const;

	// Returns the integer value assigned to a flag, 0 upon error
	// Example: run -value:"100" // GetFlagInt("value") returns 100
	int GetFlagInt(const String& flagName) const;

protected:
	StringArray m_argv;

	class Arg
	{
	public:
		bool m_flag;
		String m_name;
		String m_flagString;
	};
	Array<Arg> m_args;

private:
	void ConvertToArgs(const StringArray& argStrings);
	int  GetFlagIndex(const String& flagName) const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
