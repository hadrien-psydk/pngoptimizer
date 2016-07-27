#include "stdafx.h"

TEST(ScanType, Int32)
{
	// Cas général
	int32 nAdvance = 0;
	int32 nResult = 0;
	bool bRet = ScanType(UTF16("123"), nAdvance, 'd', &nResult);
	ASSERT_TRUE(bRet == true);
	ASSERT_TRUE(nAdvance == 3);
	ASSERT_TRUE(nResult == 123);

	// Avec espaces
	nAdvance = 0;
	nResult = 0;
	bRet = ScanType(UTF16(" \t\n\r 123"), nAdvance, 'd', &nResult);
	ASSERT_TRUE(bRet == true);
	ASSERT_TRUE(nAdvance == 8);
	ASSERT_TRUE(nResult == 123);

	// Avec signe +
	nAdvance = 0;
	nResult = 0;
	bRet = ScanType(UTF16("+123"), nAdvance, 'd', &nResult);
	ASSERT_TRUE(bRet == true);
	ASSERT_TRUE(nAdvance == 4);
	ASSERT_TRUE(nResult == 123);

	// Avec signe -
	nAdvance = 0;
	nResult = 0;
	bRet = ScanType(UTF16("-123"), nAdvance, 'd', &nResult);
	ASSERT_TRUE(bRet == true);
	ASSERT_TRUE(nAdvance == 4);
	ASSERT_TRUE(nResult == -123);

	// Avec signe + et une espace
	nAdvance = 0;
	nResult = 0;
	bRet = ScanType(UTF16("+ 123"), nAdvance, 'd', &nResult);
	ASSERT_TRUE(bRet == false);
	ASSERT_TRUE(nAdvance == 0); // Doit rester inchangé
	ASSERT_TRUE(nResult == 0); // Doit rester inchangé

	// Hexadécimal
	nAdvance = 0;
	nResult = 0;
	bRet = ScanType(UTF16("aA12cF"), nAdvance, 'x', &nResult);
	ASSERT_TRUE(bRet == true);
	ASSERT_TRUE(nAdvance == 6);
	ASSERT_TRUE(nResult == 0x0aA12cF);

	// Binaire
	nAdvance = 0;
	nResult = 0;
	bRet = ScanType(UTF16("10110001"), nAdvance, 'b', &nResult);
	ASSERT_TRUE(bRet == true);
	ASSERT_TRUE(nAdvance == 8);
	ASSERT_TRUE(nResult == 0x00B1);
}

TEST(ByteArrayTest, Find)
{
	ByteArray aData;
	aData.Add('x');
	aData.Add('a');
	ASSERT_TRUE(aData.Find((uint8*)"z", 2) == -1);
	ASSERT_TRUE(aData.Find((uint8*)"x", 0) == 0);
	ASSERT_TRUE(aData.Find((uint8*)"a", 1) == 1);
	ASSERT_TRUE(aData.Find((uint8*)"xa", 2) == 0);
	ASSERT_TRUE(aData.Find((uint8*)"uvw", 3) == -1);

	aData.Add('b');
	aData.Add('c');
	ASSERT_TRUE(aData.Find((uint8*)"abc", 3) == 1);
}

TEST(MathTest, Log2)
{
	int32 n0, n1, n2, n3;

	n0 = Math::Log2(0);
	n1 = Math::Log2(1);
	n2 = Math::Log2(2);
	n3 = Math::Log2(3);
	
	ASSERT_TRUE( n0 == 0);
	ASSERT_TRUE( n1 == 0);
	ASSERT_TRUE( n2 == 1);
	ASSERT_TRUE( n3 == 1);
	
	for(int32 i = 4; i <= 7; ++i)
	{
		ASSERT_TRUE( Math::Log2(i) == 2);
	}

	for(int32 i = 8; i <= 15; ++i)
	{
		ASSERT_TRUE( Math::Log2(i) == 3);
	}

	for(int32 i = 16; i <= 31; ++i)
	{
		ASSERT_TRUE( Math::Log2(i) == 4);
	}

	for(int32 i = 32; i <= 63; ++i)
	{
		ASSERT_TRUE( Math::Log2(i) == 5);
	}

	for(int32 i = 64; i <= 127; ++i)
	{
		ASSERT_TRUE( Math::Log2(i) == 6);
	}

	for(int32 i = 128; i <= 255; ++i)
	{
		ASSERT_TRUE( Math::Log2(i) == 7);
	}

	for(int32 i = 256; i <= 511; ++i)
	{
		ASSERT_TRUE( Math::Log2(i) == 8);
	}
}

