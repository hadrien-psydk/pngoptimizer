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

	const int32 argCount = m_args.GetSize();
	for(int32 i = 0; i < argCount; ++i)
	{
		if( m_args[i].m_flag)
		{
			String strName = m_args[i].m_name.ToLowerCase();

			if( strName == strWanted)
				return i;
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////
bool ArgvParser::HasFlag(const String& flagName) const
{
	if( GetFlagIndex(flagName) < 0)
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
String ArgvParser::GetFlagString(const String& flagName) const
{
	const int32 index = GetFlagIndex(flagName);
	if( index < 0)
	{
		return String();
	}

	return m_args[index].m_flagString;
}

///////////////////////////////////////////////////////////////////////////////
void ArgvParser::ConvertToArgs(const StringArray& astrArgs)
{
	const int argCount = astrArgs.GetSize();
	for(int iArg = 0; iArg < argCount; ++iArg)
	{
		const String& strArg = astrArgs[iArg];
		const int argLength = strArg.GetLength();

		if( argLength > 0)
		{
			int index = m_args.Add();
			Arg& arg = m_args[index];

			wchar cFirst = strArg.GetAt(0);
			if( cFirst == '/' || cFirst == '-')
			{
				StringBuilder sbName;
				StringBuilder sbFlagString;

				// We've got an argument
				arg.m_flag = true;

				// Cut the argument
				enum SearchMode { searchFlagName, searchPreFlagString, searchFlagStringNoDQ, searchFlagStringDQ };
				SearchMode searchMode = searchFlagName;

				for(int i = 1; i < argLength; ++i)
				{
					wchar c = strArg.GetAt(i);

					if( searchMode == searchFlagName)
					{
						if( c == ':')
						{
							searchMode = searchPreFlagString;
						}
						else if( c == '"')
						{
							searchMode = searchFlagStringDQ;
						}
						else
						{
							sbName += c;
						}
					}
					else
					{
						if( searchMode == searchFlagStringNoDQ)
						{
							sbFlagString += c;
						}
						else if( searchMode == searchFlagStringDQ)
						{
							if( c == '"')
							{
								// We reached the end of this argument
								break;
							}
							else
							{
								sbFlagString += c;
							}
						}
						else if( searchMode == searchPreFlagString)
						{
							if( c == '"')
							{
								searchMode = searchFlagStringDQ;
							}
							else
							{
								// The flag string is not in double-quotes
								sbFlagString += c;
								searchMode = searchFlagStringNoDQ;
							}
						}
					}
				}
				arg.m_name = sbName.ToString();
				arg.m_flagString = sbFlagString.ToString();
			}
			else
			{
				arg.m_flag = false;
				arg.m_name = strArg;
			}
		}
	}
}

/////////////////////
} // namespace chustd
