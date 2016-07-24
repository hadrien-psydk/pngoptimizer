#include "stdafx.h"

TEST(ImageFormat, PackAndUnpackPixels1bpp)
{
	static uint8 raw1[] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 1, 0, 1, 0, 1, 0,
		1, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 1,
		1, 1, 0, 0, 1, 1, 0, 0,
		0, 0, 1, 1, 0, 0, 1, 1
	};
	Buffer buf;
	buf.Assign(raw1, sizeof(raw1));
	
	ASSERT_TRUE( ImageFormat::PackPixels(buf, 8, 7, PF_1bppIndexed) );

	static uint8 expected1[] = {
		BYTE_FROM_BITS(0, 0, 0, 0, 0, 0, 0, 0),
		BYTE_FROM_BITS(1, 1, 1, 1, 1, 1, 1, 1),
		BYTE_FROM_BITS(1, 0, 1, 0, 1, 0, 1, 0),
		BYTE_FROM_BITS(1, 1, 1, 1, 0, 0, 0, 0),
		BYTE_FROM_BITS(0, 0, 0, 0, 1, 1, 1, 1),
		BYTE_FROM_BITS(1, 1, 0, 0, 1, 1, 0, 0),
		BYTE_FROM_BITS(0, 0, 1, 1, 0, 0, 1, 1)
	};
	ASSERT_TRUE( buf.GetSize() == sizeof(expected1) );
	ASSERT_TRUE( memcmp(buf.GetReadPtr(), expected1, sizeof(expected1)) == 0 );

	ASSERT_TRUE( ImageFormat::UnpackPixels(buf, 8, 7, PF_1bppIndexed) );

	ASSERT_TRUE( buf.GetSize() == sizeof(raw1) );
	ASSERT_TRUE( memcmp(buf.GetReadPtr(), raw1, sizeof(raw1)) == 0 );
}

TEST(ImageFormat, PackAndUnpackPixels2bpp)
{
	// Add also padding
	static uint8 raw1[7*5] = {
		0, 1, 2, 3,  0, 1, 2,
		1, 2, 1, 2,  1, 2, 1,
		1, 3, 1, 3,  1, 3, 1,
		2, 0, 2, 0,  2, 0, 2,
		3, 2, 1, 0,  1, 2, 3,
	};
	Buffer buf;
	buf.Assign(raw1, sizeof(raw1));
	
	ASSERT_TRUE( ImageFormat::PackPixels(buf, 7, 5, PF_2bppIndexed) );

	static uint8 expected1[] = {
		BYTE_FROM_BITS2(0, 1, 2, 3), BYTE_FROM_BITS2(0, 1, 2, 0),
		BYTE_FROM_BITS2(1, 2, 1, 2), BYTE_FROM_BITS2(1, 2, 1, 0),
		BYTE_FROM_BITS2(1, 3, 1, 3), BYTE_FROM_BITS2(1, 3, 1, 0),
		BYTE_FROM_BITS2(2, 0, 2, 0), BYTE_FROM_BITS2(2, 0, 2, 0),
		BYTE_FROM_BITS2(3, 2, 1, 0), BYTE_FROM_BITS2(1, 2, 3, 0)
	};
	ASSERT_TRUE( buf.GetSize() == sizeof(expected1) );
	ASSERT_TRUE( memcmp(buf.GetReadPtr(), expected1, sizeof(expected1)) == 0 );

	ASSERT_TRUE( ImageFormat::UnpackPixels(buf, 7, 5, PF_2bppIndexed) );

	ASSERT_TRUE( buf.GetSize() == sizeof(raw1) );
	ASSERT_TRUE( memcmp(buf.GetReadPtr(), raw1, sizeof(raw1)) == 0 );
}

