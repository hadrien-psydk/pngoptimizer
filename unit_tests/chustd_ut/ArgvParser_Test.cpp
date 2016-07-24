#include "stdafx.h"



TEST(ArgvParser, Linux)
{
	static const char* const cmdLine[] = { "/bin/app", "-simple", "-file:a b.txt" };
	//char* cmdLine[ARRAY_SIZE(cmdLineRO)];
	//memcpy(cmdLine, cmdLineRO, sizeof(cmdLineRO));

	ArgvParser ap(ARRAY_SIZE(cmdLine), cmdLine);

	ASSERT_TRUE( ap.HasFlag("simple") );
	ASSERT_TRUE( ap.HasFlag("file") );
	ASSERT_TRUE( ap.GetFlagString("file") == "a b.txt" );
}

TEST(ArgvParser, Win)
{
	const wchar* cmdLine[] = { UTF16("/bin/app"), UTF16("-simple"), UTF16("-file:a b.txt") };

	ArgvParser ap(ARRAY_SIZE(cmdLine), cmdLine);

	ASSERT_TRUE( ap.HasFlag("simple") );
	ASSERT_TRUE( ap.HasFlag("file") );
	ASSERT_TRUE( ap.GetFlagString("file") == "a b.txt" );
}