class ArrElem
{
public:
	ArrElem()  { m_nValue = 0x11223344; }
	~ArrElem() { m_nValue = 0xaabbccdd; }
public:
	uint32 m_nValue;
};

TEST(ArrayTest, Misc)
{
	////////////////////////////////////////////////////////////////////////////////////////
	// Test array copy big from small, with constructors and destructors correctly called
	Array<ArrElem> a1;
	a1.EnsureCapacity(10);
	
	// We do not use the [] operator so we can read/write after the array size
	a1.GetBuffer()[0].m_nValue = 0xfdfdfdfd;
	a1.GetBuffer()[1].m_nValue = 0xfdfdfdfd;
	a1.GetBuffer()[2].m_nValue = 0xfdfdfdfd;
	a1.GetBuffer()[3].m_nValue = 0xfdfdfdfd;

	a1.SetSize(2);

	ASSERT_TRUE( a1.GetBuffer()[0].m_nValue == 0x11223344 );
	ASSERT_TRUE( a1.GetBuffer()[1].m_nValue == 0x11223344 );
	ASSERT_TRUE( a1.GetBuffer()[2].m_nValue == 0xfdfdfdfd );
	ASSERT_TRUE( a1.GetBuffer()[3].m_nValue == 0xfdfdfdfd );

	Array<ArrElem> a2;
	a2.SetSize(1);
	a2[0].m_nValue = 0x55667788;

	a1 = a2; // Assignment

	ASSERT_TRUE( a1.GetBuffer()[0].m_nValue == 0x55667788 );
	ASSERT_TRUE( a1.GetBuffer()[1].m_nValue == 0xaabbccdd );
	ASSERT_TRUE( a1.GetBuffer()[2].m_nValue == 0xfdfdfdfd );
	ASSERT_TRUE( a1.GetBuffer()[3].m_nValue == 0xfdfdfdfd );
	////////////////////////////////////////////////////////////////////////////////////////
}

TEST(Color32Test, Misc)
{
	// 0 11111 11110 11100 => 0111_1111 1101_1100 => 7FDC
	uint16 nValue = 0x7FDC;
	Color32 col = Color32::From16bppRgb555(nValue);

	uint8 r, g, b, a;
	col.ToRgba(r, g, b, a);

	// 11111 => 11111111
	ASSERT_TRUE( r == 0xFF );
	
	// 11110 => 11110111
	ASSERT_TRUE( g == 0xF7 );

	// 11100 => 11100111
	ASSERT_TRUE( b == 0xE7 );

	ASSERT_TRUE( a == 0xFF );

	// 0 10011 10111 01011 => 0100 1110 1110 1011 => 0x4EEB
	nValue = 0x4EEB;
	col = Color32::From16bppRgb555(nValue);

	col.ToRgba(r, g, b, a);

	// 10011 => 10011 100
	ASSERT_TRUE( r == 0x9C );
	
	// 10111 => 10111 101
	ASSERT_TRUE( g == 0xBD );

	// 01011 => 01011 010
	ASSERT_TRUE( b == 0x5A );

	ASSERT_TRUE( a == 0xFF );
}

TEST(MemoryTest, Misc)
{
	{
		uint8 a1[5] = { 1, 2, 3, 0x33, 0x44 };
		uint8 a2[5] = {0xdd, 0xee, 0xff, 0xaa, 0xbb };
		
		Memory::Set(a2, 0x11, 3);
		ASSERT_TRUE(a2[0] == 0x11 && a2[1] == 0x11 && a2[2] == 0x11 && a2[3] == 0xaa && a2[4] == 0xbb);
		
		Memory::Copy(a2, a1, 3);
		ASSERT_TRUE(a2[0] == 0x1 && a2[1] == 0x2 && a2[2] == 0x3 && a2[3] == 0xaa && a2[4] == 0xbb);

		Memory::Zero(a2, 3);
		ASSERT_TRUE(a2[0] == 0x0 && a2[1] == 0x0 && a2[2] == 0x0 && a2[3] == 0xaa && a2[4] == 0xbb);
	}

	uint8 a3[6] = {   1,    2,    3,    4, 0x33, 0x44 };
	uint8 a4[6] = {0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee };

	Memory::Set(a4, 0x11, 4);
	ASSERT_TRUE(a4[0] == 0x11 && a4[1] == 0x11 && a4[2] == 0x11 && a4[3] == 0x11 && a4[4] == 0xdd && a4[5] == 0xee);
	
	Memory::Copy(a4, a3, 4);
	ASSERT_TRUE(a4[0] == 0x1 && a4[1] == 0x2 && a4[2] == 0x3 && a4[3] == 0x04 && a4[4] == 0xdd && a4[5] == 0xee);

	Memory::Zero(a4, 4);
	ASSERT_TRUE(a4[0] == 0x0 && a4[1] == 0x0 && a4[2] == 0x0 && a4[3] == 0x0 && a4[4] == 0xdd && a4[5] == 0xee);
}

