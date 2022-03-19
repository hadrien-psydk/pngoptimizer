///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ArgvParser.h"
#include "StringBuilder.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
// Builds the object from a Linux argc, argv. Assume encoding is UTF-8
///////////////////////////////////////////////////////////////////////////////
ArgvParser::ArgvParser(int argc, const char* const* argv)
{
	// Skip the exe path by starting at index 1
	if( argc < 1 )
	{
		return;
	}
	StringArray argStrings;
	argStrings.EnsureCapacity(argc-1);
	for(int i = 1; i < argc; ++i)
	{
		argStrings.Add( String::FromUtf8(argv[i], (int)strlen(argv[i])) );
	}
	ConvertToArgs(argStrings);
}

///////////////////////////////////////////////////////////////////////////////
// Builds the object from a Windows argc, argv. Encoding is UTF-16
///////////////////////////////////////////////////////////////////////////////
ArgvParser::ArgvParser(int argc, const wchar* const* argv)
{
	// Skip the exe path by starting at index 1
	if( argc < 1 )
	{
		return;
	}
	StringArray argStrings;
	argStrings.EnsureCapacity(argc-1);
	for(int i = 1; i < argc; ++i)
	{
		argStrings.Add( String(argv[i]) );
	}
	ConvertToArgs(argStrings);
}

///////////////////////////////////////////////////////////////////////////////
int ArgvParser::GetFlagIndex(const String& flagName) const
{
	String strWanted = flagName.ToLowerCase();

	const int argCount = m_args.GetSize();
	for(int i = 0; i < argCount; ++i)
	{
		if( m_args[i].flag )
		{
			String strName = m_args[i].name.ToLowerCase();

			if( strName == strWanted )
				return i;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
bool ArgvParser::HasFlag(const String& flagName) const
{
	if( GetFlagIndex(flagName) < 0 )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
String ArgvParser::GetFlagString(const String& flagName) const
{
	const int index = GetFlagIndex(flagName);
	if( index < 0 )
	{
		return String();
	}
	return m_args[index].flagString;
}

///////////////////////////////////////////////////////////////////////////////
int ArgvParser::GetFlagInt(const String& flagName) const
{
	const int index = GetFlagIndex(flagName);
	if( index < 0 )
	{
		return 0;
	}
	int value = 0;
	m_args[index].flagString.ToInt(value);
	return value;
}

///////////////////////////////////////////////////////////////////////////////
// Builds the m_args array from the list of command line arguments.
// Stop interpereting flags when "--" is found.
void ArgvParser::ConvertToArgs(const StringArray& astrArgs)
{
	bool doubleDashFound = false;
	const int argCount = astrArgs.GetSize();
	for(int iArg = 0; iArg < argCount; ++iArg)
	{
		const String& strArg = astrArgs[iArg];
		const int argLength = strArg.GetLength();

		if( argLength == 0 )
		{
			// Ignore empty arguments
			continue;
		}

		if( strArg == "--" )
		{
			doubleDashFound = true;
			// Do not store the double dash in the list
			continue;
		}

		int index = m_args.Add();
		Arg& arg = m_args[index];

		wchar cFirst = strArg.GetAt(0);

		if( !doubleDashFound && cFirst == '-' )
		{
			StringBuilder sbName;
			StringBuilder sbFlagString;

			// We've got an argument
			arg.flag = true;

			// Cut the argument
			enum SearchMode
			{
				SearchSecondDash,
				SearchFlagName,
				SearchPreFlagString,
				SearchFlagStringNoDQ,
				SearchFlagStringDQ
			};
			SearchMode searchMode = SearchSecondDash; // Accept "--flag" formatting

			for(int i = 1; i < argLength; ++i)
			{
				wchar c = strArg.GetAt(i);

				if( searchMode == SearchSecondDash )
				{
					if( c != '-' )
					{
						sbName += c;
					}
					searchMode = SearchFlagName;
				}
				else if( searchMode == SearchFlagName )
				{
					if( c == ':' )
					{
						searchMode = SearchPreFlagString;
					}
					else if( c == '"' )
					{
						searchMode = SearchFlagStringDQ;
					}
					else
					{
						sbName += c;
					}
				}
				else if( searchMode == SearchFlagStringNoDQ )
				{
					sbFlagString += c;
				}
				else if( searchMode == SearchFlagStringDQ )
				{
					if( c == '"' )
					{
						// We reached the end of this argument
						break;
					}
					else
					{
						sbFlagString += c;
					}
				}
				else if( searchMode == SearchPreFlagString )
				{
					if( c == '"' )
					{
						searchMode = SearchFlagStringDQ;
					}
					else
					{
						// The flag string is not in double-quotes
						sbFlagString += c;
						searchMode = SearchFlagStringNoDQ;
					}
				}
			}
			arg.name = sbName.ToString();
			arg.flagString = sbFlagString.ToString();
		}
		else
		{
			// Regular argument
			arg.flag = false;
			arg.name = strArg;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
StringArray ArgvParser::GetRegularArgs() const
{
	StringArray ret;
	foreach(m_args, i)
	{
		const Arg& arg = m_args[i];
		if( !arg.flag )
		{
			ret.Add(arg.name);
		}
	}
	return ret;
}

/////////////////////
} // namespace chustd
