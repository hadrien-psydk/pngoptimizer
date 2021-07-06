
#include "stdafx.h"

class TestImg;

class TestFrame : public ApngFrame
{
public:
	bool m_ownPalette;
	Palette m_palette;

	TestFrame(TestImg* pOwner) : ApngFrame((Png*)pOwner)
	{
		m_ownPalette = false;
	}

	virtual const Palette& GetPalette() const
	{
		if( m_ownPalette )
		{
			return m_palette;
		}
		return ApngFrame::GetPalette();
	}

};

class TestImg : public ImageFormat
{
public:
	PixelFormat m_pf;
	Palette m_palette;
	Array<TestFrame> m_animFrames;
	bool m_hasDefaultImage;
	int32 m_loopCount;

public:
	TestImg()
	{
		m_hasDefaultImage = false;
		m_loopCount = 0;
	}
	void SetWidth(int width) { m_width = width; }
	void SetHeight(int height) { m_height = height; }
	void SetPixels(const uint8* pPixels, int size) { m_pixels.Assign(pPixels, size); }

	virtual PixelFormat GetPixelFormat() const { return m_pf; }
	const Palette& GetPalette() const { return m_palette; }
	bool LoadFromFile(IFile&) { return false; }

	virtual bool  IsAnimated() const { return m_animFrames.GetSize() > 0; }
	virtual bool  HasDefaultImage() const { return m_hasDefaultImage; }
	virtual int32 GetFrameCount() const { return m_animFrames.GetSize(); }
	virtual const AnimFrame* GetAnimFrame(int32 nIndex) const { return &m_animFrames[nIndex]; }
	virtual int32 GetLoopCount() const { return m_loopCount; } // 0 = infinite
};

