///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageFormat.h"
#include "File.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;

//////////////////////////////////////////////////////////////////////
Color Color::Black(0, 0, 0);
Color Color::White(255, 255, 255);
Color Color::Red(255, 0, 0);
Color Color::Green(0, 255, 0);
Color Color::Blue(0, 0, 255);
Color Color::Yellow(255, 255, 0);
Color Color::Cyan(0, 255, 255);
Color Color::Magenta(255, 0, 255);
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
PixelFormat AnimFrame::GetPixelFormat() const
{
	if( m_pOwner != nullptr )
	{ 
		return m_pOwner->GetPixelFormat();
	}
	return PF_Unknown;
}

bool AnimFrame::HasSimpleTransparency() const
{
	if( m_pOwner != nullptr )
	{ 
		return m_pOwner->HasSimpleTransparency();
	}
	return false;
}

uint16 AnimFrame::GetGreyTransIndex() const
{
	if( m_pOwner != nullptr )
	{
		return m_pOwner->GetGreyTransIndex();
	}
	return 0;
}

void AnimFrame::GetTransIndexes(uint16& red, uint16& green, uint16& blue) const
{
	if( m_pOwner != nullptr )
	{
		return m_pOwner->GetTransIndexes(red, green, blue);
	}
	red = green = blue = 0;
}

const Palette& AnimFrame::GetPalette() const
{
	if( m_pOwner != nullptr )
	{
		return m_pOwner->GetPalette();
	}
	return Palette::Null;
}

//////////////////////////////////////////////////////////////////////
const Palette Palette::Null;

//////////////////////////////////////////////////////////////////////
// Returns true if one of the color has a alpha not equal to 255
bool Palette::HasNonOpaqueColor() const
{
	for(int i = 0; i < m_count; ++i) 
	{
		if( m_colors[i].GetAlpha() != 255 )
		{
			return true;
		}
	}
	return false;
}

bool Palette::AllAlphasAreOpaque() const
{
	for(int i = 0; i < m_count; ++i) 
	{
		if( m_colors[i].GetAlpha() != 0 )
			return false;
	}
	return true;
}

void Palette::SetAlphaFullOpaque()
{
	for(int i = 0; i < m_count; ++i) 
	{
		m_colors[i].SetAlpha(255);
	}
}

// Returns -1 if not found
int Palette::FindColor(Color col) const
{
	for(int i = 0; i < m_count; ++i) 
	{
		if( m_colors[i] == col )
		{
			return i;
		}
	}
	return -1;
}

