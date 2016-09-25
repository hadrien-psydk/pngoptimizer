#include "stdafx.h"

TEST(ArgvParser, Linux)
{
	static const char* const cmdLine[] = { "/bin/app", "-simple", "-file:a b.txt",
		"-file2:\"once more.txt\"", // With dquotes
		"-an_integer:42",
		"", // Empty argument
		"--double-dash",
		"/other_file_not_win_arg",
		"regular_name", "--", "-not_a_flag" };

	ArgvParser ap(ARRAY_SIZE(cmdLine), cmdLine);

	ASSERT_TRUE( ap.HasFlag("simple") );
	ASSERT_TRUE( ap.HasFlag("file") );
	ASSERT_TRUE( ap.GetFlagString("file") == "a b.txt" );
	ASSERT_TRUE( ap.GetFlagString("file2") == "once more.txt" );
	ASSERT_TRUE( ap.GetFlagInt("an_integer") == 42 );
	ASSERT_TRUE( ap.HasFlag("double-dash") );
	ASSERT_TRUE( !ap.HasFlag("other_file_not_win_arg") );

	// After the double dash, there are only regular arguments
	ASSERT_TRUE( !ap.HasFlag("not_a_flag") );
	auto regularArgs = ap.GetRegularArgs();
	ASSERT_EQ( 3, regularArgs.GetSize() );
	ASSERT_EQ( "/other_file_not_win_arg", regularArgs[0] );
	ASSERT_EQ( "regular_name", regularArgs[1] );
	ASSERT_EQ( "-not_a_flag",  regularArgs[2] );
}

TEST(ArgvParser, Win)
{
	const wchar* cmdLine[] = { UTF16("/bin/app"), UTF16("-simple"), UTF16("-file:a b.txt") };

	ArgvParser ap(ARRAY_SIZE(cmdLine), cmdLine);

	ASSERT_TRUE( ap.HasFlag("simple") );
	ASSERT_TRUE( ap.HasFlag("file") );
	ASSERT_TRUE( ap.GetFlagString("file") == "a b.txt" );
}