// Test the case with a GIF which frames are smaller than the drawing area
TEST(POEngine, OptimizeAnimated_BigTransparentFrame)
{
	TestImg img;
	img.m_pf = PF_8bppIndexed;
	img.SetWidth(7); // 2 + 3 + 2
	img.SetHeight(7);

	// The palette does provide a transparent color, this one should be used
	img.m_palette.m_count = 2;
	img.m_palette[0].SetRgba(255, 255, 255, 0);
	img.m_palette[1].SetRgba(100, 200, 100, 255);

	TestFrame frame0(&img);
	frame0.m_fctl.offsetX = 2;
	frame0.m_fctl.offsetY = 2;
	frame0.m_fctl.width = 3;
	frame0.m_fctl.height = 3;
	frame0.m_fctl.delayFracNumerator = 100;
	frame0.m_pixels.SetSize(3 * 3);
	frame0.m_pixels.Fill(1); // Fully opaque
	img.m_animFrames.Add(frame0);

	POEngine engine;
	PngDumpData dd;
	static const String resultFilePath = "result.png";
	Png png;

	File::Delete(resultFilePath);
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );

	// We should get a APNG image with the same palette
	// A default image of 7x7, containing the first 3x3 frame, with transparent color (index 0) around
	ASSERT_TRUE( png.Load(resultFilePath) );

	// One single frame : no animation information at all
	ASSERT_TRUE( !png.IsAnimated() );
	ASSERT_TRUE( png.HasDefaultImage() );
	ASSERT_EQ( 7, png.GetWidth() );
	ASSERT_EQ( 7, png.GetHeight() );
	ASSERT_TRUE( png.GetPixelFormat() == PF_1bppIndexed );
	ASSERT_TRUE( png.GetPalette().m_count == 2 );

	static const uint8 frame0Bytes[] = {
		BYTE_FROM_BITS(0, 0, 0, 0, 0, 0, 0,  0),
		BYTE_FROM_BITS(0, 0, 0, 0, 0, 0, 0,  0),
		BYTE_FROM_BITS(0, 0, 1, 1, 1, 0, 0,  0),
		BYTE_FROM_BITS(0, 0, 1, 1, 1, 0, 0,  0),
		BYTE_FROM_BITS(0, 0, 1, 1, 1, 0, 0,  0),
		BYTE_FROM_BITS(0, 0, 0, 0, 0, 0, 0,  0),
		BYTE_FROM_BITS(0, 0, 0, 0, 0, 0, 0,  0)
	};
	const uint8* pLoaded = png.GetPixels().GetReadPtr();
	ASSERT_TRUE( Memory::Equals(pLoaded, frame0Bytes, sizeof(frame0Bytes)) );

	/////////////////////////////////////////////////////////
	// Now add a 2nd and frame and redump
	img.m_animFrames[0].m_fctl.disposal = AnimFrame::DispClearToTransBlack;

	TestFrame frame1(&img);
	frame1.m_fctl.offsetX = 3;
	frame1.m_fctl.offsetY = 3;
	frame1.m_fctl.width = 1;
	frame1.m_fctl.height = 1;
	frame1.m_fctl.delayFracNumerator = 100;
	frame1.m_pixels.SetSize(1 * 1);
	frame1.m_pixels.Fill(1); // Fully opaque
	img.m_animFrames.Add(frame1);

	File::Delete(resultFilePath);
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );

	// We should get a APNG image with the same palette
	// A default image of 7x7, containing the first 3x3 frame, with transparent color (index 0) around
	ASSERT_TRUE( png.Load(resultFilePath) );

	// One single frame : no animation information at all
	ASSERT_TRUE( png.IsAnimated() );
	ASSERT_TRUE( !png.HasDefaultImage() );
	ASSERT_TRUE( png.GetWidth() == 7 );
	ASSERT_TRUE( png.GetHeight() == 7 );
	ASSERT_TRUE( png.GetFrameCount() == 2 );
	ASSERT_TRUE( png.GetPixelFormat() == PF_1bppIndexed );

	static const uint8 frame1Bytes[] = {
		BYTE_FROM_BITS(1, 0, 0, 0, 0, 0, 0,  0)
	};
	const uint8* pFramePixels = png.GetAnimFrame(0)->GetPixels().GetReadPtr();
	ASSERT_TRUE( Memory::Equals(pFramePixels, frame0Bytes, sizeof(frame0Bytes)) );
	pFramePixels = png.GetAnimFrame(1)->GetPixels().GetReadPtr();
	ASSERT_TRUE( Memory::Equals(pFramePixels, frame1Bytes, sizeof(frame1Bytes)) );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(POEngine, OptimizeAnimated_BigTransparentFrame_MultiPalette)
{
	TestImg img;
	img.m_pf = PF_8bppIndexed;
	img.SetWidth(7); // 2 + 3 + 2
	img.SetHeight(7);

	TestFrame frame0(&img);
	frame0.m_fctl.offsetX = 2;
	frame0.m_fctl.offsetY = 2;
	frame0.m_fctl.width = 3;
	frame0.m_fctl.height = 3;
	frame0.m_fctl.delayFracNumerator = 100;
	frame0.m_pixels.SetSize(3 * 3);
	frame0.m_pixels.Fill(1); // Fully opaque
	frame0.m_ownPalette = true;
	frame0.m_palette.m_count = 2;
	frame0.m_palette[0].SetRgba(255, 255, 255, 0);
	frame0.m_palette[1].SetRgba(100, 200, 100, 255);
	img.m_animFrames.Add(frame0);

	img.m_animFrames[0].m_fctl.disposal = AnimFrame::DispClearToTransBlack;

	TestFrame frame1(&img);
	frame1.m_fctl.offsetX = 3;
	frame1.m_fctl.offsetY = 3;
	frame1.m_fctl.width = 1;
	frame1.m_fctl.height = 1;
	frame1.m_fctl.delayFracNumerator = 100;
	frame1.m_pixels.SetSize(1 * 1);
	frame1.m_pixels.Fill(1); // Fully opaque
	frame1.m_ownPalette = true;
	frame1.m_palette.m_count = 2;
	frame1.m_palette[0].SetRgba(255, 255, 255, 0);
	frame1.m_palette[1].SetRgba(200, 100, 100, 255);
	img.m_animFrames.Add(frame1);

	//////////////////////////////////////////////////////////////////
	// Dump
	POEngine engine;
	PngDumpData dd;
	static const String resultFilePath = "result.png";
	Png png;

	File::Delete(resultFilePath);
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );

	// We should get a APNG image with the same palette
	// A default image of 7x7, containing the first 3x3 frame, with transparent color (index 0) around
	ASSERT_TRUE( png.Load(resultFilePath) );

	// One single frame : no animation information at all
	ASSERT_TRUE( png.IsAnimated() );
	ASSERT_TRUE( !png.HasDefaultImage() );
	ASSERT_TRUE( png.GetWidth() == 7 );
	ASSERT_TRUE( png.GetHeight() == 7 );
	ASSERT_TRUE( png.GetFrameCount() == 2 );
	ASSERT_TRUE( png.GetPixelFormat() == PF_2bppIndexed );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// After the merge of all frame palettes, the new palette exceeds 256 colors. true colors conversion needed
