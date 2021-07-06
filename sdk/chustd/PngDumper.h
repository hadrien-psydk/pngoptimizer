///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_PNGDUMPER_H
#define CHUSTD_PNGDUMPER_H

#include "Png.h"
#include "DeflateCompressor.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
// Data to dump as a PNG file
struct PngDumpData
{
	Buffer         pixels;
	Palette        palette;
	int32          width;
	int32          height;
	PixelFormat    pixelFormat;

	bool          interlaced;    // true to save an interlaced png

	bool          useTransparentColor; // Set to true to use the following members
	PngChunk_tRNS tRNS;

	bool          useBackgroundColor;
	PngChunk_bkGD bkGD;
	
	bool          usePhys;
	PngChunk_pHYs pHYs;

	Array<PngChunk_tEXt> textInfos;

	////////////////////////
	// Animation

	// true  = Use pixels in m_pBuffer to create the default image.
	// false = m_pBuffer can be null, the IDAT will be part of the animation, 
	//         but there must be one frame at least in m_apFrames.
	bool   hasDefaultImage;
	uint32 loopCount;
	PtrArray<ApngFrame> frames;

	PngDumpData()
	{
		width = 0;
		height = 0;
		pixelFormat = PF_Unknown;
		interlaced = false;
		useTransparentColor = false;
		useBackgroundColor = false;
		usePhys = false;
		hasDefaultImage = false;
		loopCount = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Settings of the PngDumper
struct PngDumpSettings
{
public:

	enum ZLibOption
	{ 
		zlibStrategyGuess = 0x00,   // Default
		zlibStrategyDefault = 0x01,
		zlibStrategyFilter = 0x02,
	
		zlibWindowBitsAndMemHigh = 0x00, // Default
		zlibWindowBitsAndMemLow = 0x10   // Can sometimes improve compression
	};

	uint8       zlibCompressionLevel; // [1..9], default = 6
	uint8       zlibStrategy;
	uint8       zlibWindowBitsAndMem;
	
	// 0 = Adaptative filtering with "None" set for each scanline
	// 1 = Adaptative filtering with various sub-method set for each scanline
	uint8       filtering;    // Default = 1

	PngDumpSettings()
	{
		zlibCompressionLevel = 6;
		zlibStrategy = zlibStrategyGuess;
		zlibWindowBitsAndMem = zlibWindowBitsAndMemHigh;
		
		filtering = 1;
	}
};

//////////////////////////////////////////////////////////////////////////////////////
// Dumps a pixel buffer as a PNG file
class PngDumper
{
public:
	/////////////////////////////////////////////////////////////////////////////////////
	static bool Dump(const String& filePath, const PngDumpData& dd, const PngDumpSettings& ds);
	static bool Dump(IFile& file, const PngDumpData& dd, const PngDumpSettings& ds);

	static bool WriteSignature(IFile& file);
	static bool WriteChunk_bkGD(ChunkedFile& cf, uint8 colorType, const PngChunk_bkGD& content);
	static bool WriteChunk_pHYs(ChunkedFile& cf, const PngChunk_pHYs& content);
	static bool WriteChunk_tEXt(ChunkedFile& cf, const PngChunk_tEXt& content);

private:
	static bool GetIHDRFeaturesFromPixelFormat(PixelFormat epf, uint8& colorType, uint8& bitDepthPerComponent);
	static bool WriteFrameControlChunk(const ApngFrame* pFrame, int32 apngSequenceNumber, ChunkedFile& file);
	static bool CreateImageData(const uint8* pSrc, int32 width, int32 height,
		const PngDumpData& dd, const PngDumpSettings& ds, ByteArray& abImageData);
	static bool InterlaceAndFilter(uint8* pDst, const uint8* pSrc,
		const int32 srcWidth, const int32 srcHeight, const int32 srcPixelBytesPerRow,
		int32 sizeofPixelInBits, bool bDoFiltering);
	static bool FilterBlock(uint8* const pBlock, int32 rowCount, int32 pixelBytesPerRow, int32 bytesPerPixel);
	static int Compress(uint8* pDest, uint32* pDestLen, const uint8* pSource, uint32 sourceLen, 
		int level, DeflateStrategy strategy, int windowBits, int memLevel);
};

} // namespace chustd;

#endif // ndef CHUSTD_PNGDUMPER_H
