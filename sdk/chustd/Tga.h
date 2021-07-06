///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_TGA_H
#define CHUSTD_TGA_H

#include "ImageFormat.h"

namespace chustd {

class Tga : public ImageFormat
{
public:
	enum
	{
		errBadColorMapType = firstErrEnumDerived,
		errUnsupportedFormat,
		errUnsupportedColorMapDepth,
		errColorMapTooLarge,
		errBadCompressedData,
	};

	///////////////////////////////////////////////////////
	virtual bool LoadFromFile(IFile& file);
	virtual PixelFormat GetPixelFormat() const;
	virtual const Palette& GetPalette() const;

	virtual String GetLastErrorString() const;
	///////////////////////////////////////////////////////

	static bool IsTga(IFile& file);

	Tga();
	virtual ~Tga();

protected:
	enum TgaImageType
	{
		titNoImage = 0,           // No image data included
		titUncompColorMapped = 1, // Uncompressed, color-mapped images
		titUncompRgb = 2,         // Uncompressed, RGB images
		titUncompBaw = 3,         // Uncompressed, black and white images
		titRunlengthColorMapped = 9, // Runlength encoded color-mapped images
		titRunlengthRgb = 10,        // Runlength encoded RGB images
		titCompBaw = 11,          // Compressed, black and white images
		titCompColorMapped = 32,  // Compressed color-mapped data, using Huffman, Delta, and runlength encoding
		titCompColorMapped4Pass = 33 // Compressed color-mapped data, using Huffman, Delta, and
		                             // runlength encoding.  4-pass quadtree-type process
	};

	enum TgaImageOrigin
	{
		tioBottomLeft,
		tioBottomRight,
		tioTopLeft,
		tioTopRight
	};

	enum AttributesType
	{
		atUndef = -1,  // Should be out of [0-255]

		atNoAlphaDataIncluded = 0,
		atUndefDataIgnore,
		atUndefDataRetain,
		atUsefulAlpha,
		atPremultAlpha
	};

	AttributesType m_eAttributesType;
	PixelFormat m_epf;
	Palette m_pal;
	
	struct TgaFooter
	{
		uint32 extensionAreaOffset;
		uint32 developerDirectoryOffset;
		uint8 aSignature[16];
		uint8 dot;
		uint8 zero;
	};

protected:
	bool DecodeRawPixels(IFile& file, int32 bytesPerPixel);
	bool DecodeRlePixels(IFile& file, int32 bytesPerPixel);
	bool Decode1bppGrayScale(IFile& file);

	void Initialize();

	bool ReadPalette(IFile& file, int16 colorMapOrigin, int16 colorMapLength, uint8 colorMapDepth);
	void ReadNewTgaFormatUsefulInfo(IFile& file);
	static bool ReadFooter(IFile& file, TgaFooter& tf);
};

} // namespace chustd

#endif // ndef CHUSTD_TGA_H
