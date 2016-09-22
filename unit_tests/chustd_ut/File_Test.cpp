#include "stdafx.h"

TEST(File, GetDrive)
{
	String str0 = File::GetDrive("abc");
	ASSERT_TRUE(str0.IsEmpty());

	String str1 = File::GetDrive("/abc");
	ASSERT_TRUE(str1.IsEmpty());

	String str2 = File::GetDrive("\\abc");
	ASSERT_TRUE(str2.IsEmpty());

	String str3 = File::GetDrive("System:abc");
	ASSERT_TRUE(str3 == "System");

	String str4 = File::GetDrive("System:/abc");
	ASSERT_TRUE(str4 == "System");
}

TEST(File, Open)
{
	String filePath = Process::GetCurrentDirectory();
	filePath = FilePath::Combine(filePath, "testfile.bin");
	File::Delete(filePath); // Ensure clean state
	
	// Write a new file
	{
	File file;
	ASSERT_TRUE( file.Open(filePath, File::modeWrite) );
	ASSERT_TRUE( file.WriteStringUtf8("plop") );
	file.Close();
	}

	// Check content
	{
	File file;
	ASSERT_TRUE( file.Open(filePath, File::modeRead) );
	char buf[100];
	ASSERT_EQ( 4, file.Read(buf, sizeof(buf)) );
	ASSERT_TRUE( memcmp(buf, "plop", 4) == 0 );
	file.Close();
	}

	// Reopen in write mode, the size should shrink to 0 upon Open
	{
	File file;
	ASSERT_TRUE( file.Open(filePath, File::modeWrite) );
	ASSERT_TRUE( file.WriteStringUtf8("xy") );
	file.Close();
	}

	// Check content again
	{
	File file;
	ASSERT_TRUE( file.Open(filePath, File::modeRead) );
	char buf[100];
	ASSERT_EQ( 2, file.Read(buf, sizeof(buf)) );
	ASSERT_TRUE( memcmp(buf, "xy", 2) == 0 );
	file.Close();
	}

	ASSERT_TRUE( File::Delete(filePath));
}

TEST(File, Append)
{
	String filePath = Process::GetCurrentDirectory();
	filePath = FilePath::Combine(filePath, "testfile.txt");
	
	if( File::Exists(filePath) )
	{
		ASSERT_TRUE( File::Delete(filePath));
	}

	File file0;
	for(int i = 0; i < 4; ++i)
	{
		ASSERT_TRUE( file0.Open(filePath, File::modeAppend) );
		ASSERT_TRUE( file0.WriteString("A", TextEncoding::Windows1252()) == 1 );
		file0.Close();
	}

	uint32 nContent;
	ASSERT_TRUE( file0.Open(filePath) );
	ASSERT_TRUE( file0.GetSize() == 4 );
	ASSERT_TRUE( file0.Read32(nContent) );

	// No more data
	uint8 buf[10];
	ASSERT_EQ( 0, file0.Read(buf, 0) );
	ASSERT_EQ( 0, file0.Read(buf, 10) );

	file0.Close();

	ASSERT_TRUE( nContent == MAKE32('A','A','A','A') );
	
	ASSERT_TRUE( File::Delete(filePath) );
}

TEST(File, LastWriteTime)
{
	{
	File file;
	DateTime now = DateTime::GetNow();
	ASSERT_TRUE( file.Open("test-file.txt", File::modeWrite) );
	DateTime dt = file.GetLastWriteTime();
	Duration dur = dt - now;
	ASSERT_TRUE( dur.GetTotalSeconds() < 10 );
	file.Close();
	}

	{
	File file;
	ASSERT_TRUE( file.Open("test-file2.txt", File::modeWrite) );
	DateTime dt(1999, 9, 9, 0, 0, 42);
	ASSERT_TRUE( file.SetLastWriteTime(dt) );
	file.Close();

	ASSERT_TRUE( file.Open("test-file2.txt", File::modeRead) );
	ASSERT_TRUE( dt == file.GetLastWriteTime() );
	file.Close();
	}

	ASSERT_TRUE( File::Delete("test-file.txt") );
	ASSERT_TRUE( File::Delete("test-file2.txt") );
}

TEST(File, Rename)
{
	ASSERT_TRUE( File::WriteTextUtf8("test-file.txt", "rename") );
	ASSERT_TRUE( File::Rename("test-file.txt", "test-file-renamed.txt") );
	ASSERT_TRUE( File::Delete("test-file-renamed.txt") );
}
