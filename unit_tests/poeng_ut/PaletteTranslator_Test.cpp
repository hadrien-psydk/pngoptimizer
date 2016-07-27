#include "stdafx.h"

TEST(PaletteTranslator, BuildSortPopulation)
{
	uint32 colCounts[256];
	colCounts[0] = 100; // --> 3
	colCounts[1] = 101; // --> 2
	colCounts[2] = 102; // --> 1
	colCounts[3] = 103; // --> 0

	Color32 col100(1, 0, 1);
	Color32 col101(0, 1, 0);
	Color32 col102(0, 0, 1);
	Color32 col103(0, 1, 1);

	Palette pal;
	pal.m_count = 4;
	pal[0] = col100;
	pal[1] = col101;
	pal[2] = col102;
	pal[3] = col103;

	PaletteTranslator pt;
	pt.BuildSortPopulation(pal, colCounts);

	ASSERT_TRUE( col103 == pal[0] );
	ASSERT_TRUE( col102 == pal[1] );
	ASSERT_TRUE( col101 == pal[2] );
	ASSERT_TRUE( col100 == pal[3] );

	ASSERT_EQ( 103u, colCounts[0] );
	ASSERT_EQ( 102u, colCounts[1] );
	ASSERT_EQ( 101u, colCounts[2] );
	ASSERT_EQ( 100u, colCounts[3] );
}

TEST(PaletteTranslator, BuildSortPopulation2)
{
	uint32 colCounts[256] = {
		662512, //  0
		 46278, //  6
		  3344, // 13
		  2723, // 15
		  3140, // 14
		  3427, // 12
		 10729, //  7
		 54691, //  3
		  1924, // 16
		  1502, // 20
		  1763, // 17
		  6214, //  9
		 46457, //  5
		  1695, // 18
		  1581, // 19
		  4623, // 11
		 50134, //  4
		  1292, // 21
		  4658, // 10
		 61208, //  2
		  6616, //  8
		443410, //  1
		     0
	};

	uint32 expColCounts[256] = {
		662512,
		443410,
		 61208,
		 54691,
		 50134,
		 46457,
		 46278,
		 10729,
		  6616,
		  6214,
		  4658,
		  4623,
		  3427,
		  3344,
		  3140,
		  2723,
		  1924,
		  1763,
		  1695,
		  1581,
		  1502,
		  1292,
		    0
	};

	Palette pal;
	pal.m_count = 22;
	pal[ 0].SetRgba(255, 255, 255,   0);
	pal[ 1].SetRgba(  0,   0,   0, 255);
	pal[ 2].SetRgba( 51,   0,   0, 255);
	pal[ 3].SetRgba(102,   0,   0, 255);
	pal[ 4].SetRgba(153,   0,   0, 255);
	pal[ 5].SetRgba(204,   0,   0, 255);
	pal[ 6].SetRgba(255,   0,   0, 255);
	pal[ 7].SetRgba( 51,  51,  51, 255);
	pal[ 8].SetRgba(102,  51,  51, 255);
	pal[ 9].SetRgba(153,  51,  51, 255);
	pal[10].SetRgba(204,  51,  51, 255);
	pal[11].SetRgba(255,  51,  51, 255);
	pal[12].SetRgba(102, 102, 102, 255);
	pal[13].SetRgba(153, 102, 102, 255);
	pal[14].SetRgba(204, 102, 102, 255);
	pal[15].SetRgba(255, 102, 102, 255);
	pal[16].SetRgba(153, 153, 153, 255);
	pal[17].SetRgba(204, 153, 153, 255);
	pal[18].SetRgba(255, 153, 153, 255);
	pal[19].SetRgba(204, 204, 204, 255);
	pal[20].SetRgba(255, 204, 204, 255);
	pal[21].SetRgba(255, 255, 255, 255);
	pal[22].SetRgba(  0,   0,   0, 255);
	pal[23].SetRgba(  0,   0,   0, 255);

	Palette exp;
	exp.m_count = 22;
	exp[ 0].SetRgba(255, 255, 255,   0);
	exp[ 1].SetRgba(255, 255, 255, 255);
	exp[ 2].SetRgba(204, 204, 204, 255);
	exp[ 3].SetRgba( 51,  51,  51, 255);
	exp[ 4].SetRgba(153, 153, 153, 255);
	exp[ 5].SetRgba(102, 102, 102, 255);
	exp[ 6].SetRgba(  0,   0,   0, 255);
	exp[ 7].SetRgba(255,   0,   0, 255);
	exp[ 8].SetRgba(255, 204, 204, 255);
	exp[ 9].SetRgba(255,  51,  51, 255);
	exp[10].SetRgba(255, 153, 153, 255);
	exp[11].SetRgba(255, 102, 102, 255);
	exp[12].SetRgba(204,   0,   0, 255);
	exp[13].SetRgba( 51,   0,   0, 255);
	exp[14].SetRgba(153,   0,   0, 255);
	exp[15].SetRgba(102,   0,   0, 255);
	exp[16].SetRgba(102,  51,  51, 255);
	exp[17].SetRgba(204,  51,  51, 255);
	exp[18].SetRgba(153, 102, 102, 255);
	exp[19].SetRgba(204, 102, 102, 255);
	exp[20].SetRgba(153,  51,  51, 255);
	exp[21].SetRgba(204, 153, 153, 255);
	exp[22].SetRgba(  0,   0,   0, 255);
	exp[23].SetRgba(  0,   0,   0, 255);

	PaletteTranslator populationTranslator;
	populationTranslator.BuildSortPopulation(pal, colCounts);

	ASSERT_TRUE(pal == exp);
	ASSERT_TRUE( memcmp(colCounts, expColCounts, 22*sizeof(colCounts[0])) == 0 );
}

