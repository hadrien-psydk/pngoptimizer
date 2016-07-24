#include "stdafx.h"

TEST(FilePath, Split)
{
	String path, name;

	FilePath::Split("abc", path, name);
	ASSERT_TRUE(path == "");
	ASSERT_TRUE(name == "abc");

	FilePath::Split("/def", path, name);
	ASSERT_TRUE(path == "/");
	ASSERT_TRUE(name == "def");

	FilePath::Split("\\ghi", path, name);
	ASSERT_TRUE(path == "\\");
	ASSERT_TRUE(name == "ghi");

	FilePath::Split(":jkl", path, name);
	ASSERT_TRUE(path == ":");
	ASSERT_TRUE(name == "jkl");

	FilePath::Split("onk/mno", path, name);
	ASSERT_TRUE(path == "onk/");
	ASSERT_TRUE(name == "mno");

	FilePath::Split("plop/onk/pqr", path, name);
	ASSERT_TRUE(path == "plop/onk/");
	ASSERT_TRUE(name == "pqr");

	FilePath::Split("Work:plop/onk/stu", path, name);
	ASSERT_TRUE(path == "Work:plop/onk/");
	ASSERT_TRUE(name == "stu");
}

TEST(FilePath, GetExtension)
{
	ASSERT_TRUE("png" == FilePath::GetExtension("/plop.png"));
	ASSERT_TRUE("" == FilePath::GetExtension("/plop"));
}

TEST(FilePath, GetName)
{
	ASSERT_TRUE("plop.png" == FilePath::GetName("/plop.png"));
	ASSERT_TRUE("" == FilePath::GetName("somedir/"));
	ASSERT_TRUE("plop.svg" == FilePath::GetName("plop.svg"));
}

