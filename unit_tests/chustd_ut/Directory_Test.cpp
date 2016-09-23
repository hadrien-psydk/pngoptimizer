#include "stdafx.h"

TEST(Directory, CreateDelete)
{
	// We start in a known state
	Directory::Delete("./testdir");

	ASSERT_FALSE( Directory::Exists("./testdir") );

	ASSERT_TRUE( Directory::Create("./testdir") );
	ASSERT_TRUE( Directory::Exists("./testdir") );
	ASSERT_TRUE( Directory::Exists("./testdir/") );

	ASSERT_TRUE( Directory::Delete("./testdir") );
	ASSERT_FALSE( Directory::Exists("./testdir") );
	ASSERT_FALSE( Directory::Exists("./testdir/") );

	// Test without ./
	Directory::Delete("./youpla");
	ASSERT_TRUE( Directory::Create("youpla") );
	ASSERT_TRUE( Directory::Exists("youpla") );
	ASSERT_TRUE( Directory::Delete("youpla") );
	ASSERT_FALSE( Directory::Exists("youpla") );
}

TEST(Directory, GetFileNames)
{
	// Obviously if we can cleanup, GetFileNames works fine :)
	StringArray delList = Directory::GetFileNames("mydir", "*", true);
	foreach(delList, i)
	{
		File::Delete(delList[i]);
	}
	if( Directory::Exists("mydir") )
	{
		ASSERT_TRUE( Directory::Delete("mydir") ) << "Failed to cleanup";
	}

	ASSERT_TRUE( Directory::Create("mydir") );
	ASSERT_TRUE( File::WriteTextUtf8("mydir/file1.txt", "") );
	ASSERT_TRUE( File::WriteTextUtf8("mydir/file2.txt", "") );
	ASSERT_TRUE( File::WriteTextUtf8("mydir/file3.xml", "") );

	StringArray fileNames = Directory::GetFileNames("mydir", "file1.txt");
	ASSERT_EQ( 1, fileNames.GetSize() );
	ASSERT_TRUE( "file1.txt" == fileNames[0] );

	fileNames = Directory::GetFileNames("mydir", "*");
	ASSERT_EQ( 3, fileNames.GetSize() );
	fileNames = fileNames.Sort();
	ASSERT_TRUE( "file1.txt" == fileNames[0] );
	ASSERT_TRUE( "file2.txt" == fileNames[1] );
	ASSERT_TRUE( "file3.xml" == fileNames[2] );

	
	fileNames = Directory::GetFileNames("mydir", "*.txt");
	ASSERT_EQ( 2, fileNames.GetSize() );
	fileNames = fileNames.Sort();
	ASSERT_TRUE( "file1.txt" == fileNames[0] );
	ASSERT_TRUE( "file2.txt" == fileNames[1] );

	fileNames = Directory::GetFileNames("mydir", "file*");
	ASSERT_EQ( 3, fileNames.GetSize() );
	fileNames = fileNames.Sort();
	ASSERT_TRUE( "file1.txt" == fileNames[0] );
	ASSERT_TRUE( "file2.txt" == fileNames[1] );
	ASSERT_TRUE( "file3.xml" == fileNames[2] );

	fileNames = Directory::GetFileNames("mydir", "file*", true);
	ASSERT_EQ( 3, fileNames.GetSize() );
	fileNames = fileNames.Sort();
	ASSERT_TRUE( "mydir/file1.txt" == fileNames[0] );
	ASSERT_TRUE( "mydir/file2.txt" == fileNames[1] );
	ASSERT_TRUE( "mydir/file3.xml" == fileNames[2] );

	// Cleanup
	// Should fail, mydir is not empty
	ASSERT_FALSE( Directory::Delete("mydir") );

	ASSERT_TRUE( File::Delete("mydir/file1.txt") );
	ASSERT_TRUE( File::Delete("mydir/file2.txt") );
	ASSERT_TRUE( File::Delete("mydir/file3.xml") );

	// Should be ok now
	ASSERT_TRUE( Directory::Delete("mydir") );
}

TEST(Joker, Matches)
{
	{
	Joker joker("abc");
	ASSERT_TRUE( joker.Matches("abc") );
	ASSERT_FALSE( joker.Matches("def") );
	}

	{
	Joker joker("*");
	ASSERT_TRUE( joker.Matches("abc") );
	ASSERT_TRUE( joker.Matches("def") );
	}

	{
	Joker joker("ab*");
	ASSERT_TRUE( joker.Matches("abc") );
	ASSERT_TRUE( joker.Matches("abde") );
	ASSERT_FALSE( joker.Matches("zab") );
	ASSERT_FALSE( joker.Matches("zak") );
	}

	{
	Joker joker("*bc");
	ASSERT_TRUE( joker.Matches("abc") );
	ASSERT_TRUE( joker.Matches("zzbc") );
	ASSERT_FALSE( joker.Matches("xbcy") );
	}

	{
	Joker joker("ab*bc");
	ASSERT_TRUE( joker.Matches("abbc") );
	ASSERT_TRUE( joker.Matches("abzbc") );
	ASSERT_FALSE( joker.Matches("abc") );
	}
}

