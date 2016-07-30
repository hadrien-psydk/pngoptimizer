///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tga.h"
#include "File.h"
#include "Math.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;
//////////////////////////////////////////////////////////////////////

Tga::Tga()
{
	Initialize();
}

Tga::~Tga()
{

}

void Tga::Initialize()
{
	m_width = 0;
	m_height = 0;
	m_lastError = 0;

	m_eAttributesType = atUndef;
	m_epf = PF_Unknown;
	m_pal.m_count = 0;
}

bool Tga::IsTga(IFile& file)
{
	// Unfortunately the TGA format does not include a signature in the header
	
	// At least read some constant values in the header
	uint8 aHeader[18];
	if( file.Read(aHeader, 18) != 18 )
	{
		return false;
	}

	TgaFooter tf;
	if( ReadFooter(file, tf) )
	{
		// YES ! For sure it is a TGA file
		return true;
	}

	// Try what we can with the header :-/
	if( (aHeader[1] & 0xFE) != 0 )
	{
		// ColorMapType can be 0 or 1 only
		return false;
	}

	if( (aHeader[2] & 0xC0) != 0 )
	{
		// Image type from 0 to 33 (assume max = 63)
		return false;
	}

	uint8 colorMapDepth = aHeader[7];
	if( !(colorMapDepth == 0 || colorMapDepth == 15 || colorMapDepth == 16
		|| colorMapDepth == 24 || colorMapDepth == 32))
	{
		// Invalid palette color format
		return false;	
	}
	
	if( !(0 < aHeader[16] && aHeader[16] <= 64) )
	{
		// Depth out of range
		return false;
	}

	// One can say for sure that TGA is not the state of the art in graphics format
	return true;
}

