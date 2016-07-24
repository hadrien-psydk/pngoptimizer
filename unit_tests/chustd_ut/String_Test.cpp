#include "stdafx.h"

TEST(String, Trim)
{
	// Same string should be returned
	String str1 = "abc";
	String str2 = str1.Trim();
	ASSERT_TRUE(str1.GetBuffer() == str2.GetBuffer());

	// Trim gauche
	String str3 = "  abc";
	String str4 = str3.Trim();
	ASSERT_TRUE(str4 == "abc");

	// Trim droite
	String str5 = "abc  ";
	String str6 = str5.Trim();
	ASSERT_TRUE(str6 == "abc");

	// Trim double
	String str7 = "  abc  ";
	String str8 = str7.Trim();
	ASSERT_TRUE(str8 == "abc");

	// Trim multi-caractères
	String str9 = " \r\n\t abc \n\t\r ";
	String strA = str9.Trim();
	ASSERT_TRUE(strA == "abc");
}

TEST(String, SplitByChar)
{
	String str1 = "";
	StringArray astr1 = str1.Split(',');
	ASSERT_TRUE( astr1.GetSize() == 0 );

	String str2 = "abc";
	StringArray astr2 = str2.Split(',');
	ASSERT_TRUE( astr2.GetSize() == 1 && astr2[0] == "abc" );

	String str3 = "abc,def";
	StringArray astr3 = str3.Split(',');
	ASSERT_TRUE( astr3.GetSize() == 2 && astr3[0] == "abc" && astr3[1] == "def" );

	String str4 = "abc,";
	StringArray astr4 = str4.Split(',');
	ASSERT_TRUE( astr4.GetSize() == 2 && astr4[0] == "abc" && astr4[1].IsEmpty() );

	String str5 = ",def";
	StringArray astr5 = str5.Split(',');
	ASSERT_TRUE( astr5.GetSize() == 2 && astr5[0].IsEmpty() && astr5[1] == "def" );

	// Split with surrogates
	uint32 nUcs4 = 0x2ffff;
	uint16 nA, nB;
	String::CodePointToCodeUnits(nUcs4, nA, nB);

	const wchar sz[] = {'a', 'b', 'c', nA, nB, 'd', 'e', 'f', 0};
	String str6 = sz;
	StringArray astr6 = str6.Split(nUcs4);
	ASSERT_TRUE( astr6.GetSize() == 2 && astr6[0] == "abc" && astr6[1] == "def" );
}

TEST(String, SplitByString)
{
	String str1 = "";
	StringArray astr1 = str1.Split(",");
	ASSERT_TRUE(astr1.GetSize() == 0);

	String str2 = "abc";
	StringArray astr2 = str2.Split(",");
	ASSERT_TRUE(astr2.GetSize() == 1 && astr2[0] == "abc");

	String str3 = "abc,def";
	StringArray astr3 = str3.Split(",");
	ASSERT_TRUE(astr3.GetSize() == 2 && astr3[0] == "abc" && astr3[1] == "def");

	String str4 = "abc,";
	StringArray astr4 = str4.Split(",");
	ASSERT_TRUE(astr4.GetSize() == 2 && astr4[0] == "abc" && astr4[1].IsEmpty());

	String str5 = ",def";
	StringArray astr5 = str5.Split(",");
	ASSERT_TRUE(astr5.GetSize() == 2 && astr5[0].IsEmpty() && astr5[1] == "def");

	String str6 = "abc:.:def";
	StringArray astr6 = str6.Split(":.:");
	ASSERT_TRUE(astr6.GetSize() == 2 && astr6[0] == "abc" && astr6[1] == "def");

	String str7 = "abc:.:";
	StringArray astr7 = str7.Split(":.:");
	ASSERT_TRUE(astr7.GetSize() == 2 && astr7[0] == "abc" && astr7[1].IsEmpty());

	String str8 = ":.:def";
	StringArray astr8 = str8.Split(":.:");
	ASSERT_TRUE(astr8.GetSize() == 2 && astr8[0].IsEmpty() && astr8[1] == "def");
}