void Helper_OptimizeAnimated_MultiPalette_Saturated(int w, int h, PixelFormat expectedPixelFormat)
{
	TestImg img;
	img.m_pf = PF_8bppIndexed;
	img.SetWidth(w);
	img.SetHeight(h);

	TestFrame frame0(&img);
	frame0.m_fctl.offsetX = 0;
	frame0.m_fctl.offsetY = 0;
	frame0.m_fctl.width = 16;
	frame0.m_fctl.height = 16;
	frame0.m_fctl.delayFracNumerator = 100;
	frame0.m_pixels.SetSize(16 * 16);
	frame0.m_ownPalette = true;
	frame0.m_palette.m_count = 256;
	uint8* pRaw = frame0.m_pixels.GetWritePtr();
	for(int i = 0; i < 256; ++i)
	{
		pRaw[i] = i;
		frame0.m_palette[i].SetRgba(i, 255, 255, 255);
	}
	img.m_animFrames.Add(frame0);

	img.m_animFrames[0].m_fctl.disposal = AnimFrame::DispClearToTransBlack;

	TestFrame frame1(&img);
	frame1.m_fctl.offsetX = 0;
	frame1.m_fctl.offsetY = 0;
	frame1.m_fctl.width = 16;
	frame1.m_fctl.height = 16;
	frame1.m_fctl.delayFracNumerator = 100;
	frame1.m_pixels.SetSize(16 * 16);
	frame1.m_ownPalette = true;
	frame1.m_palette.m_count = 256;
	pRaw = frame1.m_pixels.GetWritePtr();
	for(int i = 0; i < 256; ++i)
	{
		pRaw[i] = i;
		frame1.m_palette[i].SetRgba(255, i, 255, 255);
	}
	img.m_animFrames.Add(frame1);

	//////////////////////////////////////////////////////////////////
	// Dump
	POEngine engine;
	PngDumpData dd;
	static const String resultFilePath = "result.png";
	Png png;

	File::Delete(resultFilePath);
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );

	// We should get a APNG image with the same palette
	// A default image of 7x7, containing the first 3x3 frame, with transparent color (index 0) around
	ASSERT_TRUE( png.Load(resultFilePath) );

	// One single frame : no animation information at all
	ASSERT_TRUE( png.IsAnimated() );
	ASSERT_TRUE( !png.HasDefaultImage() );
	ASSERT_TRUE( png.GetWidth() == w );
	ASSERT_TRUE( png.GetHeight() == h );
	ASSERT_TRUE( png.GetFrameCount() == 2 );
	ASSERT_TRUE( png.GetPixelFormat() == expectedPixelFormat );
}

TEST(POEngine, OptimizeAnimated_MultiPalette_Saturated)
{
	Helper_OptimizeAnimated_MultiPalette_Saturated(16, 16, PF_24bppRgb);
}

TEST(POEngine, OptimizeAnimated_BigTransparentFrame_MultiPalette_Saturated)
{
	Helper_OptimizeAnimated_MultiPalette_Saturated(20, 20, PF_32bppRgba);
}