TEST(PaletteTranslator, BuildSortLuminance)
{
	//uint32 y = r * 299 + g * 587 + b * 114;

	Color32 col0(0, 0, 10);
	Color32 col1(0, 0, 11);
	Color32 col2(10, 0, 0);
	Color32 col3(11, 0, 0);
	Color32 col4(0, 10, 0);
	Color32 col5(0, 11, 0);

	Palette pal;
	pal.m_count = 6;
	pal[0] = col3;
	pal[1] = col5;
	pal[2] = col1;
	pal[3] = col2;
	pal[4] = col4;
	pal[5] = col0;

	Palette exp;
	exp.m_count = 6;
	exp[0] = col0;
	exp[1] = col1;
	exp[2] = col2;
	exp[3] = col3;
	exp[4] = col4;
	exp[5] = col5;

	PaletteTranslator pt;
	pt.BuildSortLuminance(pal, nullptr);

	ASSERT_TRUE( pal == exp );
}

TEST(PaletteTranslator, BuildUnusedColors)
{
	uint32 colCounts[256] = { 0 };
	colCounts[0] = 1000;
	colCounts[1] = 0;
	colCounts[2] = 1000;
	colCounts[3] = 1000;

	uint32 expColCounts[256] = { 0 };
	expColCounts[0] = 1000;
	expColCounts[1] = 1000;
	expColCounts[2] = 1000;
	expColCounts[3] = 0;

	Color32 col00(0, 0, 10);
	Color32 col11(0, 0, 11);
	Color32 col21(10, 0, 0);
	Color32 col32(11, 0, 0);

	Palette pal;
	pal.m_count = 4;
	pal[0] = col00;
	pal[1] = col11;
	pal[2] = col21;
	pal[3] = col32;

	Palette exp;
	exp.m_count = 3;
	exp[0] = col00;
	exp[1] = col21;
	exp[2] = col32;

	PaletteTranslator pt;
	pt.BuildUnusedColors(pal, colCounts);

	ASSERT_TRUE( pal == exp );
	ASSERT_TRUE( memcmp(colCounts, expColCounts, 3*sizeof(colCounts[0])) == 0 );
}

TEST(PaletteTranslator, BuildDuplicatedColors)
{
	Color32 col0(11, 12, 13);
	Color32 col1(14, 15, 16);
	Color32 col2(20, 21, 22, 0);
	Color32 col3(11, 12, 13);
	Color32 col4(30, 31, 32, 0);

	Palette pal;
	pal.m_count = 5;
	pal[0] = col0;
	pal[1] = col1;
	pal[2] = col2;
	pal[3] = col3;
	pal[4] = col4;

	Palette exp;
	exp.m_count = 3;
	exp[0] = col0;
	exp[1] = col1;
	exp[2] = col2;

	uint32 colCounts[256] = { 1, 10, 100, 1000, 10000 };
	uint32 expectedColCounts[256] = { 1001, 10, 10100, 0, 0 };

	PaletteTranslator pt;
	pt.BuildDuplicatedColors(pal, colCounts);
	ASSERT_TRUE( pal == exp );
	ASSERT_TRUE( memcmp(colCounts, expectedColCounts, sizeof(colCounts)) == 0 );

	uint8 pixels[] = { 0, 1, 2, 3, 4 };
	const uint8 expectedPixels[] = { 0, 1, 2, 0, 2 };
	
	pt.Translate(pixels, sizeof(pixels));
	
	ASSERT_TRUE( memcmp(pixels, expectedPixels, sizeof(pixels)) == 0 );
}