TEST(String, Utf8)
{
	// Split with surrogates
	uint32 nUcs4 = 0x2ffff;
	uint16 nA, nB;
	String::CodePointToCodeUnits(nUcs4, nA, nB);

	const wchar sz[] = {'a', 0x00E9, 0x20AC, nA, nB, 'z', 0};
	String str1 = sz;
	ByteArray aBytes1 = str1.ToBytes(TextEncoding::Utf8());
	ASSERT_TRUE(aBytes1.GetSize() == 11);

	ByteArray aBytes2 = str1.ToBytes(TextEncoding::Utf8(), true);
	ASSERT_TRUE(aBytes2.GetSize() == 12);

	String str2 = String::FromBytes(aBytes1, TextEncoding::Utf8());
	ASSERT_TRUE(str1 == str2);

	String str3 = TextEncoding::Utf8().BytesToString(aBytes1);
	ASSERT_TRUE(str1 == str3);
}

TEST(String, InsertAt)
{
	String str1 = "abc";
	String str2 = "xy";

	String str3 = str1.InsertAt(0, str2);
	ASSERT_TRUE(str3 == "xyabc");

	String str4 = str1.InsertAt(3, str2);
	ASSERT_TRUE(str4 == "abcxy");

	String str5 = str1.InsertAt(1, str2);
	ASSERT_TRUE(str5 == "axybc");
}

TEST(String, ToUpperCase)
{
	////////////////////////////////////////
	// Test avec code points <= 0xffff

	// Page 00
	String str1 =  "abcZf";
	String str2 = str1.ToUpperCase();
	ASSERT_TRUE(str2 == "ABCZF");

	// Page 01
	String str3 = UTF16("\x0101\x0102\x0135");
	String str4 = str3.ToUpperCase();
	ASSERT_TRUE(str4 == UTF16("\x0100\x0102\x0134"));

	////////////////////////////////////////
	// Test avec code points > 0xffff

	// Split with surrogates
	uint16 nA, nB;
	String::CodePointToCodeUnits(0x10428, nA, nB);
	uint16 nC, nD;
	String::CodePointToCodeUnits(0x1044F, nC, nD);
	uint16 nE, nF;
	String::CodePointToCodeUnits(0x11000, nE, nF);

	const wchar sz[] = {nA, nB, nC, nD, nE, nF, 0};

	String str5 =  sz;
	String str6 = str5.ToUpperCase();
	
	StringBuilder sb7;
	sb7 += 0x10400;
	sb7 += 0x10427;
	sb7 += 0x11000;
	String str7 = sb7.ToString();

	ASSERT_TRUE(str6 == str7);

	////////////////////////////////////////
	// Test avec code points de mapping 1:N
	String str8 = UTF16("\x00df"); // es-zed
	String str9 = str8.ToUpperCase();
	ASSERT_TRUE(str9 == "SS");

	String str10 = UTF16("\x1FE7");
	String str11 = str10.ToUpperCase();
	ASSERT_TRUE(str11 == UTF16("\x03A5\x0308\x0342"));

	String str12 = UTF16("\xFB04");
	String str13 = str12.ToUpperCase();
	ASSERT_TRUE(str13 == UTF16("\x0046\x0046\x004C"));
}