bool Tga::LoadFromFile(IFile& file)
{
	FreeBuffer();
	Initialize();
	
	// The TGA format is little endian
	TmpEndianMode tem(file, boLittleEndian);

	// I would have done a struct if the header was not such a mess of misaligned attributes
	uint8 aHeader[18];
	int32 read = file.Read(aHeader, 18);
	if( read != 18 )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	uint8 imageIdLength = aHeader[0];
	uint8 colorMapType  = aHeader[1];
	uint8 imageType     = aHeader[2];
	int16 colorMapOrigin = MAKE16( aHeader[4], aHeader[3] );
	int16 colorMapLength = MAKE16( aHeader[6], aHeader[5] );
	uint8 colorMapDepth  = aHeader[7];
	int16 xOrigin = MAKE16( aHeader[9], aHeader[8] );
	int16 yOrigin = MAKE16( aHeader[11], aHeader[10] );
	int16 width   = MAKE16( aHeader[13], aHeader[12] );
	int16 height  = MAKE16( aHeader[15], aHeader[14] );
	uint8 bitsPerPixel = aHeader[16];
	uint8 imageDescriptor = aHeader[17];

	(void)xOrigin;
	(void)yOrigin;

	if( colorMapType == 0 )
	{
		// No color-map data is included with this image
	}
	else if( colorMapType == 1 )
	{
		// A color-map is included
	}
	else
	{
		// Error
		m_lastError = errBadColorMapType;
		return false;
	}

	
	// Here comes the ImageId
	if( imageIdLength > 0 )
	{
		uint8 aImageId[255];
		read = file.Read(aImageId, imageIdLength);
		if( read != imageIdLength )
		{
			m_lastError = uncompleteFile;
			return false;
		}
	
	}

	// Here comes the colormap (also called palette)
	if( colorMapType == 1 && colorMapLength > 0 )
	{
		if( !ReadPalette(file, colorMapOrigin, colorMapLength, colorMapDepth) )
		{
			return false;
		}
	}

	m_width = width;
	m_height = height;

	TgaImageOrigin eImageOrigin = TgaImageOrigin (imageDescriptor >> 4);
	uint8 alphaBitCount = imageDescriptor & 0x0f;

	// Retrieve additional information that can be used to know what to do with the attribute fields
	ReadNewTgaFormatUsefulInfo(file);

	bool bDecodeOk = false;

	TgaImageType eImageType = TgaImageType(imageType);
	if( eImageType == titNoImage )
	{
		// So... ?	
		m_lastError = errUnsupportedFormat;
		return false;
	}
	else if( eImageType == titUncompColorMapped )
	{
		// Uncompressed, color-mapped images
		if( bitsPerPixel == 8 && colorMapType == 1 )
		{
			m_epf = PF_8bppIndexed;
			bDecodeOk = DecodeRawPixels(file, 1);
		}
		else
		{
			m_lastError = errUnsupportedFormat;
			return false;
		}
	}
	else if( eImageType == titUncompRgb )
	{
		// Uncompressed, RGB images
		if( bitsPerPixel == 24 && alphaBitCount == 0 )
		{
			m_epf = PF_24bppBgr;
			bDecodeOk = DecodeRawPixels(file, 3);
		}
		else if( bitsPerPixel == 32 )
		{
			m_epf = PF_32bppBgra;
			bDecodeOk = DecodeRawPixels(file, 4);
		}
		else if( bitsPerPixel == 16 )
		{
			// Note : the most significant bit stores an attribute, not an alpha channel, so we ignore it

			m_epf = PF_16bppRgb555;
			bDecodeOk = DecodeRawPixels(file, 2);
			if( bDecodeOk )
			{
				// Arrange values from GGGBBBBB_ARRRRRGG to ARRRRRGG_GGGBBBBB in memory
				const int32 pixelCount = m_width * m_height;
				IFile::Swap16( (uint16*) m_pixels.GetWritePtr(), pixelCount);
			}
		}
		else
		{
			m_lastError = errUnsupportedFormat;
			return false;
		}
	}
	else if( eImageType == titUncompBaw )
	{
		// Uncompressed, black and white images
		if( bitsPerPixel == 8 )
		{
			m_epf = PF_8bppGrayScale;
			bDecodeOk = DecodeRawPixels(file, 1);
		}
		else if( bitsPerPixel == 1 )
		{
			m_epf = PF_1bppGrayScale;
			bDecodeOk = Decode1bppGrayScale(file);
		}
		else
		{
			m_lastError = errUnsupportedFormat;
			return false;
		}
	}
	else if( eImageType == titRunlengthColorMapped )
	{
		// Runlength encoded color-mapped images
		if( bitsPerPixel == 8 && colorMapType == 1 )
		{
			m_epf = PF_8bppIndexed;
			bDecodeOk = DecodeRlePixels(file, 1);
		}
		else
		{
			m_lastError = errUnsupportedFormat;
			return false;
		}
	}
	else if( eImageType == titRunlengthRgb )
	{
		// Runlength encoded RGB images
		if( bitsPerPixel == 24 && alphaBitCount == 0 )
		{
			m_epf = PF_24bppBgr;
			bDecodeOk = DecodeRlePixels(file, 3);
		}
		else if( bitsPerPixel == 32 )
		{
			m_epf = PF_32bppBgra;
			bDecodeOk = DecodeRlePixels(file, 4);
		}
		else if( bitsPerPixel == 16 )
		{
			// Note : the most significant bit stores an attribute, not an alpha channel, so we ignore it

			m_epf = PF_16bppRgb555;
			bDecodeOk = DecodeRlePixels(file, 2);
			if( bDecodeOk )
			{
				// Arrange values from GGGBBBBB_ARRRRRGG to ARRRRRGG_GGGBBBBB in memory
				const int32 pixelCount = m_width * m_height;
				IFile::Swap16( (uint16*) m_pixels.GetWritePtr(), pixelCount);
			}
		}
		else
		{
			m_lastError = errUnsupportedFormat;
			return false;
		}
	}
	else if( eImageType == titCompBaw )
	{
		// Compressed, black and white images
		if( bitsPerPixel == 8 )
		{
			m_epf = PF_8bppGrayScale;
			bDecodeOk = DecodeRlePixels(file, 1);
		}
		else
		{
			m_lastError = errUnsupportedFormat;
			return false;
		}
	}
	else if( eImageType == titCompColorMapped )
	{
		// Compressed color-mapped data, using Huffman, Delta, and runlength encoding
		m_lastError = errUnsupportedFormat;
		return false;
	}
	else if( eImageType == titCompColorMapped4Pass )
	{
		// Compressed color-mapped data, using Huffman, Delta, and
		// runlength encoding.  4-pass quadtree-type process
		m_lastError = errUnsupportedFormat;
		return false;
	}

	if( !bDecodeOk )
	{
		FreeBuffer();
		return false;
	}

	
	/////////////////////////////////////////////////////////////////////////////
	// Perform additional treatment for 32 bits images

	if( bitsPerPixel == 32 )
	{
		bool bDiscardAlpha = false;

		if( alphaBitCount == 8 )
		{
			// Ok, there are 8 bits to store an "attribute" with each color, but is it really
			// an alpha channel ?
			if( m_eAttributesType == atUndef )
			{
				// No additional information contained in the file, assume the attributes are alphas
			}
			else if( m_eAttributesType == atUsefulAlpha )
			{
				// Additional information available, this is a real alpha channel
			}
			else
			{
				// Discard alpha channel
				bDiscardAlpha = true;
			}
		}
		else
		{
			bDiscardAlpha = true;
		}

		if( bDiscardAlpha )
		{
			SetAlphaFullOpaque();
		}
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Change orientation

	if( eImageOrigin == tioBottomLeft )
	{
		FlipVertical();
	}
	else if( eImageOrigin == tioBottomRight )
	{
	
	}
	else if( eImageOrigin == tioTopLeft )
	{
		// Natural orientation, nothing to do
	}
	else if( eImageOrigin == tioTopRight )
	{
	
	}
	return true;
}

PixelFormat Tga::GetPixelFormat() const
{
	return m_epf;
}

const Palette& Tga::GetPalette() const
{
	return m_pal;
}

String Tga::GetLastErrorString() const
{
	switch(m_lastError)
	{
	case errBadColorMapType:
		return "Bad color map type (0 or 1 expected)";
	case errUnsupportedFormat:
		return "Unsupported format";
	case errUnsupportedColorMapDepth:
		return "Unsupported color map depth (only 16, 24 and 32 are supported)";
	case errColorMapTooLarge:
		return "Color map too large (more than 256 colors)";
	case errBadCompressedData:
		return "Bad compressed data";
	}
	return ImageFormat::GetLastErrorString();
}

bool Tga::DecodeRawPixels(IFile& file, int32 bytesPerPixel)
{
	const int32 bufSize = m_width * m_height * bytesPerPixel;
	if( !m_pixels.SetSize(bufSize) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	uint8* pPixels = m_pixels.GetWritePtr();

	int32 read = file.Read(pPixels, bufSize);
	if( read != bufSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	return true;
}

bool Tga::DecodeRlePixels(IFile& file, int32 bytesPerPixel)
{
	const int32 pixelCount = m_width * m_height;
	const int32 outBufSize = pixelCount * bytesPerPixel;
	if( !m_pixels.SetSize(outBufSize) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	uint8* pPixels = m_pixels.GetWritePtr();

	int32 dstOffset = 0;
	///////////////////////////////////////
	while( dstOffset < outBufSize )
	{
		uint8 aInBuf[16];

		const int32 minRead = 1 + bytesPerPixel;
		int32 read = file.Read(aInBuf, minRead);
		if( read != minRead )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		
		uint8 rleHeader = aInBuf[0];

		uint8 runPixelCount = (rleHeader & 0x7f) + 1;
		int32 runByteCount = runPixelCount * bytesPerPixel;
		if( dstOffset + runByteCount > outBufSize )
		{
			// Troubles here, too much data
			m_lastError = errBadCompressedData;
			return false;
		}
		
		if( (rleHeader & 0x80) == 0x80 )
		{
			///////////////////////////////
			//     Run-length packet     //
			///////////////////////////////

			// Next is the color

			for(int i = 0; i < runPixelCount; ++i)
			{
				for(int iByte = 0; iByte < bytesPerPixel; ++iByte)
				{
					pPixels[dstOffset + iByte] = aInBuf[1 + iByte];
				}
				dstOffset += bytesPerPixel;
			}
		}
		else
		{
			///////////////////////////////
			//          Raw packet       //
			///////////////////////////////
		
			// Put first pixel
			for(int iByte = 0; iByte < bytesPerPixel; ++iByte)
			{
				pPixels[dstOffset + iByte] = aInBuf[1 + iByte];
			}
			dstOffset += bytesPerPixel;

			// Read next directly into dst buffer
			if( runPixelCount > 1 )
			{
				// Read more

				// The "- bytesPerPixel" si here because we already read the first pixel whose size is bytesPerPixel
				int32 wantedByteCount = runByteCount - bytesPerPixel;
				read = file.Read(pPixels + dstOffset, wantedByteCount);
				if( read != wantedByteCount )
				{
					m_lastError = uncompleteFile;
					return false;
				}
				dstOffset += wantedByteCount;
			}
		}
	}

	return true;
}

bool Tga::ReadPalette(IFile& file, int16 colorMapOrigin, int16 colorMapLength, uint8 colorMapDepth)
{
	if( !(colorMapDepth == 15 || colorMapDepth == 16 || colorMapDepth == 24 || colorMapDepth == 32) )
	{
		m_lastError = errUnsupportedColorMapDepth;
		return false;
	}

	// Jump over unused colors
	file.SetPosition(colorMapOrigin, IFile::posCurrent);
	int32 colorCount = colorMapLength;
	if( colorCount > 256 )
	{
		// No support for more than 256 colors in palette
		m_lastError = errColorMapTooLarge;
		return false;
	}
	
	const int32 bytesPerColor = colorMapDepth / 8;
	const int32 wantedBytes = (colorMapLength - colorMapOrigin) * bytesPerColor;

	uint8 aCols[256 * 4];
	int32 readCol = file.Read(aCols, wantedBytes);
	if( readCol != wantedBytes )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	int32 iSrcColByte = 0;

	m_pal.m_count = colorCount;
	for(int i = 0; i < colorCount; ++i)
	{
		if( colorMapDepth == 32 )
		{
			uint8 b = aCols[iSrcColByte + 0];
			uint8 g = aCols[iSrcColByte + 1];
			uint8 r = aCols[iSrcColByte + 2];
			uint8 a = aCols[iSrcColByte + 3];

			m_pal.m_colors[i] = Color(r, g, b, a);
		}
		else if( colorMapDepth == 24 )
		{
			uint8 b = aCols[iSrcColByte + 0];
			uint8 g = aCols[iSrcColByte + 1];
			uint8 r = aCols[iSrcColByte + 2];
			
			m_pal.m_colors[i] = Color(r, g, b, 255);
		}
		else if( colorMapDepth == 16 || colorMapDepth == 15 )
		{
			uint8 low = aCols[iSrcColByte];
			uint8 nHi = aCols[iSrcColByte + 1];
			uint16 color = (nHi << 8) | low;
			m_pal.m_colors[i] = Color::From16bppRgb555(color);
		}
		
		iSrcColByte += bytesPerColor;
	}

	return true;
}

bool Tga::Decode1bppGrayScale(IFile& file)
{
	int32 bytesPerRow = Math::DivCeil(m_width, 8);
	int32 bufSize = bytesPerRow * m_height;

	if( !m_pixels.SetSize(bufSize) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	uint8* pPixels = m_pixels.GetWritePtr();

	int32 read = file.Read(pPixels, bufSize);
	if( read != bufSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	return true;
}

// Returns false if no footer or cannot be read
bool Tga::ReadFooter(IFile& file, TgaFooter& tf)
{
	if( !file.SetPosition(-26, IFile::posEnd) )
	{
		// Cannot seek
		return false;
	}

	if( file.Read(&tf, 26) == 26 )
	{
		if( file.ShouldSwapBytes() )
		{
			tf.extensionAreaOffset = IFile::Swap32(tf.extensionAreaOffset);
			tf.developerDirectoryOffset = IFile::Swap32(tf.developerDirectoryOffset);
		}

		if( tf.dot == '.' && tf.zero == 0x00 && Memory::Equals(tf.aSignature, "TRUEVISION-XFILE", 16) )
		{
			// Footer read ok
			return true;
		}
	}
	return false;
}

// This can help to determine if in a 32 bits images the alpha channel actually stores alpha values
void Tga::ReadNewTgaFormatUsefulInfo(IFile& file)
{
	int64 oldPos = file.GetPosition();
	if( oldPos < 0 )
	{
		// Cannot seek
		return;
	}

	TgaFooter tf;
	if( ReadFooter(file, tf) )
	{
		if( tf.extensionAreaOffset > 0 )
		{
			if( file.SetPosition(tf.extensionAreaOffset + 494) )
			{
				uint8 attributesTypeField24;
				if( file.Read8(attributesTypeField24) )
				{
					// Damn, that was not simple to get
					m_eAttributesType = AttributesType(attributesTypeField24);
				}
			}
		}
	}

	// Back to first position
	file.SetPosition(oldPos);
}