// Create a default image and two frames, all of them use 3 colors out of 4
// We should obtain a PNG with a palette of 3 colors only
TEST(POEngine, OptimizeAnimated_UnusedColors)
{
	TestImg img;
	img.m_pf = PF_8bppIndexed;
	img.SetWidth(7); // 2 + 3 + 2
	img.SetHeight(7);

	// Do not use frame palettes, as local palettes (GIF) is mutually exclusive with a default image (PNG)

	TestFrame frame0(&img);
	frame0.m_fctl.offsetX = 2;
	frame0.m_fctl.offsetY = 2;
	frame0.m_fctl.width = 3;
	frame0.m_fctl.height = 3;
	frame0.m_fctl.delayFracNumerator = 100;
	frame0.m_pixels.SetSize(3 * 3);
	frame0.m_pixels.Fill(1); // Fully opaque
	frame0.m_ownPalette = false;
	img.m_animFrames.Add(frame0);

	img.m_animFrames[0].m_fctl.disposal = AnimFrame::DispClearToTransBlack;

	TestFrame frame1(&img);
	frame1.m_fctl.offsetX = 3;
	frame1.m_fctl.offsetY = 3;
	frame1.m_fctl.width = 1;
	frame1.m_fctl.height = 1;
	frame1.m_fctl.delayFracNumerator = 100;
	frame1.m_pixels.SetSize(1 * 1);
	frame1.m_pixels.Fill(3); // Fully opaque
	frame1.m_ownPalette = false;
	img.m_animFrames.Add(frame1);

	//////////////////////////////////////////////////////////////////
	// Create a default image and fill the common palette
	static const uint8 defaultBytes[] = {
		1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 1,
		1, 0, 3, 3, 3, 0, 1,
		1, 0, 3, 3, 3, 0, 1,
		1, 0, 3, 3, 3, 0, 1,
		1, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1,
	};
	img.m_hasDefaultImage = true;
	img.SetPixels(defaultBytes, sizeof(defaultBytes));
	img.m_palette.m_count = 4;
	img.m_palette[0].SetRgba(255, 255, 255, 0);
	img.m_palette[1].SetRgba(200, 100, 50, 255);
	img.m_palette[2].SetRgba(88, 88, 88, 255); // Unused color
	img.m_palette[3].SetRgba(200, 100, 100, 255);

	//////////////////////////////////////////////////////////////////
	// Dump
	POEngine engine;
	PngDumpData dd;
	static const String resultFilePath = "result.png";
	Png png;

	File::Delete(resultFilePath);
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );
	ASSERT_TRUE( png.Load(resultFilePath) );
	ASSERT_TRUE( png.IsAnimated() );
	ASSERT_TRUE( png.HasDefaultImage() );
	ASSERT_TRUE( png.GetWidth() == 7 );
	ASSERT_TRUE( png.GetHeight() == 7 );
	ASSERT_TRUE( png.GetFrameCount() == 2 );
	ASSERT_TRUE( png.GetPixelFormat() == PF_2bppIndexed );
	ASSERT_EQ( png.GetPalette().m_count, 3 );
}

// Create a default image and two frames, all of them use 3 colors out of 4
// We should obtain a PNG with a palette of 3 colors only
TEST(POEngine, OptimizeAnimated_Loops)
{
	TestImg img;
	img.m_pf = PF_8bppIndexed;
	img.SetWidth(4);
	img.SetHeight(4);

	TestFrame frame0(&img);
	frame0.m_fctl.width = 4;
	frame0.m_fctl.height = 4;
	frame0.m_fctl.delayFracNumerator = 200;
	frame0.m_pixels.SetSize(16);
	frame0.m_pixels.Fill(0);
	img.m_animFrames.Add(frame0);

	img.m_animFrames[0].m_fctl.disposal = AnimFrame::DispClearToTransBlack;

	TestFrame frame1(&img);
	frame1.m_fctl.width = 4;
	frame1.m_fctl.height = 4;
	frame1.m_fctl.delayFracNumerator = 200;
	frame1.m_pixels.SetSize(16);
	frame1.m_pixels.Fill(1);
	img.m_animFrames.Add(frame1);

	img.m_palette.m_count = 2;
	img.m_palette[0].SetRgb(255, 0, 0);
	img.m_palette[1].SetRgb(0, 255, 0);

	//////////////////////////////////////////////////////////////////
	// Dump
	POEngine engine;
	PngDumpData dd;
	static const String resultFilePath = "result.png";
	Png png;

	File::Delete(resultFilePath);
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );
	ASSERT_TRUE( png.Load(resultFilePath) );
	ASSERT_TRUE( png.IsAnimated() );
	ASSERT_TRUE( png.GetLoopCount() == 0 ); // 0 = infinite

	img.m_loopCount = 1;

	File::Delete(resultFilePath);
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );
	ASSERT_TRUE( png.Load(resultFilePath) );
	ASSERT_TRUE( png.IsAnimated() );
	ASSERT_TRUE( png.GetLoopCount() == 1 );

	img.m_loopCount = 347;

	File::Delete(resultFilePath);
	ASSERT_TRUE( engine.OptimizeAnimated(img, dd, resultFilePath, optiInfo) );
	ASSERT_TRUE( png.Load(resultFilePath) );
	ASSERT_TRUE( png.IsAnimated() );
	ASSERT_TRUE( png.GetLoopCount() == 347 );
}

