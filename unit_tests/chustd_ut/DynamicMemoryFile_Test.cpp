#include "stdafx.h"


TEST(DynamicMemoryFile, Empty)
{
	uint8 buf[10];
	DynamicMemoryFile dmf;
	ASSERT_FALSE(dmf.Open(-1));
	ASSERT_TRUE(dmf.Open(0));
	ASSERT_FALSE(dmf.SetPosition(-1));
	ASSERT_FALSE(dmf.SetPosition(1));
	ASSERT_TRUE(dmf.SetPosition(0));
	ASSERT_EQ(0, dmf.GetSize());
	ASSERT_EQ(0, dmf.GetPosition());
	ASSERT_EQ(0, dmf.Read(buf, 10));
	ASSERT_EQ(0, dmf.GetPosition());
	ASSERT_EQ(0, dmf.Write(buf, 0));
	ASSERT_EQ(0, dmf.GetPosition());
	ASSERT_EQ(boBigEndian, dmf.GetByteOrder());
}

TEST(DynamicMemoryFile, ReadWrite)
{
	const uint8 inbuf[] = { 1, 2, 3 };
	uint8 buf[10];

	DynamicMemoryFile dmf;
	ASSERT_TRUE(dmf.Open(0));
	ASSERT_EQ(3, dmf.Write(inbuf, 3));
	ASSERT_EQ(3, dmf.GetPosition());
	ASSERT_EQ(0, dmf.Read(buf, 10));
	ASSERT_TRUE(dmf.SetPosition(0));
	ASSERT_EQ(3, dmf.Read(buf, 10));
	ASSERT_TRUE(Memory::Equals(inbuf, buf, 3));
	ASSERT_EQ(3, dmf.GetPosition());
}

TEST(DynamicMemoryFile, WriteFromFile)
{
	const uint8 inbuf[] = { 10, 11, 12 };
	uint8 buf[10];

	DynamicMemoryFile src;
	ASSERT_TRUE(src.Open(0));
	ASSERT_EQ(3, src.Write(inbuf, 3));
	ASSERT_TRUE(src.SetPosition(0));

	DynamicMemoryFile dst;

	// First write
	ASSERT_EQ(1, dst.WriteFromFile(src, 1));
	ASSERT_EQ(1, src.GetPosition());

	ASSERT_EQ(0, dst.Read(buf, 10));
	ASSERT_EQ(1, dst.GetPosition());
	ASSERT_TRUE(dst.SetPosition(0));
	ASSERT_EQ(1, dst.Read(buf, 10));
	ASSERT_EQ(10, buf[0]);

	// Second write
	ASSERT_EQ(2, dst.WriteFromFile(src, 5));
	ASSERT_EQ(3, src.GetPosition());

	ASSERT_EQ(0, dst.Read(buf, 10));
	ASSERT_EQ(3, dst.GetPosition());
	ASSERT_TRUE(dst.SetPosition(0));
	memset(buf, 0, sizeof(buf));
	ASSERT_EQ(3, dst.Read(buf, 10));
	ASSERT_EQ(10, buf[0]);
	ASSERT_EQ(11, buf[1]);
	ASSERT_EQ(12, buf[2]);
}

TEST(DynamicMemoryFile, WriteFromFile_EOF)
{
	const uint8 inbuf[] = { 10, 11, 12 };
	uint8 buf[10];

	DynamicMemoryFile src;
	ASSERT_TRUE(src.Open(0));
	ASSERT_EQ(3, src.Write(inbuf, 3));
	ASSERT_TRUE(src.SetPosition(0));

	// EOF case
	ASSERT_TRUE(src.SetPosition(0));
	DynamicMemoryFile dst2;
	// readAmount is set to 1 to ensure we will loop inside the function
	ASSERT_EQ(3, dst2.WriteFromFile(src, -1, 1));
	ASSERT_EQ(3, src.GetPosition());
	ASSERT_TRUE(dst2.SetPosition(0));
	memset(buf, 0, sizeof(buf));
	ASSERT_EQ(3, dst2.Read(buf, 10));
	ASSERT_EQ(10, buf[0]);
	ASSERT_EQ(11, buf[1]);
	ASSERT_EQ(12, buf[2]);
}
