///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PngDumper.h"
#include "TextEncoding.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Dumps image data as a PNG file
bool PngDumper::Dump(const String& filePath, const PngDumpData& dd, const PngDumpSettings& ds)
{
	File file;
	if( !file.Open(filePath, File::modeWrite) )
	{
		return false;
	}
	return Dump(file, dd, ds);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Dumps an image or animation in the PNG format.
//
// [in,out] fileDst File to write to
// [in]     dd      Dump data
// [in]     ds      Dump settings
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::Dump(IFile& fileDst, const PngDumpData& dd, const PngDumpSettings& ds)
{
	PixelFormat epf = dd.pixelFormat;
	const int32 width = dd.width;
	const int32 height = dd.height;
	const Palette& palette = dd.palette;

	if( epf == PF_32bppBgra )
	{
		// Accept BGRA pixels, they will be reverted in CreateImageData()
		epf = PF_32bppRgba;
	}

	uint8 colorType;
	uint8 bitsPerComponent;
	if( !GetIHDRFeaturesFromPixelFormat(epf, colorType, bitsPerComponent) )
	{
		return false;
	}

	if( (colorType & PngChunk_IHDR::CTF_Palette) && palette.m_count == 0 )
	{
		return false;
	}

	/////////////////////////////////////////////////////////////////////////
	// Send the data to the stream
	
	// Start to send the png signature
	if( !WriteSignature(fileDst) )
	{
		return false;
	}

	// First chunk : the header	
	ChunkedFile file(fileDst);
	
	// 1 = Adam7
	uint8 interlaceMethod = dd.interlaced ? uint8(1) : uint8(0);
	
	file.BeginChunkWrite(PngChunk_IHDR::Name);
	
	PngChunk_IHDR hi;
	hi.width = width;
	hi.height = height;
	hi.bitDepth = bitsPerComponent;
	hi.colorType = colorType;
	hi.compressionMethod = 0; // 0 = deflate/inflate
	hi.filterMethod = 0;      // 0 = Adaptative
	hi.interlaceMethod = interlaceMethod;
	
	if( fileDst.ShouldSwapBytes() )
	{
		hi.SwapBytes();
	}
	
	const int32 headerSize = PngChunk_IHDR::DataSize;
	if( file.Write(&hi, headerSize) != headerSize )
	{
		return false;
	}

	if( !file.EndChunkWrite() )
	{
		return false;
	}

	if( colorType & PngChunk_IHDR::CTF_Palette )
	{
		// Palettized picture
		const int32 colorCount = palette.m_count;

		if( !(0 <= colorCount && colorCount <= 256) )
		{
			// Bad number of colors
			return false;
		}

		/////////////////////////////////////////////////////////////////////////
		// Write the palette colors
		file.BeginChunkWrite(PngChunk_PLTE::Name);
		
		uint8 aTmpPalette[256 * 3];
		int32 iTmpColor = 0;
		for(int i = 0; i < colorCount; ++i)
		{
			uint8 r, g, b, a;
			palette[i].ToRgba(r, g, b, a);

			aTmpPalette[iTmpColor + 0] = r;
			aTmpPalette[iTmpColor + 1] = g;
			aTmpPalette[iTmpColor + 2] = b;
			iTmpColor += 3;
		}

		file.Write( aTmpPalette, colorCount * 3);

		if( !file.EndChunkWrite() )
			return false;
	
		/////////////////////////////////////////////////////////////////////////
		// Check if we need to write a tRNS chunk
		// We don't if all alphas are 255
		// Note : an external png optimizer should sort the palette colors by their
		// alpha values so the lowest alpha is found first
		
		int32 alphaCount = 0;
		for(int i = colorCount - 1; i >= 0; --i)
		{
			if( palette[i].GetAlpha() != 255 )
			{
				alphaCount = i + 1;
				break;
			}
		}

		if( alphaCount > 0 )
		{
			// tRNS chunk needed
			uint8 aAlphas[256];
			for(int i = 0; i < alphaCount; ++i)
			{
				aAlphas[i] = palette[i].GetAlpha();
			}

			file.BeginChunkWrite(PngChunk_tRNS::Name);
			
			file.Write(aAlphas, alphaCount);

			if( !file.EndChunkWrite() )
				return false;
		}
	}
	else if( colorType == 0 )
	{
		// Grey
		if( dd.useTransparentColor )
		{
			file.BeginChunkWrite(PngChunk_tRNS::Name);
			file.Write16(dd.tRNS.grey);
			
			if( !file.EndChunkWrite() )
				return false;
		}
	}
	else if( colorType == 2 )
	{
		// True colors
		if( dd.useTransparentColor )
		{
			file.BeginChunkWrite(PngChunk_tRNS::Name);
			file.Write16(dd.tRNS.red);
			file.Write16(dd.tRNS.green);
			file.Write16(dd.tRNS.blue);

			if( !file.EndChunkWrite() )
				return false;
		}
	}

	////////////////////////////////////////////////////
	// bKGD chunk
	if( dd.useBackgroundColor )
	{
		if( !WriteChunk_bkGD(file, colorType, dd.bkGD) )
		{
			return false;
		}
	}

	////////////////////////////////////////////////////
	// pHYs chunk
	if( dd.usePhys )
	{
		if( !WriteChunk_pHYs(file, dd.pHYs) )
		{
			return false;
		}
	}

	////////////////////////////////////////////////////
	// tEXt chunk
	foreach(dd.textInfos, i)
	{
		if( !WriteChunk_tEXt(file, dd.textInfos[i]) )
		{
			return false;
		}
	}

	////////////////////////////////////////////////////
	// Now it's time for image data
	const uint8* pIdatSrc = dd.pixels.GetReadPtr();

	int32 apngSequenceNumber = 0;
	int32 iFrame = 0;

	int32 frameCount = dd.frames.GetSize();
	if( frameCount > 0 )
	{
		// Aha, an animation, put the acTL chunk first
		file.BeginChunkWrite(PngChunk_acTL::Name);
		if( !(file.Write32(frameCount) && file.Write32(dd.loopCount)) )
		{
			return false;
		}
		if( !file.EndChunkWrite() )
		{
			return false;
		}

		if( !dd.hasDefaultImage )
		{
			// The IDAT is part of the animation and its pixels are in apFrames[0].
			
			const ApngFrame* pFrame = dd.frames[0];

			// Check that the size matches the IHDR
			bool bOk1 = (pFrame->m_fctl.width == dd.width);
			bool bOk2 = (pFrame->m_fctl.height == dd.height);
			bool bOk3 = (pFrame->m_fctl.offsetX == 0);
			bool bOk4 = (pFrame->m_fctl.offsetY == 0);

			if( !(bOk1 && bOk2 && bOk3 && bOk4) )
			{
				// Nop
				return false;
			}

			// Write a fcTL first.
			if( !WriteFrameControlChunk(pFrame, apngSequenceNumber, file) )
			{
				return false;
			}
			apngSequenceNumber++;

			pIdatSrc = pFrame->GetPixels().GetReadPtr();
			iFrame++;
		}
	}

	/////////////////////////////////////////////////
	// Write the IDAT
	file.BeginChunkWrite(PngChunk_IDAT::Name);

	ByteArray abImageData;
	if( !CreateImageData(pIdatSrc, width, height, dd, ds, abImageData) )
	{
		return false;
	}
	if( !file.Write(abImageData.GetPtr(), abImageData.GetSize()) )
	{
		return false;
	}
	if( !file.EndChunkWrite() )
	{
		return false;
	}
	/////////////////////////////////////////////////
	
	/////////////////////////////////////////////////
	// Write remaining APNG frames
	ByteArray frameImageData;

	for(; iFrame < frameCount; ++iFrame)
	{
		const ApngFrame* pFrame = dd.frames[iFrame];

		/////////////////////////
		// Write a fcTL
		if( !WriteFrameControlChunk(pFrame, apngSequenceNumber, file) )
		{
			return false;
		}
		apngSequenceNumber++;
		/////////////////////////

		/////////////////////////
		// Write the fdAT
		file.BeginChunkWrite(PngChunk_fdAT::Name);

		if( !file.Write32(apngSequenceNumber) )
		{
			return false;
		}
		apngSequenceNumber++;

		const uint8* pFramePixels = pFrame->GetPixels().GetReadPtr();
		const int32 frameWidth = pFrame->GetWidth();
		const int32 frameHeight = pFrame->GetHeight();

		if( !CreateImageData(pFramePixels, frameWidth, frameHeight, dd, ds, frameImageData) )
		{
			return false;
		}
		if( !file.Write(frameImageData.GetPtr(), frameImageData.GetSize()) )
		{
			return false;
		}
		if( !file.EndChunkWrite() )
		{
			return false;
		}
		/////////////////////////
	}

	/////////////////////////////////////////////////
	// Last chunk : end chunk
	file.BeginChunkWrite(PngChunk_IEND::Name);
	if( !file.EndChunkWrite() )
	{
		return false;
	}	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Writes the signature of the PNG file.
//
// [in,out] file  Destination file
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::WriteSignature(IFile& file)
{
	int32 written = file.Write(k_PngSignature, 8);
	return written == 8;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Writes a bkGD chunk (background color)
//
// [in,out] cf  Destination chunked file
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::WriteChunk_bkGD(ChunkedFile& cf, uint8 colorType, const PngChunk_bkGD& content)
{
	if( colorType == 0x03 && content.index >= 0 )
	{
		// Indexed color
		cf.BeginChunkWrite(PngChunk_bKGD::Name);
		cf.Write8(content.index);
		if( !cf.EndChunkWrite() )
			return false;
	}
	else if( (colorType == 0x00 || colorType == 0x04) && content.grey >= 0 )
	{
		// Grey or Grey+Alpha
		cf.BeginChunkWrite(PngChunk_bKGD::Name);
		cf.Write16(content.grey);
		if( !cf.EndChunkWrite() )
			return false;
	}
	else if( (colorType == 0x02 || colorType == 0x06) && content.red >= 0 )
	{
		// TrueColor or TrueColor+Alpha
		cf.BeginChunkWrite(PngChunk_bKGD::Name);
		cf.Write16(content.red);
		cf.Write16(content.green);
		cf.Write16(content.blue);
		if( !cf.EndChunkWrite() )
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::WriteChunk_tEXt(ChunkedFile& cf, const PngChunk_tEXt& content)
{
	if( content.keyword.IsEmpty() )
	{
		return true;
	}
	ByteArray kwBytes = content.keyword.ToBytes(TextEncoding::Iso8859_1());
	int kwLength = kwBytes.GetSize();
	if( kwLength > 79 )
	{
		kwLength = 79; // Truncate keyword, 79 is the max as per the PNG spec
	}
	ByteArray chunkBytes;
	chunkBytes.Set(kwBytes.GetPtr(), kwLength);
	chunkBytes.Add(0);
	ByteArray dataBytes = content.data.ToBytes(TextEncoding::Iso8859_1());
	chunkBytes.Add(dataBytes.GetPtr(), dataBytes.GetSize());

	cf.BeginChunkWrite(PngChunk_tEXt::Name);
	cf.Write(chunkBytes.GetPtr(), chunkBytes.GetSize());
	if( !cf.EndChunkWrite() )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::WriteChunk_pHYs(ChunkedFile& cf, const PngChunk_pHYs& content)
{
	cf.BeginChunkWrite(PngChunk_pHYs::Name);
	cf.Write32(content.pixelsPerUnitX);
	cf.Write32(content.pixelsPerUnitY);
	cf.Write8(content.unit);
	if( !cf.EndChunkWrite() )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::GetIHDRFeaturesFromPixelFormat(PixelFormat epf, uint8& colorType, uint8& bitsPerComponent)
{
	switch(epf)
	{
	case PF_Unknown:
		return false;
		
	case PF_1bppGrayScale:      // 1bit per pixel
		colorType = PngChunk_IHDR::CTF_Grey;
		bitsPerComponent = 1;
		return true;

	case PF_2bppGrayScale:       // 2bits per pixel
		colorType = PngChunk_IHDR::CTF_Grey;
		bitsPerComponent = 2;
		return true;

	case PF_4bppGrayScale:       // 4bits per pixel
		colorType = PngChunk_IHDR::CTF_Grey;
		bitsPerComponent = 4;
		return true;

	case PF_8bppGrayScale:       // 8bits per pixel
		colorType = PngChunk_IHDR::CTF_Grey;
		bitsPerComponent = 8;
		return true;

	case PF_16bppGrayScale:      // 16bits per pixel
		colorType = PngChunk_IHDR::CTF_Grey;
		bitsPerComponent = 16;
		return true;

	case PF_16bppGrayScaleAlpha:  // GGAA
		colorType = PngChunk_IHDR::CTF_Alpha;
		bitsPerComponent = 8;
		return true;

	case PF_32bppGrayScaleAlpha: // GGGGAAAA
		colorType = PngChunk_IHDR::CTF_Alpha;
		bitsPerComponent = 16;
		return true;

	case PF_1bppIndexed:    // Indexed 1bit per pixel
		colorType = PngChunk_IHDR::CTF_Color | PngChunk_IHDR::CTF_Palette;
		bitsPerComponent = 1;
		return true;

	case PF_2bppIndexed:    // Indexed 2bits per pixel
		colorType = PngChunk_IHDR::CTF_Color | PngChunk_IHDR::CTF_Palette;
		bitsPerComponent = 2;
		return true;

	case PF_4bppIndexed:    // Indexed 4bits per pixel
		colorType = PngChunk_IHDR::CTF_Color | PngChunk_IHDR::CTF_Palette;
		bitsPerComponent = 4;
		return true;

	case PF_8bppIndexed:    // Indexed 8bits per pixel
		colorType = PngChunk_IHDR::CTF_Color | PngChunk_IHDR::CTF_Palette;
		bitsPerComponent = 8;
		return true;

	case PF_24bppRgb:        // RRGGBB
		colorType = PngChunk_IHDR::CTF_Color;
		bitsPerComponent = 8;
		return true;

	case PF_48bppRgb:       // RRRRGGGGBBBB 
		colorType = PngChunk_IHDR::CTF_Color;
		bitsPerComponent = 16;
		return true;

	case PF_32bppRgba:   // RRGGBBAA
		colorType = PngChunk_IHDR::CTF_Color | PngChunk_IHDR::CTF_Alpha;
		bitsPerComponent = 8;
		return true;

	case PF_32bppBgra:   // BBGGRRAA
		colorType = PngChunk_IHDR::CTF_Color | PngChunk_IHDR::CTF_Alpha;
		bitsPerComponent = 8;
		return true;

	case PF_64bppRgba:  // RRRRGGGGBBBBAAAA	
		colorType = PngChunk_IHDR::CTF_Color | PngChunk_IHDR::CTF_Alpha;
		bitsPerComponent = 16;
		return true;
		
	default:
		break;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::WriteFrameControlChunk(const ApngFrame* pFrame, int32 apngSequenceNumber, ChunkedFile& file)
{
	file.BeginChunkWrite(PngChunk_fcTL::Name);
	PngChunk_fcTL fctl = pFrame->m_fctl;

	fctl.sequenceNumber = apngSequenceNumber;
	if( file.ShouldSwapBytes() )
	{
		// Swap to big endian
		fctl.SwapBytes();
	}
	if( !file.Write(&fctl, PngChunk_fcTL::DataSize) )
	{
		return false;
	}
	if( !file.EndChunkWrite() )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PngDumper::CreateImageData(const uint8* pSrc, int32 width, int32 height, 
                          const PngDumpData& dd, const PngDumpSettings& ds, ByteArray& abImageData)
{
	PixelFormat epf = dd.pixelFormat;

	////////////////////////////////////////////////////////////
	ByteArray rbSwapped;
	if( epf == PF_32bppBgra )
	{
		const int32 pixelBytesPerRow = ImageFormat::ComputeByteWidth(dd.pixelFormat, width);
		const int32 srcSize = pixelBytesPerRow * height;

		if( !rbSwapped.SetSize(srcSize) )
		{
			// Not enough memory
			return false;
		}
		uint8* pNewBuffer = rbSwapped.GetPtr();

		// Should swap bytes as PNG does not support BGRA8
		// We convert the pixels into RGBA8 format
		const int32 pixelCount = srcSize / 4;
		for(int iCopy = 0; iCopy < pixelCount; ++iCopy)
		{
			uint8 b = pSrc[4 * iCopy + 0];
			uint8 g = pSrc[4 * iCopy + 1];
			uint8 r = pSrc[4 * iCopy + 2];
			uint8 a = pSrc[4 * iCopy + 3];
			
			pNewBuffer[4 * iCopy + 0] = r;
			pNewBuffer[4 * iCopy + 1] = g;
			pNewBuffer[4 * iCopy + 2] = b;
			pNewBuffer[4 * iCopy + 3] = a;
		}

		pSrc = pNewBuffer;
		epf = PF_32bppRgba;
	}
	////////////////////////////////////////////////////////////

	const int32 pixelBytesPerRow = ImageFormat::ComputeByteWidth(epf, width);
	const int32 srcSize = pixelBytesPerRow * height;

	/////////////////////////////////////
	const int32 bitsPerPixel = ImageFormat::SizeofPixelInBits(epf);

	DeflateStrategy strategy = DF_STRATEGY_DEFAULT;
	if( ds.zlibStrategy == PngDumpSettings::zlibStrategyGuess )
	{
		// Guess which strategy we should use
		if( ds.filtering != 0 )
		{
			// Usually achieves better compression with filtered images
			if( bitsPerPixel <= 8 )
			{
				// But for low depths only
				strategy = DF_STRATEGY_FILTERED;
			}
		}
	}
	else
	{
		if( ds.zlibStrategy == PngDumpSettings::zlibStrategyFilter )
		{
			strategy = DF_STRATEGY_FILTERED;
		}
	}

	// The buffer that will be given to the zlib routines
	ByteArray abBufferToCompress;

	const int32 bytesPerPixel = (bitsPerPixel + 7 ) / 8;

	// If special sub-filtering is not requested do not try other methods
	bool bDoFiltering = false;
	if( ds.filtering != 0 )
	{
		bDoFiltering = true;
	}
	
	int32 bufferToCompressSize = 0;
	uint8* pBufferToCompress = null;

	if( dd.interlaced )
	{
		bufferToCompressSize = Png::ComputeInterlacedSize(width, height, bitsPerPixel);
		if( !abBufferToCompress.SetSize(bufferToCompressSize) )
		{
			// Not enough memory
			return false;
		}

		pBufferToCompress = abBufferToCompress.GetPtr();
		if( !InterlaceAndFilter(pBufferToCompress, pSrc, width, height, pixelBytesPerRow, bitsPerPixel, bDoFiltering) )
		{
			// Not enough memory
			return false;
		}
	}
	else
	{
		/////////////////////////////////////////////////////////////
		// Create a new buffer from the image with a blank byte at each
		// row beginning (the sub-filtering method)

		bufferToCompressSize = srcSize + height;
		if( !abBufferToCompress.SetSize(bufferToCompressSize) )
		{
			// Not enough memory
			return false;
		}
		pBufferToCompress = abBufferToCompress.GetPtr();

		uint8* pCurNewByte = pBufferToCompress;
		const uint8* pCurSrcByte = pSrc;

		for(int iRow = 0; iRow < height; ++iRow)
		{
			pCurNewByte[0] = 0;
			pCurNewByte++;

			for(int iByte = 0; iByte < pixelBytesPerRow; ++iByte)
			{
				pCurNewByte[0] = pCurSrcByte[0];
				pCurSrcByte++;
				pCurNewByte++;
			}
		}

		////////////////////////////////////////////////////////////////
		// Apply filtering
		if( bDoFiltering )
		{
			FilterBlock(pBufferToCompress, height, pixelBytesPerRow, bytesPerPixel);
		}
	}

	// ZLib documentation about compress() :
	// Upon entry, destLen is the total size of the destination buffer,
	// which must be at least 0.1% larger than sourceLen plus 12 bytes.
	uint32 compressedBufferSize = 12 + bufferToCompressSize + ((bufferToCompressSize + 63) / 64);
	
	if( !abImageData.SetSize(compressedBufferSize) )
	{
		// Not enough memory
		return false;
	}
	uint8* const pCompressedBuffer = abImageData.GetPtr();

	const int32 compressionLevel = ds.zlibCompressionLevel;

	int maxWindowBits = 15;
	int memLevel = 8;

	// Sometimes it improves compression to use settings for low memory platforms,
	// that's why we have this option
	if( ds.zlibWindowBitsAndMem == PngDumpSettings::zlibWindowBitsAndMemLow )
	{
		maxWindowBits = 14;
		memLevel = 6;
	}

	int32 ret = Compress(pCompressedBuffer, &compressedBufferSize,
						pBufferToCompress, bufferToCompressSize,
						compressionLevel, strategy, maxWindowBits, memLevel);

	if( ret != 0 )
	{
		// Error in compression
		return false;
	}

	// Set the final size
	abImageData.SetSize(compressedBufferSize);

	return true;
}

// pSrc : pointer to a linear buffer of pixels
bool PngDumper::InterlaceAndFilter(uint8* pDst, const uint8* pSrc,
							  const int32 srcWidth, const int32 srcHeight,
							  const int32 srcPixelBytesPerRow,
							  int32 sizeofPixelInBits, bool bDoFiltering)
{
	static const int32 aStartingRow[7] =  { 0, 0, 4, 0, 2, 0, 1 };
	static const int32 aRowIncrement[7] = { 8, 8, 8, 4, 4, 2, 2 };

	static const int32 aStartingCol[7] =  { 0, 4, 0, 2, 0, 1, 0 };
	static const int32 aColIncrement[7] = { 8, 8, 4, 4, 2, 2, 1 };

	///////////////////////////////////////////////////////////////////////////
	// Masks arrays for pixel sizes < 8
	static const uint8 masks4[2] = { 0xf0, 0x0f };
	static const uint8 shifts4[2] = { 4, 0 };

	static const uint8 masks2[4] = { 0xc0, 0x30, 0x0c, 0x03 };
	static const uint8 shifts2[4] = { 6, 4, 2, 0 };

	static const uint8 masks1[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	static const uint8 shifts1[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
	
	const uint8* paMask = null;
	const uint8* paShift = null;
	int32 srcShift = 0;

	if( sizeofPixelInBits < 8 )
	{
		if( sizeofPixelInBits == 4 )
		{
			paMask = masks4;
			paShift = shifts4;
			srcShift = 1;
		}
		else if( sizeofPixelInBits == 2 )
		{
			paMask = masks2;
			paShift = shifts2;
			srcShift = 2;
		}
		else if( sizeofPixelInBits == 1 )
		{
			paMask = masks1;
			paShift = shifts1;
			srcShift = 3;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// The Png file is made of several sub-images, going more and more detailed

	const int32 sizeofPixelInBytes = (sizeofPixelInBits + 7 ) / 8;
	int32 dstIndex = 0; // The byte index in the destination buffer
	
	// Sub index for pixel < 8 bits
	int32 dstSubIndex = 0;
	int32 maxDstSubIndex = (8 / sizeofPixelInBits) - 1;

	for(int8 iPass = 0; iPass < 7; ++iPass)
	{
		//////////////////////////////////////////////////////////////////////////////
		uint8* const pDstPass = pDst + dstIndex;

		int32 localRowCount, localPixelBytesPerRow;
		Png::GetPassSize(iPass, localRowCount, localPixelBytesPerRow, srcWidth, srcHeight, sizeofPixelInBits);
	
		//////////////////////////////////////////////////////////////////////////////
		if( localRowCount == 0 || localPixelBytesPerRow == 0 )
		{
			// Empty pass
			continue;
		}

		if( sizeofPixelInBits < 8 )
		{
			// As for small bit depth we perform binary OR to set the pixels, the destination buffer
			// must be previously filled with 0s
			Memory::Zero(pDstPass, (localPixelBytesPerRow + 1) * localRowCount);
		}

		// Prepare the pass-pixel-block by getting pixels around from the original buffer
		int32 row = aStartingRow[iPass];
		while(row < srcHeight)
		{
			int32 col = aStartingCol[iPass];
			if( col >= srcWidth )
			{
				// End of row reached
				break;
			}

			// Insert an empty byte for the sub-filtering value
			// (We will filter the whole pass buffer later)
			pDst[dstIndex] = 0;
			dstIndex++;

			const int32 srcRowOffset = row * srcPixelBytesPerRow;
			do
			{
				if( sizeofPixelInBits > 8 )
				{
					const int32 srcOffset = srcRowOffset + col * sizeofPixelInBytes;
					for(int i = 0; i < sizeofPixelInBytes; ++i)
					{
						pDst[dstIndex] = pSrc[srcOffset + i];
						dstIndex++;
					}
				}
				else if( sizeofPixelInBits == 8 )
				{
					pDst[dstIndex] = pSrc[srcRowOffset + col];
					dstIndex++;
				}
				else if( sizeofPixelInBits < 8 )
				{
					////////////////////////////////////////////////////
					// Extract the pixel from the source
					int32 srcOffset = srcRowOffset + (col >> srcShift);
					uint8 byte = pSrc[srcOffset];
					int32 srcSubIndex = col & maxDstSubIndex;
					
					byte &= paMask[srcSubIndex];
					byte >>= paShift[srcSubIndex];
										
					// byte now contains the source pixel value
					////////////////////////////////////////////////////

					byte <<= paShift[dstSubIndex];

					pDst[dstIndex] |= byte;

					dstSubIndex++;

					if( dstSubIndex > maxDstSubIndex )
					{
						// End of byte
						dstIndex++;
						dstSubIndex = 0;
					}
				}
				
				col += aColIncrement[iPass];
			}
			while(col < srcWidth);

			if( dstSubIndex != 0 )
			{
				// Padding
				dstIndex++;
				dstSubIndex = 0; // Reset the sub pixel index
			}

			row += aRowIncrement[iPass];
		}
		
		// End of the pass !
		if( bDoFiltering )
		{
			if( !FilterBlock(pDstPass, localRowCount, localPixelBytesPerRow, sizeofPixelInBytes) )
			{
				// Not enough memory
				return false;
			}
		}
	}
	return true;
}

// pBlock points on a buffer which already has room for the sub-filtering byte info given
// at the beginning of each row.
bool PngDumper::FilterBlock(uint8* const pBlock, int32 rowCount, int32 pixelBytesPerRow, int32 bytesPerPixel)
{
	// Prepare the rows so the filter byte appears at the beginning of each row
	//for(int iRow = rowCount - 1; iRow >= 0; --iRow)
	{

	}
	// Add the byte size for the filtering type
	const int32 totalBytesPerRow = pixelBytesPerRow + 1;
	///////////////////////////////////////////////////////////////////////////

	//const int32 filteredBufSize = filteredRowSize * rowCount;

	// Allocate buffers for 5 possible filtered rows
	ByteArray rbFilteredRows;
	if( !rbFilteredRows.SetSize(pixelBytesPerRow * 5) )
	{
		// Not enough memory
		return false;
	}

	uint8* const pFilteredRows = rbFilteredRows.GetPtr();

	uint8* const pDstNone = pFilteredRows;
	uint8* const pDstSub = pDstNone + pixelBytesPerRow;
	uint8* const pDstUp = pDstSub + pixelBytesPerRow;
	uint8* const pDstAverage = pDstUp + pixelBytesPerRow;
	uint8* const pDstPaeth = pDstAverage + pixelBytesPerRow;

	uint8* const apTmpRows[5] = {pDstNone, pDstSub, pDstUp, pDstAverage, pDstPaeth };
	
	////////////////////////////////////////////////////////////////////////
	// The filtering process must be bottom-up
	// TODO : this kills the CPU cache we should sometimes prefetch some data to speed-up the process

	uint8* pRow = pBlock + totalBytesPerRow * rowCount - totalBytesPerRow;

	for(int iRow = rowCount - 1; iRow >= 0; --iRow)
	{
		uint8* pMethod = pRow;

		// Jump over the method byte
		pRow++;

		// Filtering method used
		uint8 method = 0;

		///////////////////////////////////////
		// Try all of the 5 sub-filter types //
		///////////////////////////////////////

		// Filtering = None (0)
		for(int iByte = 0; iByte < pixelBytesPerRow; ++iByte)
		{
			const uint8 byteCurrent = pRow[iByte];
			const uint8 none = byteCurrent;
			
			pDstNone[iByte] = none;
		}

		// Try other types of filtering

		// Filtering = Sub (1)
		for(int iByte = 0; iByte < pixelBytesPerRow; ++iByte)
		{
			uint8 byteLeft;
			if( iByte < bytesPerPixel )
			{
				byteLeft = 0;
			}
			else
			{
				byteLeft = pRow[iByte - bytesPerPixel];
			}

			const uint8 byteCurrent = pRow[iByte];
			const uint8 sub = uint8(byteCurrent - byteLeft);
			
			pDstSub[iByte] = sub;
		}

		// Filtering = Up (2)
		for(int iByte = 0; iByte < pixelBytesPerRow; ++iByte)
		{
			uint8 byteUp;
			if( iRow == 0 )
			{
				byteUp = 0;
			}
			else
			{
				byteUp = pRow[-totalBytesPerRow + iByte];
			}

			const uint8 byteCurrent = pRow[iByte];
			const uint8 nUp = uint8(byteCurrent - byteUp);
			
			pDstUp[iByte] = nUp;
		}

		// Filtering = Average (3)
		for(int iByte = 0; iByte < pixelBytesPerRow; ++iByte)
		{
			uint8 byteUp;
			if( iRow == 0 )
			{
				byteUp = 0;
			}
			else
			{
				byteUp = pRow[-totalBytesPerRow + iByte];
			}

			uint8 byteLeft;
			if( iByte < bytesPerPixel )
			{
				byteLeft = 0;
			}
			else
			{
				byteLeft = pRow[iByte - bytesPerPixel];
			}

			const uint8 byteCurrent = pRow[iByte];
			const uint8 average = uint8(byteCurrent - (byteLeft + byteUp) / 2);
			
			pDstAverage[iByte] = average;
		}

		// Filtering = Paeth (4)
		for(int iByte = 0; iByte < pixelBytesPerRow; ++iByte)
		{
			uint8 byteUp;
			if( iRow == 0 )
			{
				byteUp = 0;
			}
			else
			{
				byteUp = pRow[-totalBytesPerRow + iByte];
			}

			uint8 byteLeft;
			if( iByte < bytesPerPixel )
			{
				byteLeft = 0;
			}
			else
			{
				byteLeft = pRow[iByte - bytesPerPixel];
			}

			uint8 byteUpLeft;
			if( iByte < bytesPerPixel || iRow == 0 )
			{
				byteUpLeft = 0;
			}
			else
			{
				byteUpLeft = pRow[-totalBytesPerRow + iByte - bytesPerPixel];
			}

			const uint8 byteCurrent = pRow[iByte];
			const uint8 paeth = uint8(byteCurrent - Png::PaethPredictor(byteLeft, byteUp, byteUpLeft));
			
			pDstPaeth[iByte] = paeth;
		}

		//////////////////////////////////////////////////////////////////////////////////
		// Choose the best filtered row of the 5

		uint32 smallest = MAX_UINT32;
		method = 0; // Index of the method to use. If all have a sum of 0, choose the method 0

		for(uint8 i = 0; i < 5; ++i)
		{
			uint32 sum = 0;
			for(int iByte = 0; iByte < pixelBytesPerRow; ++iByte)
			{
				uint8 byte = apTmpRows[i][iByte];
				/*
				if( byte >= 128 )
				{
					byte = uint8(256 - byte);
				}*/
			
				sum += byte;
			}

			if( sum < smallest )
			{
				smallest = sum;
				method = i;
			}
		}
		method = 4; // dbg
		///////////////////////////////////////////////////////////////////////
		// Copy the wanted filtered row to the result buffer
		pMethod[0] = method;
		Memory::Copy(pRow, apTmpRows[method], pixelBytesPerRow);

		pRow -= (1 + totalBytesPerRow);
	}
	////////////////////////////////////////////////////////////////////////
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int PngDumper::Compress(uint8* pDest, uint32* pDestLen, const uint8* pSource, uint32 sourceLen, 
				  int level, DeflateStrategy strategy, int windowBits, int memLevel)
{
	ASSERT(strategy == DF_STRATEGY_DEFAULT || strategy == DF_STRATEGY_FILTERED);

	DeflateCompressor deflateCompressor;
	deflateCompressor.SetBuffers(pSource, sourceLen, pDest, *pDestLen);
	
	DeflateRet err;
	
	DeflateMethod method = DF_METHOD_DEFLATED;

	err = deflateCompressor.Init2(level, method, windowBits, memLevel, strategy);
	if( err != DF_RET_OK )
	{
		return err;
	}
	err = deflateCompressor.Compress(DF_FLUSH_FINISH);
	if( err != DF_RET_STREAM_END )
	{
		deflateCompressor.End();
		return err == DF_RET_OK ? DF_RET_BUF_ERROR : err;
	}
	*pDestLen = deflateCompressor.GetOutTotalRead();
	
	err = deflateCompressor.End();
	return err;
}