TEST(String, ToLowerCase)
{
	////////////////////////////////////////
	// Test avec code points <= 0xffff

	// Page 00
	String str1 =  "ABCzF";
	String str2 = str1.ToLowerCase();
	ASSERT_TRUE(str2 == "abczf");

	// Page 01
	String str3 =  UTF16("\x0100\x0103\x0134");
	String str4 = str3.ToLowerCase();
	ASSERT_TRUE(str4 == UTF16("\x0101\x0103\x0135"));
	
	////////////////////////////////////////
	// Test avec code points > 0xffff

	// Split with surrogates
	uint16 nA, nB;
	String::CodePointToCodeUnits(0x10400, nA, nB);
	uint16 nC, nD;
	String::CodePointToCodeUnits(0x10427, nC, nD);
	uint16 nE, nF;
	String::CodePointToCodeUnits(0x11000, nE, nF);

	const wchar sz[] = {nA, nB, nC, nD, nE, nF, 0};

	String str5 =  sz;
	String str6 = str5.ToLowerCase();
	
	StringBuilder sb7;
	sb7 += 0x10428;
	sb7 += 0x1044F;
	sb7 += 0x11000;
	String str7 = sb7.ToString();

	ASSERT_TRUE(str6 == str7);

	////////////////////////////////////////
	// Test avec code points de mapping 1:N
	/*
	String str8 = "\x0130"; // I capital avec point --> 0x0069 seul ou 0x0069 0x0307
	String str9 = str8.ToLowerCase();
	ASSERT_TRUE(str9 == "\x0069\x0307");
	*/
}

TEST(String, FromInt)
{
	ASSERT_TRUE("12" == String::FromInt(12));
	ASSERT_TRUE("c" == String::FromInt(12, 'x'));
	ASSERT_TRUE("C" == String::FromInt(12, 'X'));
	ASSERT_TRUE("1100" == String::FromInt(12, 'b'));

	ASSERT_TRUE("  12" == String::FromInt(12, 'd', 4));
	ASSERT_TRUE("0012" == String::FromInt(12, 'd', 4, '0'));

	ASSERT_TRUE("   c" == String::FromInt(12, 'x', 4));
	ASSERT_TRUE("000c" == String::FromInt(12, 'x', 4, '0'));

	ASSERT_TRUE("   C" == String::FromInt(12, 'X', 4));
	ASSERT_TRUE("000C" == String::FromInt(12, 'X', 4, '0'));

	ASSERT_TRUE("  1100" == String::FromInt(12, 'b', 6));
	ASSERT_TRUE("001100" == String::FromInt(12, 'b', 6, '0'));
}

TEST(String, FromInt64)
{
	ASSERT_TRUE("12" == String::FromInt64(12));
	ASSERT_TRUE("c" == String::FromInt64(12, 'x'));
	ASSERT_TRUE("C" == String::FromInt64(12, 'X'));
	ASSERT_TRUE("1100" == String::FromInt64(12, 'b'));

	ASSERT_TRUE("  12" == String::FromInt64(12, 'd', 4));
	ASSERT_TRUE("0012" == String::FromInt64(12, 'd', 4, '0'));

	ASSERT_TRUE("   c" == String::FromInt64(12, 'x', 4));
	ASSERT_TRUE("000c" == String::FromInt64(12, 'x', 4, '0'));

	ASSERT_TRUE("   C" == String::FromInt64(12, 'X', 4));
	ASSERT_TRUE("000C" == String::FromInt64(12, 'X', 4, '0'));

	ASSERT_TRUE("  1100" == String::FromInt64(12, 'b', 6));
	ASSERT_TRUE("001100" == String::FromInt64(12, 'b', 6, '0'));
}

TEST(String, UnifyNewlines)
{
	String str = "";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Dos) == "" );

	str = "abc";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Dos) == "abc" );

	str = "A\r\nB\r\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Dos) == "A\r\nB\r\nC" );

	str = "A\r\nB\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Dos) == "A\r\nB\r\nC" );

	str = "A\nB\r\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Dos) == "A\r\nB\r\nC" );

	str = "A\nB\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Dos) == "A\r\nB\r\nC" );

	////////////////////////////

	str = "abc";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Unix) == "abc" );

	str = "A\r\nB\r\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Unix) == "A\nB\nC" );

	str = "A\r\nB\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Unix) == "A\nB\nC" );

	str = "A\nB\r\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Unix) == "A\nB\nC" );

	str = "A\nB\nC";
	ASSERT_TRUE( str.UnifyNewlines(String::NT_Unix) == "A\nB\nC" );
}

TEST(String, FromUtf8)
{
	String str = String::FromUtf8("ab\0c", 4);
	ASSERT_TRUE( str.GetLength() == 4 );
	ASSERT_TRUE( str == String("ab\0c", 4) );
}
