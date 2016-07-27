///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Bmp.h"
#include "Array.h"
#include "Memory.h"
#include "Math.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;

static const uint8 k_BmpSignature[2] = {'B', 'M'};
//////////////////////////////////////////////////////////////////////

void Bmp::BmpHeader::SwapBytes()
{
	fileSize     = IFile::Swap32(fileSize);
	reserved     = IFile::Swap32(reserved);
	offsetToData = IFile::Swap32(offsetToData);
	bitmapHeaderSize = IFile::Swap32(bitmapHeaderSize);
	width  = IFile::Swap32(width);
	height = IFile::Swap32(height);
	planes = IFile::Swap16(planes);
	depth  = IFile::Swap16(depth);
	compression     = IFile::Swap32(compression);
	bitmapDataSize  = IFile::Swap32(bitmapDataSize);
	xPelsPerMeter   = IFile::Swap32(xPelsPerMeter);
	yPelsPerMeter   = IFile::Swap32(yPelsPerMeter);
	colorCount      = IFile::Swap32(colorCount);
	importantColors = IFile::Swap32(importantColors);
}

//////////////////////////////////////////////////////////////////////
Bmp::Bmp()
{
	Initialize();	
}

Bmp::~Bmp()
{

}

void Bmp::Initialize()
{
	m_width = 0;
	m_height = 0;
	m_lastError = 0;

	m_epf = PF_Unknown;
}

bool Bmp::IsBmp(IFile& file)
{
	const int32 signSize = sizeof(k_BmpSignature);
	uint8 aRead[signSize];
		
	if( file.Read(aRead, signSize) != signSize )
	{
		return false;
	}

	if( !chustd::Memory::Equals(k_BmpSignature, aRead, signSize) )
	{
		return false;
	}
	return true;
}


