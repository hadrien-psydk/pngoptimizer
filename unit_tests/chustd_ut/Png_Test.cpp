#include "stdafx.h"

TEST(Png, Flavours)
{
	StringArray fileNames = Directory::GetFileNames("utfiles/PngSuite", "*.png");

	// Remove the logo file
	for(int i = fileNames.GetSize() - 1; i >= 0; --i)
	{
		if( fileNames[i].ToLowerCase().StartsWith("pngsuite") )
		{
			fileNames.RemoveAt(i);
		}
	}

	foreach(fileNames, i)
	{
		//////////////////////////////////////
		// Compute expectations
		String fileName = fileNames[i];
		String filePath = FilePath::Combine("utfiles/PngSuite", fileName);
		String abc = fileName.Mid(0, 3);
		String ni = fileName.Mid(3, 1);
		String ctNum = fileName.Mid(4, 1);
		String dp = fileName.Mid(6, 2);

		SCOPED_TRACE( ("fileName: " + fileName + " - index: " + String::FromInt(i)).GetBuffer());
		//std::cout << fileName << std::endl;

		bool expectedLoadResult = !abc.StartsWith("x");
		bool expectedInterlaced = (ni == "i");

		int colorType = 0;
		int bitDepth = 0;
		ASSERT_TRUE(ctNum.ToInt(colorType));
		ASSERT_TRUE(dp.ToInt(bitDepth));
		PixelFormat expectedPixelFormat = Png::GetPixelFormat(uint8(colorType), uint8(bitDepth));
		ASSERT_NE(PF_Unknown, expectedPixelFormat);

		int expectedWidth = 32;
		int expectedHeight = 32;
		if( abc.StartsWith("s") )
		{
			String wh = abc.Mid(1);
			ASSERT_TRUE(wh.ToInt(expectedWidth));
			expectedHeight = expectedWidth;
		}
		else if( abc == "cdf" )
		{
			expectedWidth = 8;
		}
		else if( abc == "cdh" )
		{
			expectedHeight = 8;
		}
		else if( abc == "cds" )
		{
			expectedWidth = 8;
			expectedHeight = 8;
		}

		if( abc == "xcs" || abc == "xhd" )
		{
			// Bad checksum for IDAT and IHDR, ignored
			expectedLoadResult = true;
		}
		//////////////////////////////////////
		// Verify expectations

		Png png;
		EXPECT_EQ(expectedLoadResult, png.Load(filePath));
		if( expectedLoadResult )
		{
			EXPECT_EQ(expectedInterlaced, png.IsInterlaced());
			EXPECT_EQ(expectedWidth, png.GetWidth());
			EXPECT_EQ(expectedHeight, png.GetHeight());

			if( abc == "ct1" )
			{
				// Check texts
				static const PngChunk_tEXt expectedTexts[] = {
					{ "Title", "" },
					{ "Author", "" },
					{ "Copyright", "" },
					{ "Description", "" },
					{ "Software", "" },
					{ "Disclaimer", "" },
				};
				int expectedTextCount = ARRAY_SIZE(expectedTexts);
				
				const Array<PngChunk_tEXt>& texts = png.GetTexts();
				
				EXPECT_EQ(expectedTextCount, texts.GetSize());
				foreach(texts, j)
				{
					char tmp1[300];
					expectedTexts[j].keyword.ToUtf8Z(tmp1);

					char tmp2[300];
					texts[j].keyword.ToUtf8Z(tmp2);
					EXPECT_STREQ( tmp1, tmp2 );
				}
			}
		}
	}
}

/*
TEST(Png, Burp)
{
	Png png;
	ASSERT_TRUE(png.Load("PngSuite/template.png"));
	PngDumpSettings ds;
	ds.width = png.GetWidth();
	ds.height = png.GetHeight();
	ds.pixelFormat = png.GetPixelFormat();
	ds.pBuffer = png.GetBuffer();
	Png::Dump("PngSuite/xcsn0g01.png", ds);
}
*/