TEST(POEngine, pHYs_Remove)
{
	static const String resultFilePath = "test.png";
	File::Delete(resultFilePath);

	// Build test image
	PngDumpData dd;
	dd.pixelFormat = PF_8bppIndexed;
	dd.width = 7; // 2 + 3 + 2
	dd.height = 7;
	dd.pixels.SetSize(dd.width * dd.height);
	dd.palette.m_count = 2;
	dd.palette[0].SetRgba(255, 255, 255, 0);
	dd.palette[1].SetRgba(100, 200, 100, 255);
	dd.usePhys = true;
	dd.pHYs.pixelsPerUnitX = 1000;
	dd.pHYs.pixelsPerUnitY = 500;
	dd.pHYs.unit = 1;
	ASSERT_TRUE( PngDumper::Dump(resultFilePath, dd, PngDumpSettings()) );

	// Remove is default options
	POEngine engine;
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeFileDiskNoBackup(resultFilePath, resultFilePath, optiInfo) );

	// Reload
	Png png;
	ASSERT_TRUE( png.Load(resultFilePath) );
	ASSERT_TRUE( !png.HasPhysicalPixelDimensions() );
	ASSERT_EQ( 0u, png.GetPhysPpuX() );
	ASSERT_EQ( 0u, png.GetPhysPpuX() );
	ASSERT_EQ( 0, png.GetPhysUnit() );
}

TEST(POEngine, pHYs_Keep)
{
	static const String resultFilePath = "test.png";
	File::Delete(resultFilePath);

	// Build test image
	PngDumpData dd;
	dd.pixelFormat = PF_8bppIndexed;
	dd.width = 7; // 2 + 3 + 2
	dd.height = 7;
	dd.pixels.SetSize(dd.width * dd.height);
	dd.palette.m_count = 2;
	dd.palette[0].SetRgba(255, 255, 255, 0);
	dd.palette[1].SetRgba(100, 200, 100, 255);
	dd.usePhys = true;
	dd.pHYs.pixelsPerUnitX = 1002;
	dd.pHYs.pixelsPerUnitY = 502;
	dd.pHYs.unit = 17; // Strange unit, but should be kept
	ASSERT_TRUE( PngDumper::Dump(resultFilePath, dd, PngDumpSettings()) );

	// Remove is default options
	POEngine engine;
	engine.m_settings.physOption = POChunk_Keep;
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeFileDiskNoBackup(resultFilePath, resultFilePath, optiInfo) );

	// Reload
	Png png;
	ASSERT_TRUE( png.Load(resultFilePath) );
	ASSERT_TRUE( png.HasPhysicalPixelDimensions() );
	ASSERT_EQ( 1002u, png.GetPhysPpuX() );
	ASSERT_EQ( 502u, png.GetPhysPpuY() );
	ASSERT_EQ( 17, png.GetPhysUnit() );
}

TEST(POEngine, pHYs_Force)
{
	static const String resultFilePath = "test.png";
	File::Delete(resultFilePath);

	// Build test image
	PngDumpData dd;
	dd.pixelFormat = PF_8bppIndexed;
	dd.width = 7; // 2 + 3 + 2
	dd.height = 7;
	dd.pixels.SetSize(dd.width * dd.height);
	dd.palette.m_count = 2;
	dd.palette[0].SetRgba(255, 255, 255, 0);
	dd.palette[1].SetRgba(100, 200, 100, 255);
	ASSERT_TRUE( PngDumper::Dump(resultFilePath, dd, PngDumpSettings()) );

	// Force phys
	POEngine engine;
	engine.m_settings.physOption = POChunk_Force;
	engine.m_settings.physPpmX = 7000;
	engine.m_settings.physPpmY = 6000;
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeFileDiskNoBackup(resultFilePath, resultFilePath, optiInfo) );

	// Reload
	Png png;
	ASSERT_TRUE( png.Load(resultFilePath) );
	ASSERT_TRUE( png.HasPhysicalPixelDimensions() );
	ASSERT_EQ( 7000u, png.GetPhysPpuX() );
	ASSERT_EQ( 6000u, png.GetPhysPpuY() );
	ASSERT_EQ( 1, png.GetPhysUnit() ); // Always meter in POEngine
}