bool Bmp::LoadFromFile(IFile& file)
{
	/////////////////////////////////////////////////////
	// Reset the object
	FreeBuffer();
	Initialize();

	// The BMP format is little endian
	TmpEndianMode tem(file, boLittleEndian);

	/////////////////////////////////////////////////////
	// Read the header first
	if( !IsBmp(file) )
	{
		m_lastError = errNotABmpFile;
		return false;
	}

	BmpHeader bh;
	const int32 headerSize = sizeof(BmpHeader);
	if( file.Read(&bh, headerSize) != headerSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	if( file.ShouldSwapBytes() )
	{
		bh.SwapBytes();
	}

	////////////////////////////////////////////
	// Determine the version
	int32 bmpVersion = 0;
	if( bh.bitmapHeaderSize < 40 )
	{
		m_lastError = errBadHeaderSize;
		return false;
	}

	if( bh.bitmapHeaderSize == 40 )
	{
		bmpVersion = 3;
	}
	else
	{
		bmpVersion = 4;
	}
	////////////////////////////////////////////


	m_width = bh.width;
	m_height = bh.height;

	bool rowOrderTopToBottom = false;
	if( m_height < 0 )
	{
		// The row order is top to bottom
		rowOrderTopToBottom = true;
		m_height = -m_height;
	}

	if( bh.compression > compBitfields )
	{
		m_lastError = errUnsupportedCompressionFormat;
		return false;
	}

	if( bh.compression == compRle8bits || bh.compression == compRle4bits )
	{
		if( bh.depth != 4 && bh.depth != 8 )
		{
			m_lastError = errUnconsistentDepthAndCompression;
			return false;
		}
	}

	if( bh.compression == compBitfields )
	{
		if( bh.depth != 16 && bh.depth != 32 )
		{
			m_lastError = errUnconsistentDepthAndCompression;
			return false;
		}
	}

	if( bh.depth <= 8 )
	{
		if( bh.colorCount == 0 )
		{
			// Valid value, means use the maximum possible number of colors
			// for the current depth
			bh.colorCount = 1 << bh.depth;
		}

		if( bh.colorCount > 256 )
		{
			// Too many colors
			m_lastError = errUnconsistentDepthAndColorCount;
			return false;
		}
	}
	else
	{
		// True colors picture, reset the colorCount value which is not always 0
		// Some picture editors put 16777216 inside
		bh.colorCount = 0;
	}

	////////////////////////////////////////////////
	// Read the palette
	uint8 colors[256 * 4];
	const int32 paletteSize = bh.colorCount * 4;
	if( file.Read(colors, paletteSize) != paletteSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	
	m_ColorTable.m_count = bh.colorCount;

	for(int i = 0; i < bh.colorCount; ++i)
	{
		uint8 b = colors[4 * i + 0];
		uint8 g = colors[4 * i + 1];
		uint8 r = colors[4 * i + 2];
		
		m_ColorTable.m_colors[i].SetRgb(r, g, b);
	}
	
	////////////////////////////////////////////////
	// Get the pixel format
	PixelFormat pf = GetPfFromDepth(bh.depth);
	
	////////////////////////////////////////////////
	// Refine the pixel format
	uint32 maskRed = 0;
	uint32 maskGreen = 0;
	uint32 maskBlue = 0;
	uint32 maskAlpha = 0;

	if( bh.compression == compBitfields )
	{
		// Read the masks
		if( !(file.Read32(maskRed) && file.Read32(maskGreen) && file.Read32(maskBlue)) )
		{
			m_lastError = uncompleteFile;
			return false;
		}

		if( bmpVersion >= 4 )
		{
			file.Read32(maskAlpha);
		}

		if( bh.depth == 16 )
		{
			if( maskRed == 0x7C00 && maskGreen == 0x03E0 && maskBlue == 0x001F )
			{
				pf = PF_16bppRgb555;
			}
			else if( maskRed == 0xF800 && maskGreen == 0x07E0 && maskBlue == 0x001F )
			{
				pf = PF_16bppRgb565;
			}
			else
			{
				m_lastError = errUnsupportedBitfieldsFormat;
				return false;
			}
		}
		else if( bh.depth == 32 )
		{
			if( maskRed == 0x00FF0000 && maskGreen == 0x0000FF00 && maskBlue == 0x000000FF )
			{
				pf = PF_32bppBgra;
			}
			else
			{
				m_lastError = errUnsupportedBitfieldsFormat;
				return false;
			}
		}
		else
		{
			m_lastError = errUnsupportedBitfieldsFormat;
			return false;
		}
	}
	
	m_epf = pf;
	//////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// Read the pixels
	
	file.SetPosition(bh.offsetToData);

	bool readOk = ReadPixelData(file, bh);
	if( readOk )
	{
		if( pf == PF_32bppBgra && maskAlpha != 0xFF000000 )
		{
			// The byte used for alpha is not used, so we set every alpha bytes to 255
			SetAlphaFullOpaque();
		}

		if( !rowOrderTopToBottom )
		{
			// Flip is needed
			FlipVertical();
		}
	}

	file.SetPosition(bh.fileSize);
	return readOk;
}


bool Bmp::ReadPixelData(IFile& file, const BmpHeader& bh)
{
	uint16 depth = bh.depth;

	if( depth == 32 )
	{
		if( !ReadPixelsFromFile(file) )
		{
			return false;
		}

		return true;
	}
	
	if( depth == 24 )
	{
		if( !ReadPixelsFromFile(file) )
		{
			return false;
		}

		RemovePadding();
		return true;
	}
	
	if( depth == 16 )
	{
		if( !ReadPixelsFromFile(file) )
		{
			return false;
		}
		RemovePadding();

		// Arrange values from GGGBBBBB_ARRRRRGG to ARRRRRGG_GGGBBBBB in memory
		const int32 pixelCount = m_width * m_height;
		IFile::Swap16( (uint16*) m_pixels.GetWritePtr(), pixelCount);

		return true;
	}
	
	if( depth == 8 || depth == 4 || depth == 1 )
	{
		// There is padding at the end of each row in chustd image buffers for depth < 8
		const int32 niceBytePerRow = ImageFormat::ComputeByteWidth(m_epf, m_width);

		///////////////////////////////////////////////////////////////
		// Preparing Src values

		int32 srcBytesPerRow = ComputeBmpBytesPerRow(niceBytePerRow);
		int32 srcRowPadding = srcBytesPerRow - niceBytePerRow;

		int32 dataSize = srcBytesPerRow * m_height;
		
		ByteArray rbSrc;
		rbSrc.SetSize(dataSize);

		if( bh.compression == compRle8bits || bh.compression == compRle4bits )
		{
			// Should uncompress first
			int32 compressedSize = bh.fileSize - (int32) file.GetPosition();
			
			ByteArray rbCompressedData;
			rbCompressedData.SetSize(compressedSize);
			uint8* const pCompressedData = rbCompressedData.GetPtr();

			const int32 actuallyRead = file.Read(pCompressedData, compressedSize);
			if( actuallyRead != compressedSize )
			{
				m_lastError = errUnexpectedEndOfFile;
				return false;
			}

			if( bh.compression == compRle4bits )
			{
				// RleUncompress works with a destination buffer of 8 bits pixels
				rbSrc.SetSize( rbSrc.GetSize() * 2);
			}

			if( !RleUncompress(rbCompressedData, rbSrc, m_width, int32(bh.depth)) )
			{
				m_lastError = errBadCompressedData;
				return false;
			}
			
			// RleUncompress removes the uint32 alignment of bmp
			srcRowPadding = 0;
		}
		else if( bh.compression == compNone )
		{
			file.Read(rbSrc.GetPtr(), rbSrc.GetSize());
		}
		else
		{
			ASSERT(0);
			return false;
		}

		int32 srcOffset = 0;
		uint8* const pSrc = rbSrc.GetPtr();
		///////////////////////////////////////////////////////////////
		
		///////////////////////////////////////////////////////////////
		// Preparing Dst values
		const int32 dstBytesPerRow = niceBytePerRow;

		int32 dstOffset = 0;
		const int32 nextDstRowOffset = 0;

		if( !m_pixels.SetSize(dstBytesPerRow * m_height) )
		{
			m_lastError = notEnoughMemory;
			return false;
		}
		uint8* const pDst = m_pixels.GetWritePtr();

		for(int iVert = 0; iVert < m_height; iVert++)
		{
			if( bh.compression == compRle4bits )
			{
				// Repack with two pixels per byte
				for(int iHrz = 0; iHrz < m_width; iHrz++)
				{
					if( (iHrz & 1) == 0 )
					{
						pDst[dstOffset + iHrz / 2] = uint8(pSrc[srcOffset + iHrz] << 4);
					}
					else
					{
						pDst[dstOffset + iHrz / 2] |= pSrc[srcOffset + iHrz];
					}
				}
				srcOffset += m_width;
				dstOffset += dstBytesPerRow;
			}
			else
			{
				for(int iHrz = 0; iHrz < dstBytesPerRow; iHrz++)
				{
					pDst[dstOffset] = pSrc[srcOffset];
					
					srcOffset += 1;
					dstOffset += 1;
				}
			}
			srcOffset += srcRowPadding;
			dstOffset -= nextDstRowOffset;
		}
		return true;
	}

	m_lastError = errDepthNotSupported;
	return false;
}

////////////////////////////////////////////////////////////////
// Uncompress this weirdo RLE BMP compressed pixel data for depths of 4 and 8 bits
bool Bmp::RleUncompress(const ByteArray& rbSrc, ByteArray& rbDst, int32 niceBytePerRow, int32 depth)
{
	const uint8* pSrc = rbSrc.GetPtr();
	const int32 srcSize = rbSrc.GetSize();

	uint8* pDst = rbDst.GetPtr();
	const int32 dstSize = rbDst.GetSize();

	enum { stateCalm, stateGetColorIndex, stateGetEscape, 
		stateGetHrzDelta, stateGetVrtDelta, stateGetSinglePixels };

	int state = stateCalm;

	// Values stored during the parsing
	uint8 repeatCount = 0;
	uint8 hrzDelta = 0;
	uint8 vrtDelta = 0;
	uint8 absRepeatCount = 0;

	int32 iSrc = 0;
	int32 iDst = 0;

	int32 iDstStartRow = 0;

	while( iSrc < srcSize)
	{
		if( state == stateCalm )
		{
			uint8 code = pSrc[iSrc];
			if( code > 0 )
			{
				state = stateGetColorIndex;
				repeatCount = code;
			}
			else
			{
				state = stateGetEscape;
			}
			++iSrc;
		}
		else if( state == stateGetColorIndex )
		{
			uint8 colorIndex = pSrc[iSrc];

			if( iDst + repeatCount > dstSize )
			{
				return false;
			}
			
			if( depth == 8 )
			{
				for(int i = 0; i < repeatCount; ++i)
				{
					pDst[iDst + i] = colorIndex;
				}
			}
			else
			{
				for(int i = 0; i < repeatCount; ++i)
				{
					pDst[iDst + i] = uint8(((i & 1) == 0) ? (colorIndex >> 4) : (colorIndex & 0x0f));
				}
			}
			iDst += repeatCount;
			state = stateCalm;
			++iSrc;
		}
		else if( state == stateGetEscape )
		{
			uint8 escape = pSrc[iSrc];

			if( escape == 0 )
			{
				// 0 = End of line
				const int32 dstNextRow = iDstStartRow + niceBytePerRow;

				// Fill the remaining _ROW_ with 0
				int32 fillCount = dstNextRow - iDst;
				if( fillCount != 0 )
				{
					Memory::Zero(pDst + iDst, fillCount);
				}

				// A new row is starting
				iDst = dstNextRow;
				iDstStartRow = dstNextRow;

				state = stateCalm;
			}
			else if( escape == 1 )
			{
				// End of bitmap

				// Fill the remaining _DST-DATA_ with 0
				int32 fillCount = dstSize - iDst;
				if( fillCount != 0 )
				{
					Memory::Zero(pDst + iDst, fillCount);
				}
				return true;
			}
			else if( escape == 2 )
			{
				// delta
				state = stateGetHrzDelta;
			}
			else
			{
				// Absolute mode
				
				absRepeatCount = escape;
				state = stateGetSinglePixels;
			}

			++iSrc;
		}
		else if( state == stateGetSinglePixels )
		{
			int32 srcByteCount;
			if( depth == 4 )
			{
				srcByteCount = Math::DivCeil(absRepeatCount, 2);
			}
			else
			{
				srcByteCount = absRepeatCount;
			}

			if( iSrc + srcByteCount > srcSize )
			{
				return false;
			}

			if( iDst + absRepeatCount > dstSize )
			{
				return false;
			}
		
			if( depth == 8 )
			{
				for(int i = 0; i < absRepeatCount; ++i)
				{
					pDst[iDst + i] = pSrc[iSrc + i];
				}
			}
			else
			{
				for(int i = 0; i < absRepeatCount; ++i)
				{
					uint8 colorIndex = pSrc[iSrc + i / 2];
					pDst[iDst + i] = uint8(((i & 1) == 0) ? (colorIndex >> 4) : (colorIndex & 0x0f));
				}
			}

			iSrc += srcByteCount;
			iDst += absRepeatCount;

			// In absolute mode, each run must be aligned on a word boundary
			if( (srcByteCount & 1) != 0 )
			{
				++iSrc;
			}

			state = stateCalm;
		}
		else if( state == stateGetHrzDelta )
		{
			hrzDelta = pSrc[iSrc];
			state = stateGetVrtDelta;

			++iSrc;
		}
		else if( state == stateGetVrtDelta )
		{
			vrtDelta = pSrc[iSrc];
			
			// Move the current position
			const int32 newDst = iDst + vrtDelta * niceBytePerRow + hrzDelta;
			if( newDst > dstSize )
				return false;

			int32 fillCount = newDst - iDst;

			Memory::Zero(pDst + iDst, fillCount);

			iDst = newDst;
			iDstStartRow += vrtDelta * niceBytePerRow;

			state = stateCalm;

			iSrc++;
		}
		else
		{
			ASSERT(0);
			return false;
		}
	}
	return false; // Should end with an EndOfBitmap escape
}

const Palette& Bmp::GetPalette() const
{
	return m_ColorTable;
}

PixelFormat Bmp::GetPixelFormat() const
{
	return m_epf;
}

int32 Bmp::ComputeBmpBytesPerRow(int32 expectedBytesPerRow)
{
	const int32 remainBit1 = (expectedBytesPerRow >> 1) & 1;
	const int32 remainBit0 = (expectedBytesPerRow >> 0) & 1;
	const int32 newByteCount = ( (expectedBytesPerRow >> 2) + (remainBit1 | remainBit0)) * 4;
	return newByteCount;
}

PixelFormat Bmp::GetPfFromDepth(uint16 depth) const
{
	if( depth == 32 )
	{
		return PF_32bppBgra;
	}
	else if( depth == 24 )
	{
		return PF_24bppBgr;
	}
	else if( depth == 16 )
	{
		return PF_16bppRgb555;
	}
	else if( depth == 8 )
	{
		return PF_8bppIndexed;
	}
	else if( depth == 4 )
	{
		return PF_4bppIndexed;
	}
	else if( depth == 1 )
	{
		return PF_1bppIndexed;
	}
	
	return PF_Unknown;
}

bool Bmp::Dump(const String& filePath, const BmpDumpSettings& ds)
{
	File file;
	if( !file.Open(filePath, File::modeWrite) )
		return false;

	return Dump(file, ds);
}

bool Bmp::Dump(IFile& file, const BmpDumpSettings& ds)
{
	const uint8* pSrc = ds.pixels.GetReadPtr();
	const int32 width = ds.width;
	const int32 height = ds.height;
	PixelFormat epf = ds.pixelFormat;

	int16 depth = (int16) ImageFormat::SizeofPixelInBits(epf);
	if( depth != 24 )
	{
		// Not supported
		return false;
	}

	TmpEndianMode tem(file, boLittleEndian);

	file.Write(k_BmpSignature, sizeof(k_BmpSignature));

	// Complete file size in bytes
	int64 fieldPosition_FileSize = file.GetPosition();
	
	// Offset from beginning of the file to the beginning of the image data
	int64 fieldPosition_OffsetToData = 4 + 4 + file.GetPosition();
	
	// Number of colors
	int32 colorCount = 0;
	if( ds.palette.m_count != 0 )
	{
		colorCount = ds.palette.m_count;
	}

	BmpHeader bh;
	bh.fileSize = 0;
	bh.reserved = 0;
	bh.offsetToData = 0;
	bh.bitmapHeaderSize = 40;
	bh.width = width;
	bh.height = height;
	bh.planes = 1;
	bh.depth = depth;  // Possible values : 1, 4, 8, 16, 24, 32
	bh.compression = 0; // 0=None (BI_RGB), 1=RLE 8bits, 2=RLE 4bits, 3=BI_BITFIELDS
	bh.bitmapDataSize = 0; // Rounded to the next 4 byte boundary
	bh.xPelsPerMeter = 2834;
	bh.yPelsPerMeter = 2834;
	bh.colorCount = colorCount;
	bh.importantColors = 0;

	if( file.ShouldSwapBytes() )
	{
		bh.SwapBytes();
	}
	
	int32 wantedWritten = sizeof(BmpHeader);
	if( file.Write(&bh, wantedWritten) != wantedWritten )
	{
		return false;
	}

	////////////////////////////////////////////////
	// Write the palette
	if( colorCount != 0 )
	{
		uint8 aPalBuf[256 * 4];
		uint8* pPal = aPalBuf;

		for(int i = 0; i < colorCount; ++i)
		{
			ds.palette.m_colors[i].ToRgba( pPal[2], pPal[1], pPal[0], pPal[3]);

			pPal += 4;
		}

		const int32 palByteSize = ds.palette.m_count * 4;
		if( file.Write(aPalBuf, palByteSize) != palByteSize )
		{
			return false;
		}
	}
	////////////////////////////////////////////////

	//////////////////////////////////////////////////
	uint32 offsetToData = (uint32) file.GetPosition();

	if( depth == 24 )
	{
		// Each row in a bmp file must be uint32 aligned
		int32 pad = (int32(width * 3) & 0x0003);
		if( pad != 0 )
		{
			pad = 4 - pad;
		}

		ByteArray aRow;
		aRow.SetSize(width * 3);

		for(int iRow = 0; iRow < height; ++iRow)
		{
			const uint8* pWriteSrc = pSrc + ((height - 1) - iRow) * width * 3;
			
			if( epf == PF_24bppBgr )
			{
				file.Write(pWriteSrc, width * 3);
			}
			else if( epf == PF_24bppRgb )
			{
				// Swap red and blue
				for(int i = 0; i < width; ++i)
				{
					uint8 r = pWriteSrc[3 * i];
					uint8 g = pWriteSrc[3 * i + 1];
					uint8 b = pWriteSrc[3 * i + 2];
					aRow[3 * i] = b;
					aRow[3 * i + 1] = g;
					aRow[3 * i + 2] = r;
				}
				file.Write(aRow.GetPtr(), aRow.GetSize());
			}
			
			if( pad != 0 )
			{
				int32 dummy = 0;
				file.Write(&dummy, pad);
			}
		}
	}
	//////////////////////////////////////////////////

	uint32 fileSize = (uint32) file.GetPosition();
	file.SetPosition(fieldPosition_FileSize);
	if( !file.Write32(fileSize) )
	{
		return false;
	}

	file.SetPosition(fieldPosition_OffsetToData);
	if( !file.Write32(offsetToData) )
	{
		return false;
	}

	/////////////////////////////////
	return true;
}


String Bmp::GetLastErrorString() const
{
	switch(m_lastError)
	{
	case Bmp::errNotABmpFile:
		return "Not a BMP file";

	case Bmp::errBadHeaderSize:
		return "Bad header size";

	case Bmp::errUnconsistentDepthAndCompression:
		return "Unconsistent depth and compression";

	case Bmp::errUnconsistentDepthAndColorCount:
		return "Unconsistent depth and color count";

	case Bmp::errUnsupportedCompressionFormat:
		return "Unsupported compression format";

	case Bmp::errUnsupportedBitfieldsFormat:
		return "Unsupported bitfields format";

	case Bmp::errDepthNotSupported:
		return "Depth not supported (accepted : 32, 24, 16, 8, 4, 1) for BMP files";

	case Bmp::errUnexpectedEndOfFile:
		return "Unexpected end of file";

	case Bmp::errBadCompressedData:
		return "Bad compressed data";

	case Bmp::errTooManyColors:
		return "Too many colors";
	}

	return ImageFormat::GetLastErrorString();
}

bool Bmp::ReadPixelsFromFile(IFile& file)
{
	int32 srcBytesPerRow = ImageFormat::ComputeByteWidth(m_epf, m_width);
	int32 bmpBytesPerRow = ComputeBmpBytesPerRow(srcBytesPerRow);
	const int32 dataSize = bmpBytesPerRow * m_height;
	
	if( !m_pixels.SetSize(dataSize) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}

	uint8* pPixels = m_pixels.GetWritePtr();

	int32 read = file.Read(pPixels, dataSize);
	if( read != dataSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	return true;
}

// Remove the last unused bytes used by the Bmp pixels so the row are 32 bits aligned
void Bmp::RemovePadding()
{
	int32 wantedBytesPerRow = ImageFormat::ComputeByteWidth(m_epf, m_width);
	int32 bmpBytesPerRow = ComputeBmpBytesPerRow(wantedBytesPerRow);

	if( wantedBytesPerRow == bmpBytesPerRow )
	{
		// Not needed ! :)
		return;
	}

	uint8* pSrc = m_pixels.GetWritePtr();
	uint8* pDst = pSrc;

	const int32 height = m_height;
	for(int iY = 0; iY < height; ++iY)
	{
		Memory::Move(pDst, pSrc, wantedBytesPerRow);

		pSrc += bmpBytesPerRow;
		pDst += wantedBytesPerRow;
	}
}
