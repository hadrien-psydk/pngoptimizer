#include "stdafx.h"

TEST(Buffer, Construction)
{
	Buffer buf;
	ASSERT_TRUE(buf.GetSize() == 0);
	ASSERT_TRUE(buf.GetReadPtr() == nullptr);
}

TEST(Buffer, SetSize)
{
	Buffer buf;
	ASSERT_TRUE(buf.SetSize(3));
	ASSERT_TRUE(buf.GetSize() == 3);
	ASSERT_TRUE(buf.GetReadPtr() != nullptr);

	uint8* pBytes = buf.GetWritePtr();
	pBytes[0] = 'a';
	pBytes[1] = 'b';
	pBytes[2] = 'c';

	ASSERT_TRUE(buf.SetSize(10000));
	ASSERT_EQ(10000, buf.GetSize());
	
	// The allocated block should have stay the same, we never deallocate when the buffer shrinks
	pBytes = buf.GetWritePtr();
	ASSERT_TRUE(buf.SetSize(0));
	ASSERT_EQ(buf.GetReadPtr(), pBytes);
}

TEST(Buffer, EnsureCapacity)
{
	Buffer buf;
	ASSERT_TRUE(buf.EnsureCapacity(3));
	ASSERT_EQ(0, buf.GetSize());
	ASSERT_TRUE(buf.GetReadPtr() != nullptr);
}

TEST(Buffer, CopyConstructor)
{
	// Empty in empty
	Buffer bufA;
	Buffer bufB = bufA;

	// With data
	Buffer buf1;
	ASSERT_TRUE(buf1.SetSize(2));
	ASSERT_TRUE(buf1.GetReadPtr() != nullptr);
	uint8* p1 = buf1.GetWritePtr();
	p1[0] = 'x';
	p1[1] = 'y';

	Buffer buf2 = buf1;
	ASSERT_EQ(buf2.GetSize(), buf1.GetSize());
	const uint8* p2 = buf2.GetReadPtr();
	ASSERT_TRUE(p1 == p2);
	ASSERT_TRUE(p2[0] == 'x');
	ASSERT_TRUE(p2[1] == 'y');

	// Unshare
	uint8* p2w = buf2.GetWritePtr();
	ASSERT_TRUE(p2w[0] == 'x');
	ASSERT_TRUE(p2w[1] == 'y');
	ASSERT_TRUE(p1[0] == 'x');
	ASSERT_TRUE(p1[1] == 'y');

	// Change new buffer
	p2w[0] = 'u';
	p2w[1] = 'v';

	// Original buffer should stay the same
	ASSERT_TRUE(p1[0] == 'x');
	ASSERT_TRUE(p1[1] == 'y');
}

TEST(Buffer, AssignmentOperator)
{
	// Empty in empty
	Buffer bufA;
	Buffer bufB;
	bufB = bufA;

	// With data
	Buffer buf1;
	ASSERT_TRUE(buf1.SetSize(2));
	uint8* p1 = buf1.GetWritePtr();
	p1[0] = 'x';
	p1[1] = 'y';

	Buffer buf2;
	ASSERT_TRUE(buf2.SetSize(1));
	uint8* p2 = buf2.GetWritePtr();
	p2[0] = 'A';

	// Assign
	buf2 = buf1;
	const uint8* p2r = buf2.GetReadPtr();
	ASSERT_EQ(buf2.GetSize(), buf1.GetSize());
	ASSERT_EQ(p2r[0], p1[0]);
	ASSERT_EQ(p2r[1], p1[1]);

	// Unshare
	uint8* p2w = buf2.GetWritePtr();
	ASSERT_TRUE(p2w[0] == 'x');
	ASSERT_TRUE(p2w[1] == 'y');
	ASSERT_TRUE(p1[0] == 'x');
	ASSERT_TRUE(p1[1] == 'y');

	// Change new buffer
	p2w[0] = 'u';
	p2w[1] = 'v';

	// Original buffer should stay the same
	ASSERT_TRUE(p1[0] == 'x');
	ASSERT_TRUE(p1[1] == 'y');
}