// Returns -1 if not found
int Palette::GetFirstFullyTransparentColor() const
{
	for(int i = 0; i < m_count; ++i) 
	{
		if( m_colors[i].a == 0 )
		{
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////
ImageFormat::ImageFormat()
{
	m_width = 0;
	m_height = 0;
	m_lastError = 0;
}

ImageFormat::~ImageFormat()
{

}

////////////////////////////////////////////////////
bool ImageFormat::IsIndexed(PixelFormat pf)
{
	return pf == PF_1bppIndexed || pf == PF_2bppIndexed || pf == PF_4bppIndexed || pf == PF_8bppIndexed;
}

int32 ImageFormat::SizeofPixelInBits(PixelFormat ePixelFormat)
{
	switch(ePixelFormat)
	{
	case PF_Unknown:
		return 0;
		
	case PF_1bppGrayScale:
		return 1;
	case PF_2bppGrayScale:
		return 2;
	case PF_4bppGrayScale:
		return 4;
	case PF_8bppGrayScale:
		return 8;
	case PF_16bppGrayScale:
		return 16;
		
	case PF_16bppGrayScaleAlpha:
		return 16;
	case PF_32bppGrayScaleAlpha:
		return 32;

	case PF_1bppIndexed:
		return 1;
	case PF_2bppIndexed:
		return 2;
	case PF_4bppIndexed:
		return 4;
	case PF_8bppIndexed:
		return 8;
		
	case PF_16bppArgb1555:
	case PF_16bppArgb4444:
	case PF_16bppRgb555:
	case PF_16bppRgb565:
		return 16;

	case PF_24bppRgb:
	case PF_24bppBgr:
		return 24;

	case PF_32bppRgba:
	case PF_32bppBgra:
		return 32;

	case PF_48bppRgb:
		return 48;

	case PF_64bppRgba:
		return 64;

	}
	ASSERT(0);
	return 0;
}

int32 ImageFormat::ComputeByteWidth(PixelFormat epf, int32 width)
{
	const int32 sizeofPixelInBits = ImageFormat::SizeofPixelInBits(epf);

	// Size of a row in bits
	const int32 bitWidth = width * sizeofPixelInBits;
	
	// Round up for size of a row in bytes
	const int32 addWidth = ((bitWidth & 7) != 0) ? 1 : 0;
	const int32 byteWidth = bitWidth / 8 + addWidth;

	return byteWidth;
}
//////////////////////////////////////////////////////////////////////////////

String ImageFormat::GetLastErrorString() const
{
	switch(m_lastError)
	{
	case ioErr:
		return "Error accessing to the file";
	case badFileFormat:
		return "Bad File Format";
	case notEnoughMemory:
		return "Not enough memory";
	case uncompleteFile:
		return "Uncomplete file";
	}
	return "Unknown error";
}

bool ImageFormat::Load(const String& filePath)
{
	File file;
	if( !file.Open(filePath) )
	{
		m_lastError = ioErr;
		return false;
	}
	return LoadFromFile(file);
}

int32 ImageFormat::GetWidth() const
{
	return m_width;
}

int32 ImageFormat::GetHeight() const
{
	return m_height;
}

const Buffer& ImageFormat::GetPixels() const
{
	return m_pixels;
}

void ImageFormat::FreeBuffer()
{
	m_pixels.Clear();
}

void ImageFormat::FlipVertical()
{
	PixelFormat epf = GetPixelFormat();
	if( epf == PF_Unknown )
	{
		return;
	}
	const int32 bytesPerRow = ImageFormat::ComputeByteWidth(epf, m_width);
	uint8* pComponents = m_pixels.GetWritePtr();
	const int32 heightDiv2 = m_height / 2;

	for(int iY = 0; iY < heightDiv2; ++iY)
	{
		uint8* pSrc = pComponents + (iY * bytesPerRow);
		uint8* pDst = pComponents + ((m_height - 1 - iY) * bytesPerRow);

		for(int iByte = 0; iByte < bytesPerRow; ++iByte)
		{
			uint8 swap = pDst[iByte];
			pDst[iByte] = pSrc[iByte];
			pSrc[iByte] = swap;
		}
	}
}

void ImageFormat::SetAlphaFullOpaque()
{
	PixelFormat epf = GetPixelFormat();
	if( epf == PF_Unknown )
	{
		return;
	}

	const int32 pixelCount = m_width * m_height;
	if( epf == PF_32bppRgba || epf == PF_32bppBgra )
	{
		uint8* pDst = m_pixels.GetWritePtr();
		const uint8 alpha = 255;
		for(int i = 0; i < pixelCount; ++i)
		{
			pDst[3] = alpha;
			pDst += 4;
		}

	}
}

int32 ImageFormat::GetLastError() const
{ 
	return m_lastError; 
}

bool ImageFormat::IsIndexed() const 
{ 
	return IsIndexed(GetPixelFormat()); 
}


bool ImageFormat::IsAnimated() const
{
	return false;
}

bool ImageFormat::HasDefaultImage() const
{
	return false;
}

int32 ImageFormat::GetFrameCount() const
{
	return 0;
}

const AnimFrame* ImageFormat::GetAnimFrame(int /*index*/) const
{
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// If animated, gets the number of times the animation loops.
int32 ImageFormat::GetLoopCount() const
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Packs single-pixel bytes into multiple-pixels bytes.
//
// [in,out]  pixels       Pixels in: one pixel per byte, out: multiple pixels per byte
// [in]      width        Width in pixels
// [in]      height       Height in pixels
// [in]      pixelFormat  Target pixel format
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ImageFormat::PackPixels(Buffer& pixels, int width, int height, PixelFormat pixelFormat)
{
	if( width < 0 || height < 0 )
	{
		return false; // Bad arg
	}
	int bitsPerPix = ImageFormat::SizeofPixelInBits(pixelFormat);
	if( bitsPerPix > 8 )
	{
		return false; // Bad arg
	}
	if( bitsPerPix == 8 )
	{
		// Nothing to do
		return true;
	}
	if( pixels.GetSize() < (width * height) )
	{
		ASSERT(0);
		return false;
	}
	const int32 bytesPerRow = ImageFormat::ComputeByteWidth(pixelFormat, width);
	uint8* pIn = pixels.GetWritePtr();
	uint8* pOut = pIn;
	
	// Note: there is padding at the end of each row
	uint8 currentByte = 0; // Current byte value
	int usedBitCount = 0;  // Number of bits used inside the byte

	for(int iRow = 0; iRow < height; iRow++)
	{
		currentByte = 0;
		usedBitCount = 0;
		
		for(int iCol = 0; iCol < width; iCol++)
		{
			uint8 nVal = pIn[iRow * width + iCol];
			
			usedBitCount += bitsPerPix;
			currentByte |= (nVal << (8 - usedBitCount));
			
			if( usedBitCount == 8 )
			{
				pOut[0] = currentByte;
				++pOut;
				
				currentByte = 0;
				usedBitCount = 0;
			}
		}
		
		if( usedBitCount != 0 )
		{
			pOut[0] = currentByte;
			++pOut;
		}
	}
	int newSize = bytesPerRow * height;
	pixels.SetSize(newSize);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Unpacks multiple pixels per byte to get one pixel per byte.
//
// [in,out] pixels      Pixels in: one pixel per bytes, out: multiple pixels per byte
// [in]     width       Width in pixels
// [in]     height      Height in pixels
// [in]     pixelFormat Source pixel format
//
// Returns true upon success.
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ImageFormat::UnpackPixels(Buffer& pixels, int width, int height, PixelFormat pixelFormat)
{
	int bitsPerPix = ImageFormat::SizeofPixelInBits(pixelFormat);
	if( bitsPerPix >= 8 )
	{
		// Nothing to do
		return true;
	}
	Buffer newBuffer;
	if( !newBuffer.SetSize(width * height) )
	{
		return false;
	}

	const uint8* pSrc = pixels.GetReadPtr();
	uint8* pDst = newBuffer.GetWritePtr();

	int32 mask = ~((~0UL) << bitsPerPix);
	int32 shift = 8 - bitsPerPix;
	
	int srcIndex = 0;
	int dstIndex = 0;

	for(int i = 0; i < height; ++i)
	{
		for(int j = 0; j < width; ++j)
		{
			uint8 currentByte = pSrc[srcIndex];

			uint8 colorIndex = uint8( (currentByte >> shift) & mask);
			
			shift -= bitsPerPix;
			if( shift < 0 )
			{
				shift = 8 - bitsPerPix;
				srcIndex++;
			}
			
			pDst[dstIndex] = colorIndex;
			dstIndex++;
		}
		// Some padding bits may remain
		if( shift != 8 - bitsPerPix )
		{
			shift = 8 - bitsPerPix;
			srcIndex++;
		}
	}
	pixels = newBuffer;
	return true;
}
