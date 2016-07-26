///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_BMP_H
#define CHUSTD_BMP_H

#include "ImageFormat.h"

namespace chustd {

class BmpDumpSettings
{
public:
	Buffer  pixels;
	Palette palette;
	int32   width;
	int32   height;
	PixelFormat pixelFormat;

	BmpDumpSettings()
	{
		width = 0;
		height = 0;
		pixelFormat = PF_Unknown;
	}
};

class Bmp : public ImageFormat  
{
public:
	/////////////////////////////////////////////////////////////
	virtual bool LoadFromFile(IFile& file);
	virtual PixelFormat GetPixelFormat() const;
	virtual const Palette& GetPalette() const;

	virtual String GetLastErrorString() const;
	/////////////////////////////////////////////////////////////

	Bmp();
	virtual ~Bmp();

	// Possible errors for m_lastError
	enum
	{
		errNotABmpFile = firstErrEnumDerived,
		errBadHeaderSize,
		errUnconsistentDepthAndCompression,
		errUnconsistentDepthAndColorCount,
		errUnsupportedCompressionFormat,
		errUnsupportedBitfieldsFormat,
		errDepthNotSupported,
		errUnexpectedEndOfFile,
		errBadCompressedData,
		errTooManyColors
	};

	static bool IsBmp(IFile& file);

	static bool Dump(const String& filePath, const BmpDumpSettings& ds);
	static bool Dump(IFile& file, const BmpDumpSettings& ds);

protected:
	enum Compression
	{ 
		compNone, 
		compRle8bits, 
		compRle4bits, 
		compBitfields 
	};

	struct BmpHeader
	{
		uint32 fileSize; // Complete file size in bytes
		uint32 reserved;
		uint32 offsetToData; // Offset from beginning of the file to the beginning of the image data
		uint32 bitmapHeaderSize; // 40 bytes
		int32  width;
		int32  height;
		uint16 planes;
		uint16 depth; // Bits per pixel. Possible values : 1, 4, 8, 16, 24, 32
		uint32 compression; // Possible values : 0=None (BI_RGB), 1=RLE 8bits, 2=RLE 4bits, 3=BI_BITFIELDS
		uint32 bitmapDataSize; // Rounded to the next 4 byte boundary
		int32  xPelsPerMeter;
		int32  yPelsPerMeter;
		int32  colorCount; // Number of colors
		uint32 importantColors;

		void SwapBytes();
	};

	virtual void Initialize();

private:
	PixelFormat m_epf;
	Palette m_ColorTable;

private:
	int32 ComputeBmpBytesPerRow(int32 expectedBytesPerRow);
	PixelFormat GetPfFromDepth(uint16 depth) const;
	bool RleUncompress(const ByteArray& rbSrc, ByteArray& rbDst, int32 niceBytePerRow, int32 depth);

	bool ReadPixelData(IFile& file, const BmpHeader& bh);

	bool ReadPixelsFromFile(IFile& file);
	void RemovePadding();
};

} // namespace chustd

#endif // ndef CHUSTD_BMP_H