TEST(POEngine, IsFileExtensionSupported)
{
	ASSERT_FALSE( POEngine::IsFileExtensionSupported("zip") );
	ASSERT_FALSE( POEngine::IsFileExtensionSupported("zip", "*.zip") );
	ASSERT_TRUE( POEngine::IsFileExtensionSupported("png") );
	ASSERT_TRUE( POEngine::IsFileExtensionSupported("png", "*.png") );
	ASSERT_TRUE( POEngine::IsFileExtensionSupported("png", "*.png|*.bmp") );
	ASSERT_FALSE( POEngine::IsFileExtensionSupported("png", "*.bmp") );

	ASSERT_TRUE( POEngine::IsFileExtensionSupported("PNG") );
	ASSERT_TRUE( POEngine::IsFileExtensionSupported("Png") );
	ASSERT_TRUE( POEngine::IsFileExtensionSupported("PNG", "*.png") );
	ASSERT_TRUE( POEngine::IsFileExtensionSupported("png", "*.PNG") );

	ASSERT_TRUE( POEngine::IsFileExtensionSupported("apng") );
	ASSERT_TRUE( POEngine::IsFileExtensionSupported("APNG") );
}

bool BuildTestImage(IFile& dstFile)
{
	// Build test image
	PngDumpData dd;
	dd.pixelFormat = PF_8bppIndexed;
	dd.width = 7; // 2 + 3 + 2
	dd.height = 7;
	dd.pixels.SetSize(dd.width * dd.height);
	dd.palette.m_count = 2;
	dd.palette[0].SetRgba(255, 255, 255, 0);
	dd.palette[1].SetRgba(100, 200, 100, 255);
	uint8* pDst = dd.pixels.GetWritePtr();
	for(int i = 0; i < dd.height; ++i)
	{
		for(int j = 0; j < dd.width; ++j)
		{
			pDst[i * dd.width + j] = (i ^ j) & 0x01;
		}
	}
	return PngDumper::Dump(dstFile, dd, PngDumpSettings());
}

TEST(POEngine, OptimizeFileMem)
{
	DynamicMemoryFile dstFile;
	dstFile.Open(65536);
	ASSERT_TRUE( BuildTestImage(dstFile) );
	const Buffer& inBuf = dstFile.GetContent();

	Buffer optiBuf;
	int optiCapacity = 65536;
	optiBuf.EnsureCapacity(optiCapacity);
	uint8* pOpti = optiBuf.GetWritePtr();
	int optiSize = 0;

	POEngine engine;
	ASSERT_TRUE( engine.GetLastErrorString().IsEmpty() );

	// Test invalid arg
	engine.ClearLastError();
	ASSERT_FALSE( engine.OptimizeFileMem(inBuf.GetReadPtr(), 0, pOpti, optiCapacity, &optiSize) );
	ASSERT_FALSE( engine.GetLastErrorString().IsEmpty() );

	// Test destination capacity too small
	optiSize = 5;
	engine.ClearLastError();
	ASSERT_FALSE( engine.OptimizeFileMem(inBuf.GetReadPtr(), inBuf.GetSize(), pOpti, 1, &optiSize) );
	ASSERT_FALSE( engine.GetLastErrorString().IsEmpty() );
	ASSERT_EQ( 0, optiSize );

	// Test legit optimization
	ASSERT_TRUE( engine.OptimizeFileMem(inBuf.GetReadPtr(), inBuf.GetSize(), pOpti, optiCapacity, &optiSize) );
	ASSERT_TRUE( optiSize > (8+12) );
	// Check that we ends with an IEND chunk to ensure dstSize is correctly set
	ASSERT_EQ( 'I', pOpti[optiSize - 8] );
	ASSERT_EQ( 'E', pOpti[optiSize - 7] );
	ASSERT_EQ( 'N', pOpti[optiSize - 6] );
	ASSERT_EQ( 'D', pOpti[optiSize - 5] );
}

bool BuildTestImage_ColorToGrey(IFile& dstFile)
{
	// Build test image
	PngDumpData dd;
	dd.pixelFormat = PF_32bppRgba;
	dd.width = 7;
	dd.height = 7;
	dd.pixels.SetSize(dd.width * dd.height * 4);
	uint8* pDst = dd.pixels.GetWritePtr();
	for(int i = 0; i < dd.height; ++i)
	{
		for(int j = 0; j < dd.width; ++j)
		{
			int off = i * (dd.width * 4) + j * 4;
			uint8& r = pDst[off + 0];
			uint8& g = pDst[off + 1];
			uint8& b = pDst[off + 2];
			uint8& a = pDst[off + 3];

			r = g = b = uint8(i ^ j);
			a = 255;
		}
	}
	return PngDumper::Dump(dstFile, dd, PngDumpSettings());
}

