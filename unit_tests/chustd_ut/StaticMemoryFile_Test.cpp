#include "stdafx.h"

TEST(StaticMemoryFile, OpenRead)
{
	const uint8 buf[6] = { 0, 1, 2, 3, 4, 5 };
	
	StaticMemoryFile smf;
	ASSERT_TRUE( smf.OpenRead(buf, sizeof(buf)) );
	ASSERT_EQ( 0, smf.GetPosition() );
	ASSERT_FALSE( smf.SetPosition(-1) );
	ASSERT_FALSE( smf.SetPosition(7) );
	ASSERT_TRUE( smf.SetPosition(6) );
	ASSERT_TRUE( smf.SetPosition(0) );
	ASSERT_EQ( 6, smf.GetSize() );

	uint8 readBuf[20];
	memset(readBuf, 0xff, sizeof(readBuf));
	ASSERT_EQ( 2, smf.Read(readBuf, 2) );
	ASSERT_EQ( 0, readBuf[0] );
	ASSERT_EQ( 1, readBuf[1] );
	ASSERT_EQ( 2, smf.GetPosition() );

	memset(readBuf, 0xff, sizeof(readBuf));
	ASSERT_EQ( 4, smf.Read(readBuf, 20) );
	ASSERT_EQ( 2, readBuf[0] );
	ASSERT_EQ( 3, readBuf[1] );
	ASSERT_EQ( 4, readBuf[2] );
	ASSERT_EQ( 5, readBuf[3] );

	ASSERT_EQ( boBigEndian, smf.GetByteOrder() );
	smf.Close();
	ASSERT_EQ( 0, smf.GetPosition() );
	ASSERT_EQ( 0, smf.GetSize() );
}

TEST(StaticMemoryFile, OpenWrite)
{
	uint8 buf[4] = { 0 };
	
	StaticMemoryFile smf;
	ASSERT_TRUE( smf.OpenWrite(buf, sizeof(buf)) );
	ASSERT_EQ( 0, smf.GetPosition() );
	ASSERT_FALSE( smf.SetPosition(-1) );
	ASSERT_FALSE( smf.SetPosition(5) );
	ASSERT_FALSE( smf.SetPosition(1) );
	ASSERT_TRUE( smf.SetPosition(0) );
	ASSERT_EQ( 0, smf.GetSize() );

	const uint8 writeBuf[5] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee };
	ASSERT_EQ( 2, smf.Write(writeBuf, 2) );
	ASSERT_EQ( 0xaa, buf[0] );
	ASSERT_EQ( 0xbb, buf[1] );
	ASSERT_EQ( 2, smf.GetPosition() );
	ASSERT_EQ( 2, smf.GetSize() );

	ASSERT_EQ( 2, smf.Write(writeBuf+2, 4) );
	ASSERT_EQ( 0xcc, buf[2] );
	ASSERT_EQ( 0xdd, buf[3] );
	ASSERT_EQ( 4, smf.GetPosition() );
	ASSERT_EQ( 4, smf.GetSize() );
}