TEST(RandomTest, Misc)
{
	Random rnd;

	FixArray<int32, 21> aCounters = { { 0 } };

	for(int i = 0; i < 200; ++i)
	{
		int32 nVal = rnd.GetNext(0, 20);
		aCounters[nVal]++;
	}
	/*
	foreach(aCounters, iCounter)
	{
		const int32 nPlop = aCounters[iCounter];
		for(int i = 0; i < nPlop; ++i)
		{
			Console::Write("#");
		}
		Console::WriteLine("");
	}*/
}

TEST(MemIniFileTest, Dump)
{
	MemIniFile ini;

	ini.SetSection("Engine");
	ini.SetBool("AvoidGreyWithSimpleTransparency", true);
	ini.SetBool("BackupOldPngFiles", false);
	ini.SetBool("KeepBackgroundColor", false);
	ini.SetBool("KeepInterlacing", false);

	ini.SetSection("Screenshots");
	ini.SetBool("UseDefaultDir", true);
	ini.SetString("CustomDir", "D:\\Étoile des neiges");
	ini.SetBool("AskForFileName", false);
	ini.SetBool("MaximizeCompression", false);

	ini.SetSection("Window");
	ini.SetInt("X", 15);
	ini.SetInt("Y", 20);
	ini.SetInt("Width", 400);
	ini.SetInt("Height", 300);

	ini.SetCommentLine1("PngOptimizer configuration file");
	ini.SetCommentLine2("This file is encoded in UTF-8");

	ini.Dump("test.ini", false);
	ini.Dump("test-crlf.ini", true);
}

TEST(MemIniFileTest, Load)
{
	MemIniFile ini;
	ASSERT_TRUE( ini.Load("test.ini"));

	ASSERT_TRUE( ini.SetSection("Engine"));
	
	bool bAvoidGreyWithSimpleTransparency;
	ASSERT_TRUE( ini.GetBool("AvoidGreyWithSimpleTransparency", bAvoidGreyWithSimpleTransparency));
	ASSERT_TRUE( bAvoidGreyWithSimpleTransparency == true);

	bool bBackupOldPngFiles;
	ASSERT_TRUE( ini.GetBool("BackupOldPngFiles", bBackupOldPngFiles));
	ASSERT_TRUE( bBackupOldPngFiles == false);

	ASSERT_TRUE( ini.SetSection("Screenshots"));
	
	String strCustomDir;
	ASSERT_TRUE( ini.GetString("CustomDir", strCustomDir));
	ASSERT_TRUE( strCustomDir == "D:\\Étoile des neiges");

	ASSERT_TRUE( ini.SetSection("Window"));
	
	int32 nX;
	ASSERT_TRUE( ini.GetInt("X", nX));
	ASSERT_TRUE( nX == 15);
}

TEST(TextEncodingTest, Misc)
{
	ByteArray bytes;
	String result = TextEncoding::Utf8().BytesToString(bytes);
	ASSERT_TRUE(result == "");

	static const uint8 raw[] = { 'a', 'b' };
	bytes.Set(raw, sizeof(raw));
	result = TextEncoding::Utf8().BytesToString(bytes);
	ASSERT_TRUE(result == "ab");

}

TEST(StringArray, Sort)
{
	StringArray a;
	a.Add("plop");
	a.Add("onk");
	a.Add("zergh");
	a.Add("boo");

	// Deep copy should occur
	StringArray b = a;
	ASSERT_TRUE( b.GetPtr() != a.GetPtr() );
	StringArray c = b.Sort();
	ASSERT_EQ( 4, c.GetSize() );
	ASSERT_TRUE( c[0] == "boo" );
	ASSERT_TRUE( c[1] == "onk" );
	ASSERT_TRUE( c[2] == "plop" );
	ASSERT_TRUE( c[3] == "zergh" );
}

struct Car
{
	int color;
	Car(int c) : color(c) {}
	~Car() { color = -1; }
};

TEST(PtrArray, CopyConstructor)
{
	PtrArray<Car> cars;
	cars.Add( new Car(1) );
	cars.Add( new Car(2) );

	PtrArray<Car> newCars(cars);
	cars.SetSize(0);

	ASSERT_EQ( 1, newCars[0]->color );
	ASSERT_EQ( 2, newCars[1]->color );
}