// Convert a RGBA imge, with RGB always grey and A always 0 or 255
TEST(POEngine, OptimizeFileMem_ColorToGrey_NoAlpha)
{
	DynamicMemoryFile dstFile;
	dstFile.Open(65536);
	ASSERT_TRUE( BuildTestImage_ColorToGrey(dstFile) );
	const Buffer& inBuf = dstFile.GetContent();

	Buffer optiBuf;
	int optiCapacity = 65536;
	optiBuf.EnsureCapacity(optiCapacity);
	uint8* pOpti = optiBuf.GetWritePtr();
	int optiSize = 0;

	POEngine engine;
	ASSERT_TRUE( engine.OptimizeFileMem(inBuf.GetReadPtr(), inBuf.GetSize(), pOpti, optiCapacity, &optiSize) );

	StaticMemoryFile pngFile;
	pngFile.OpenRead(pOpti, optiSize);

	Png png;
	ASSERT_TRUE( png.LoadFromFile(pngFile) );
	ASSERT_TRUE( png.GetPixelFormat() == PF_8bppGrayScale );
}

// Test that a PNG that cannot be improved is not overwritten
TEST(POEngine, NoNeedlessOverwrite)
{
	static const String filePath = "onepix.png";
	File::Delete(filePath);

	// Build test image. This image cannot be compressed more.
	// However the IDAT content can change. We want PngOptimizer to keep
	// the original version.
	PngDumpData dd;
	dd.pixelFormat = PF_24bppRgb;
	dd.width = 1; // 2 + 3 + 2
	dd.height = 1;
	dd.pixels.SetSize(dd.width * dd.height * 3);
	dd.pixels.GetWritePtr()[0] = 0xff;
	dd.pixels.GetWritePtr()[1] = 0x7f;
	dd.pixels.GetWritePtr()[2] = 0x3f;

	PngDumpSettings ds;
	ASSERT_TRUE( PngDumper::Dump(filePath, dd, ds) );

	// To detect an attempt of overwrite
	ASSERT_TRUE( File::SetReadOnly(filePath) );

	POEngine engine;
	POEngine::OptiInfo optiInfo;
	ASSERT_TRUE( engine.OptimizeFileDiskNoBackup(filePath, filePath, optiInfo) );
	ASSERT_TRUE( optiInfo.sameContent );

	ASSERT_TRUE( File::SetReadOnly(filePath, false) );
	ASSERT_TRUE( File::Delete(filePath) );
}

// Test that when converting from palette to gray we consider
// same grays with different alphas as different colors
TEST(POEngine, PaletteSameGrayDifferentAlpha)
{
	const uint8 data[] = {
		1, 1, 1,
		1, 0, 1,
		1, 1, 1
	};
	PngDumpData dd;
	dd.pixelFormat = PF_8bppIndexed;
	dd.width = 3;
	dd.height = 3;
	dd.pixels.Assign(data, 9);
	dd.palette.m_count = 2;
	dd.palette[0] = Color(8, 8, 8, 0);
	dd.palette[1] = Color(8, 8, 8, 255);

	POEngine engine;
	ASSERT_TRUE(engine.OptimizeExternalBuffer(dd, "result.png"));

	Png png;
	ASSERT_TRUE(png.Load("result.png"));
	ASSERT_EQ(3, png.GetWidth());
	ASSERT_EQ(3, png.GetHeight());
	ASSERT_TRUE(ImageFormat::IsGray(png.GetPixelFormat()));
	ASSERT_TRUE(png.HasSimpleTransparency());
	ASSERT_EQ(0, png.GetGreyTransIndex());

	auto pixels = png.GetPixels();
	ImageFormat::UnpackPixels(pixels, 3, 3, png.GetPixelFormat());
	const uint8 expected[] = {
		8, 8, 8,
		8, 0, 8,
		8, 8, 8
	};
	ASSERT_TRUE(memcmp(pixels.GetReadPtr(), expected, 9) == 0);
}
