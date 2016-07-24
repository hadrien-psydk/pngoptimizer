///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Png.h"
#include "File.h"
#include "Math.h"
#include "TextEncoding.h"
#include "FormatType.h"

namespace chustd {
const uint8 k_PngSignature[8] = {0x89,'P','N','G','\r','\n',0x1a,'\n'};
}

///////////////////////////////////////////////////////////////////////////////
using namespace chustd;

///////////////////////////////////////////////////////////////////////////////
void Png::ImageDataInfo::Clear()
{
	pPixels = null;
	width = 0;
	height = 0;
	byteWidth = 0;
	uncompressedDataSize = 0;
}

///////////////////////////////////////////////////////////////////////////////
Png::Png()
{
	Initialize();
}

///////////////////////////////////////////////////////////////////////////////
Png::~Png()
{

}

///////////////////////////////////////////////////////////////////////////////
// Initializes members
// The initialization is made outside the constructor so we can reset the
// instance in order to load several images with the same instance
void Png::Initialize()
{
	m_IHDR.Clear();

	m_width = 0;
	m_height = 0;
	m_lastError = 0;

	m_PLTE.m_count = 0;

	m_sizeofPixel = 0;
	m_sizeofPixelInBits = 0;

	m_idiCurrent.Clear();

	m_counter_IDAT = 0;
	m_counter_fdAT = 0;
	m_outputOffset = 0;

	m_gamma = 100000;
	m_tRNS.Clear();
	m_bkGD.Clear();
	m_pHYs.Clear();
	m_tEXts.Clear();

	m_acTL.frameCount = 0;
	m_acTL.loopCount = 0;

	m_apFrames.Clear();
	m_currentSequenceNumber = 0;

	m_expectedChunks = cf_IHDR;
	m_currentChunkType = 0;
	m_foundChunks = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Reads the beginning of a file to check if it starts with the PNG signature.
bool Png::IsPng(IFile& file)
{
	const int32 signSize = sizeof(k_PngSignature);
	uint8 aRead[signSize];
	
	if( file.Read(aRead, signSize) != signSize )
	{
		return false;
	}

	if( !chustd::Memory::Equals(k_PngSignature, aRead, signSize) )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Loads a PNG picture from an open file
// Returns false if the loading failed
bool Png::LoadFromFile(IFile& file)
{
	typedef bool(Png::*PFN_CHUNKHANDLER)(IFile&, int32);
	struct ChunkInfo
	{
		uint32 name;
		uint32 flag;
		PFN_CHUNKHANDLER pfnHandler;
		uint32 unwantedAfter; // The chunk is unwanted after those
	};

	static const ChunkInfo aChunkInfo[] =
	{
		{ PngChunk_IHDR::Name, cf_IHDR, &Png::Handle_IHDR, cf_IHDR },
		{ PngChunk_gAMA::Name, cf_gAMA, &Png::Handle_gAMA, cf_IHDR|cf_gAMA },
		{ PngChunk_PLTE::Name, cf_PLTE, &Png::Handle_PLTE, cf_IHDR|cf_gAMA|cf_PLTE },
		{ PngChunk_tRNS::Name, cf_tRNS, &Png::Handle_tRNS, cf_IHDR|cf_gAMA|cf_PLTE },
		{ PngChunk_pHYs::Name, cf_pHYs, &Png::Handle_pHYs, cf_IHDR|cf_pHYs },
		{ PngChunk_IDAT::Name, cf_IDAT, &Png::Handle_IDAT, cf_IHDR|cf_gAMA|cf_PLTE|cf_tRNS|cf_bKGD|cf_acTL|cf_pHYs },
		{ PngChunk_bKGD::Name, cf_bKGD, &Png::Handle_bKGD, cf_IHDR|cf_gAMA|cf_PLTE },
		{ PngChunk_tEXt::Name, cf_tEXt, &Png::Handle_tEXt, cf_IEND },
		{ PngChunk_acTL::Name, cf_acTL, &Png::Handle_acTL, cf_IHDR },
		{ PngChunk_fcTL::Name, cf_fcTL, &Png::Handle_fcTL, cf_IHDR|cf_acTL|cf_fcTL },
		{ PngChunk_fdAT::Name, cf_fdAT, &Png::Handle_fdAT, cf_IHDR|cf_acTL|cf_IDAT }
	};

	// Reset the object
	FreeBuffer();
	Initialize();


	//////////////////////////////////////////////////////
	// Read the header first
	if( !IsPng(file) )
	{
		m_lastError = errNotAPngFile;
		return false;
	}

	ChunkedFile cd(file);

	// Current position is the start of a chunk collection

	bool bOkHandled = true;
	bool bIENDFound = false;

	while( !bIENDFound )
	{
		bool bChunkPresent = cd.BeginChunkRead();
		if( !bChunkPresent )
		{
			// No more chunk
			break;
		}
		m_currentChunkType = cd.m_chunkName;
		if( cd.m_chunkName == PngChunk_IEND::Name )
		{
			bIENDFound = true;
			break;
		}

		const int handlerCount = sizeof(aChunkInfo) / sizeof(aChunkInfo[0]);
		for(int i = 0; i < handlerCount; ++i)
		{
			const ChunkInfo& chunkinfo = aChunkInfo[i];
			if( chunkinfo.name == cd.m_chunkName )
			{
				// Handler found
				if( (m_expectedChunks & uint32(chunkinfo.flag)) == 0 )
				{
					// Error, unexpected chunk
					m_lastError = errUnexpectedChunk;
					bOkHandled = false;
				}
				else
				{
					// Ok to decode
					bOkHandled = (this->*chunkinfo.pfnHandler)(file, cd.m_chunkSize);
					m_foundChunks |= chunkinfo.flag;
					m_expectedChunks = cf_ALL & ~(chunkinfo.unwantedAfter);
				}
				break;
			}
		}

		if( !bOkHandled )
		{
			// Error while reading a chunk content in one of the Handle_* function
			// m_lastError is correctly set by one of those functions
			break;
		}

		if( !cd.EndChunkRead() )
		{
			// Unexpected end of stream, CRC not present or chunk mishandled (read too much)
			m_lastError = uncompleteFile;
			bOkHandled = false;
			break;
		}
	}

	m_compressedBuffer.Clear();

	if( bOkHandled && !bIENDFound )
	{
		// End of stream with no damaged chunk but IEND not found
		m_lastError = errNoIENDFound;
		
		// We set bOkHandled to false so we perform the needed on-error cleanup
		bOkHandled = false;
	}
	
	if( !bOkHandled )
	{
		// An error occurred, we clean the object except the lasterror field
		m_deflateUncompressor.End();

		int32 lastError = m_lastError;
		FreeBuffer();
		m_lastError = lastError;
		return false;
	}

	if( !IsChunkFound(cf_IDAT) )
	{
		// No error yet, but no mandatory IDAT chunk found
		FreeBuffer();
		m_lastError = errIDATNotFound;
		return false;
	}

	bool bTerminateOk = EndImageDataProcessing();
	if( !bTerminateOk )
	{
		// An error occurred, we clean the object except the lasterror field
		// (TerminateJob fills m_lastError correctly)

		int32 lastError = m_lastError;
		FreeBuffer();
		m_lastError = lastError;
		return false;
	}

	const int32 frameCount = m_apFrames.GetSize();

	// Check default image
	if( m_pixels.IsEmpty() )
	{
		// APNG with no default image ?
		if( frameCount >= 0 )
		{
			// Yes, the IDAT is part of the animation. But to be valid, its assigned fcTL must have
			// values compatible with those in the IHDR.
			ApngFrame* pFrame = m_apFrames[0];
			bool bOk1 = (pFrame->m_fctl.width == m_IHDR.width);
			bool bOk2 = (pFrame->m_fctl.height == m_IHDR.height);
			bool bOk3 = (pFrame->m_fctl.offsetX == 0);
			bool bOk4 = (pFrame->m_fctl.offsetY == 0);

			if( !(bOk1 && bOk2 && bOk3 && bOk4) )
			{
				// Error
				FreeBuffer();
				m_lastError = errIDATfcTLNotConsistentWithIHDR;
				return false;
			}
		}
	}

	// Check acTL missing
	if( frameCount >= 1 )
	{
		if( !IsChunkFound(cf_acTL) )
		{
			m_lastError = erracTLMissing;
			return false;
		}
	}

	// Check frame count
	if( IsChunkFound(cf_acTL) )
	{
		if( m_acTL.frameCount != frameCount )
		{
			m_lastError = errFrameCountMismatch;
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Png::IsValidBitDepth(uint8 bitDepth)
{
	return bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8 || bitDepth == 16;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Png::IsValidColorType(uint8 colorType)
{
	return colorType == 0 || colorType == 2 || colorType == 3 || colorType == 4 || colorType == 6;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int Png::Read_IHDR(IFile& file, int32 dataSizeof, PngChunk_IHDR& out)
{
	if( dataSizeof != PngChunk_IHDR::DataSize )
	{
		return errBadHeaderSize;
	}

	ASSERT( sizeof(PngChunk_IHDR) >= PngChunk_IHDR::DataSize );

	if( file.Read(&out, dataSizeof) != int32(dataSizeof) )
	{
		return uncompleteFile;
	}
	
	if( file.ShouldSwapBytes() )
	{
		out.SwapBytes();
	}

	// Maximum size is 2**25, this lets us 7 bits to do computations on width and height
	const int32 maxWidth = 1 << 25;
	if(    !(0 <= out.width && out.width <= maxWidth)
		|| !(0 <= out.height && out.height <= maxWidth) )
	{
		// We limit the picture size to a sensible maximum size so we can detect
		// bad data in the file
		return errBadPicSize;
	}

	if( !IsValidBitDepth(out.bitDepth) )
	{
		return errBadBitDepth;
	}

	if( !IsValidColorType(out.colorType) )
	{
		return errBadColorType;
	}

	// Check combination
	if( GetPixelFormat(out.colorType, out.bitDepth) == PF_Unknown )
	{
		return errBadPixelFormat;
	}

	if( out.compressionMethod != 0 )
	{
		return errBadCompressionMethod;
	}

	if( out.filterMethod != 0 )
	{
		// The only filtering method supported is "Adaptative"
		// (none, sub, up, average, paeth)
		return errBadFilterMethod;
	}

	if( out.interlaceMethod > 1 )
	{
		return errBadInterlaceMethod;
	}
	return noErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Png::Handle_IHDR(IFile& file, int32 dataSizeof)
{
	int ret = Read_IHDR(file, dataSizeof, m_IHDR);
	if( ret != 0 )
	{
		m_lastError = ret;
		return false;
	}

	// Fill parent class attributes
	m_width = m_IHDR.width;
	m_height = m_IHDR.height;

	PixelFormat epf = GetPixelFormat();
	m_sizeofPixelInBits = ImageFormat::SizeofPixelInBits(epf);

	// Computes the size of a pixel in bytes
	m_sizeofPixel = (m_sizeofPixelInBits + 7 ) / 8;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Allocates the image buffer
// [in]  interlacedBuffer   true if the buffer will hold interlaced data
// Returns true if allocation succeeded
bool Png::AllocateImageBuffer(bool interlacedBuffer)
{
	// One extra byte is used in order to store the filtering sub-code
	const int32 rowByteCount = m_idiCurrent.byteWidth + 1;

	if( interlacedBuffer )
	{
		// The raw data size is bigger is interlacing is used
		m_idiCurrent.uncompressedDataSize = ComputeInterlacedSize(m_idiCurrent.width, m_idiCurrent.height, m_sizeofPixelInBits);
	}
	else
	{
		m_idiCurrent.uncompressedDataSize = rowByteCount * m_idiCurrent.height;
	}

	if( !m_idiCurrent.pPixels->SetSize(m_idiCurrent.uncompressedDataSize) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Gets gamma information
bool Png::Handle_gAMA(IFile& file, int32 dataSizeof)
{
	if( dataSizeof < 4 )
	{
		m_lastError = errNotEnoughDataInChunk;
		return false;
	}
	
	if( !file.Read32(m_gamma) )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool Png::Handle_pHYs(IFile& file, int32 dataSizeof)
{
	const int expectedSize = 4+4+1;
	if( dataSizeof < expectedSize )
	{
		m_lastError = errNotEnoughDataInChunk;
		return false;
	}
	if( file.Read(&m_pHYs, expectedSize) != expectedSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	if( file.ShouldSwapBytes() )
	{
		m_pHYs.pixelsPerUnitX = IFile::Swap32(m_pHYs.pixelsPerUnitX);
		m_pHYs.pixelsPerUnitY = IFile::Swap32(m_pHYs.pixelsPerUnitY);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Gets bKGD information
bool Png::Handle_bKGD(IFile& file, int32 dataSizeof)
{
	if( m_IHDR.colorType == 0x03 )
	{
		// Indexed color
		if( dataSizeof < 1 )
		{
			m_lastError = errNotEnoughDataInChunk;
			return false;
		}

		if( !file.Read8(m_bkGD.index) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		return true;
	}
	
	if( m_IHDR.colorType == 0x00 || m_IHDR.colorType == 0x04 )
	{
		// Grey or Grey+Alpha
		if( dataSizeof < 2 )
		{
			m_lastError = errNotEnoughDataInChunk;
			return false;
		}

		if( !file.Read16(m_bkGD.grey) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		return true;
	}

	if( m_IHDR.colorType == 0x02 || m_IHDR.colorType == 0x06 )
	{
		if( dataSizeof < 6 )
		{
			m_lastError = errNotEnoughDataInChunk;
			return false;
		}
		
		// TrueColor or TrueColor+Alpha
		if( !(file.Read16(m_bkGD.red) && file.Read16(m_bkGD.green) && file.Read16(m_bkGD.blue)) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		return true;
	}

	// Other type should not exist and be detected when handling the IHDR chunk
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Png::Handle_tEXt(IFile& file, int32 dataSizeof)
{
	PngChunk_tEXt content;
	ByteArray bytes;
	if( !bytes.SetSize(dataSizeof) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	if( file.Read(bytes.GetPtr(), dataSizeof) != dataSizeof )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	// Find the keyword/data separator
	int zeroAt = -1;
	foreach(bytes, i)
	{
		if( bytes[i] == 0 )
		{
			zeroAt = i;
			break;
		}
	}
	if( zeroAt < 0 )
	{
		m_lastError = errTextMissingSeparator;
		return false;
	}
	if( !(1 <= zeroAt && zeroAt <= 79) )
	{
		m_lastError = errTextBadKeywordLength;
		return false;
	}
	ByteArray kwBytes;
	kwBytes.Set(bytes.GetPtr(), zeroAt);
	ByteArray dataBytes;
	dataBytes.Set(bytes.GetPtr() + zeroAt + 1, bytes.GetSize() - zeroAt - 1);
	content.keyword = TextEncoding::Iso8859_1().BytesToString(kwBytes);
	content.data = TextEncoding::Iso8859_1().BytesToString(dataBytes);
	m_tEXts.Add(content);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Png::Handle_tRNS(IFile& file, int32 dataSizeof)
{
	if( m_IHDR.colorType == 3 )
	{
		// Indexed color image
		// Note : there can be less alpha values than palette entries
		// In this case, the remaining entries use 255 for their alpha value
		uint8 aAlphaValues[256];

		int32 colorCount = dataSizeof;
		if( colorCount > 256 )
		{
			colorCount = 256;
		}
		else if( colorCount < 0 )
		{
			colorCount = 0;
		}

		file.Read(aAlphaValues, colorCount);

		for(int i = 0; i < colorCount; ++i)
		{
			m_PLTE.m_colors[i].SetAlpha( aAlphaValues[i]);
		}
	}
	else if( m_IHDR.colorType == 0 )
	{
		// Greyscale
		if( !file.Read16(m_tRNS.grey) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
	}
	else if( m_IHDR.colorType == 2 )
	{
		// True color without alpha
		if( !(file.Read16(m_tRNS.red) && file.Read16(m_tRNS.green) && file.Read16(m_tRNS.blue)) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
	}
	return true;
}

// Gets palette information
bool Png::Handle_PLTE(IFile& file, int32 dataSizeof)
{
	int32 colorCount = dataSizeof / 3;
	if( colorCount > 256 )
	{
		colorCount = 256;
	}
	m_PLTE.m_count = colorCount;

	uint8 aTmpPalette[256 * 3];
	file.Read( aTmpPalette, colorCount * 3);

	int32 iTmpColor = 0;

	for(int i = 0; i < m_PLTE.m_count; ++i)
	{
		const uint8 r = aTmpPalette[iTmpColor + 0];
		const uint8 g = aTmpPalette[iTmpColor + 1];
		const uint8 b = aTmpPalette[iTmpColor + 2];
		iTmpColor += 3;

		m_PLTE[i].SetRgb(r, g, b);
	}
	return true;
}

bool Png::BeginImageDataProcessing()
{
	if( m_apFrames.GetSize() == 0 )
	{
		// The default image is not part of the animation, use the image pixel buffer
		m_idiCurrent.pPixels = &m_pixels;
		m_idiCurrent.width = m_IHDR.width;
		m_idiCurrent.height = m_IHDR.height;
	}
	else
	{
		// Uncompressing a frame
		ApngFrame* pFrame = m_apFrames.GetLast();
		
		m_idiCurrent.pPixels = &(pFrame->m_pixels);
		m_idiCurrent.width = pFrame->m_fctl.width;
		m_idiCurrent.height = pFrame->m_fctl.height;
	}

	// Compute the width of the image in bytes
	PixelFormat epf = GetPixelFormat();
	m_idiCurrent.byteWidth = ImageFormat::ComputeByteWidth(epf, m_idiCurrent.width);

	m_idiCurrent.uncompressedDataSize = 0; // Filled by AllocateImageBuffer

	// Allocate the image buffer
	if( !AllocateImageBuffer(m_IHDR.interlaceMethod == 1) )
	{
		return false;
	}

	// The first time we meet an IDAT chunk, we initialize the zstream structure
	m_deflateUncompressor.SetBuffers(null, 0, null, 0);
	
	const int nZErr = m_deflateUncompressor.Init();
	if( nZErr != DF_RET_OK )
	{
		m_lastError = errInflateErr;
		return false;
	}
	m_outputOffset = 0;
	return true;
}

// Gets image data information
bool Png::Handle_IDAT(IFile& file, int32 dataSizeof)
{
	m_counter_IDAT++;
	if( m_counter_IDAT == 1 )
	{
		// Special case for first IDAT
		if( !BeginImageDataProcessing() )
		{
			return false;
		}
	}

	return ProcessImageData(file, dataSizeof);
}

bool Png::ProcessImageData(IFile& file, int32 dataSizeof)
{
	if( dataSizeof == 0 )
	{
		// Valid, skip this data to avoid any decoding error
		return true;
	}

	const uint32 compressedSize = dataSizeof;
	if( !m_compressedBuffer.SetSize(compressedSize) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}

	uint32 read = file.Read(m_compressedBuffer.GetPtr(), compressedSize);
	if( read != compressedSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	
	const uint32 availableOut = m_idiCurrent.uncompressedDataSize - m_outputOffset;

	uint8* pIn  = m_compressedBuffer.GetPtr();
	uint8* pOut = m_idiCurrent.pPixels->GetWritePtr() + m_outputOffset;
	m_deflateUncompressor.SetBuffers(pIn, compressedSize, pOut, availableOut);
	
	int nZRet2 = m_deflateUncompressor.Uncompress(DF_FLUSH_SYNC);
	
	if( nZRet2 == DF_RET_OK )
	{
	
	}
	else if( nZRet2 == DF_RET_STREAM_END )
	{
	
	}
	else
	{
		m_lastError = errInflateErr;
		return false;
	}

	uint32 uncompressedSize = availableOut - m_deflateUncompressor.GetOutAvailable();
	m_outputOffset += uncompressedSize;

	return true;
}

bool Png::Handle_acTL(IFile& file, int32 dataSizeof)
{
	if( IsChunkFound(cf_acTL) )
	{
		m_lastError = erracTLRepeated;
		return false;
	}

	if( IsChunkFound(cf_IDAT) )
	{
		// acTL must appear before the first IDAT chunk within a valid PNG stream
		m_lastError = errIDATFoundBeforeacTL;
		return false;
	}
	
	if( dataSizeof < 8 )
	{
		m_lastError = errNotEnoughDataInChunk;
		return false;
	}
	
	if( !(file.Read32(m_acTL.frameCount) && file.Read32(m_acTL.loopCount)) )
	{
		m_lastError = uncompleteFile;
		return false;	
	}
	
	if( m_acTL.frameCount == 0 )
	{
		// Not valid
		m_lastError = errFrameCountIsZero;
		return false;
	}

	if( m_acTL.frameCount > MAX_INT32 )
	{
		m_lastError = errFrameCountOutsideRange;
		return false;
	}

	return true;
}

bool Png::Handle_fcTL(IFile& file, int32 dataSizeof)
{
	// Finish processing previous image data for a previous IDAT or previous fdAT.
	// Except if no IDAT or no fdAT was pending
	if( m_counter_IDAT > 0 || m_counter_fdAT > 0 )
	{
		if( !EndImageDataProcessing() )
		{
			return false;
		}
	}

	////////////////////////////////////////////////////////////
	const int wantedSizeof = PngChunk_fcTL::DataSize;
	if( dataSizeof < wantedSizeof )
	{
		m_lastError = errNotEnoughDataInChunk;
		return false;	
	}

	PngChunk_fcTL fctl;
	if( file.Read(&fctl, wantedSizeof) != wantedSizeof )
	{
		m_lastError = uncompleteFile;
		return false;	
	}

	if( file.ShouldSwapBytes() )
	{
		fctl.SwapBytes();
	}

	if( fctl.sequenceNumber != m_currentSequenceNumber )
	{
		m_lastError = errBadFrameControlSequenceNumber;
		return false;
	}

	if( fctl.delayFracDenominator == 0 )
	{
		// If the denominator is 0, it is to be treated as if it were 100
		fctl.delayFracDenominator = 100;
	}
	m_currentSequenceNumber++; // Incremented for fcTL and fdAT chunks

	ApngFrame* pFrame = new ApngFrame(this);
	if( pFrame == null )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	pFrame->m_fctl = fctl;
	if( m_apFrames.Add(pFrame) < 0 )
	{
		m_lastError = notEnoughMemory;
		return false;
	}

	return true;
}

bool Png::Handle_fdAT(IFile& file, int32 dataSizeof)
{
	if( m_counter_IDAT != 0 )
	{
		m_lastError = errMissingfcTLBeforefdAT;
		return false;
	}

	int32 sequenceNumber;
	if( !file.Read32(sequenceNumber) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	// Verify that the sequence number is consistent
	if( sequenceNumber != m_currentSequenceNumber )
	{
		m_lastError = errBadFrameDataSequenceNumber;
		return false;
	}
	m_currentSequenceNumber++; // Incremented for fcTL and fdAT chunks

	m_counter_fdAT++;
	if( m_counter_fdAT == 1 )
	{
		// Special case for first fdAT
		if( !BeginImageDataProcessing() )
		{
			return false;
		}
	}
	return ProcessImageData(file, dataSizeof - 4);
}

PixelFormat Png::GetPixelFormat(const PngChunk_IHDR& pngIHDR)
{
	return GetPixelFormat(pngIHDR.colorType, pngIHDR.bitDepth);
}

PixelFormat Png::GetPixelFormat(uint8 colorType, uint8 bitDepth)
{
	// To the bottom : 
	//   default: grey, flags: 0x1=palette used, 0x2=color used, 0x4=alpha used
	// To the left :
	//   Bit depth
	static const PixelFormat pixelFormats[7][6] =
	{
		{ PF_1bppGrayScale, PF_2bppGrayScale, PF_4bppGrayScale, PF_8bppGrayScale, PF_16bppGrayScale },
		{ PF_Unknown,       PF_Unknown,       PF_Unknown,       PF_Unknown,       PF_Unknown },
		{ PF_Unknown,       PF_Unknown,       PF_Unknown,       PF_24bppRgb,      PF_48bppRgb },
		{ PF_1bppIndexed,   PF_2bppIndexed,   PF_4bppIndexed,   PF_8bppIndexed,   PF_Unknown },
		{ PF_Unknown,       PF_Unknown,       PF_Unknown,       PF_16bppGrayScaleAlpha, PF_32bppGrayScaleAlpha },
		{ PF_Unknown,       PF_Unknown,       PF_Unknown,       PF_Unknown,       PF_Unknown },
		{ PF_Unknown,       PF_Unknown,       PF_Unknown,       PF_32bppRgba,     PF_64bppRgba  }
	};

	const int down = colorType;
	const int left = Math::Log2(bitDepth);
	if( (0 <= down && down <= 6) && (0 <= left && left <= 4) )
	{
		return pixelFormats[down][left];
	}
	return PF_Unknown;
}

PixelFormat Png::GetPixelFormat() const
{
	return GetPixelFormat(m_IHDR);
}

const Palette& Png::GetPalette() const
{
	return m_PLTE;
}

const Buffer& Png::GetPixels() const
{
	if( m_pixels.IsEmpty() )
	{
		// APNG with no default image ?
		const int32 frameCount = m_apFrames.GetSize();
		if( frameCount >= 0 )
		{
			// Yes, return the first frame, whose size and offset 
			// must be compatible with what is stored in the IHDR
			return m_apFrames[0]->GetPixels();
		}
	}

	return m_pixels;
}

// Ends uncompression and unfilters
bool Png::EndImageDataProcessing()
{
	int nZRet = m_deflateUncompressor.End();
	if( nZRet != DF_RET_OK )
	{
		m_lastError = errInflateErr;
		return false;
	}
	// Reset image data chunk counters
	m_counter_IDAT = 0;
	m_counter_fdAT = 0;

	// Check that we output an expected byte count
	if( m_outputOffset < uint32(m_idiCurrent.uncompressedDataSize) )
	{
		m_lastError = errImageDataMissing;
		return false;
	}

	bool processingOk = false;
	if( m_IHDR.interlaceMethod == 1 )
	{
		processingOk = UnfilterAndUninterlace();
	}
	else
	{
		// No interlacing, just unfilter
		processingOk = UnfilterBlock(m_idiCurrent.pPixels->GetWritePtr(), 
		                             m_idiCurrent.height, m_idiCurrent.byteWidth, m_sizeofPixel);
		if( !processingOk )
		{
			m_lastError = errBadFilterType;  // Unknown filtering type
		}
	}
	if( processingOk )
	{
		// Compute final size of the pixel buffer
		int finalSize = m_idiCurrent.height * m_idiCurrent.byteWidth;
		m_idiCurrent.pPixels->SetSize(finalSize);
	}
	return processingOk;
}

// One of the Adaptative unfiltering method
// a = left, b = top, c = top-left
uint8 Png::PaethPredictor(uint8 a, uint8 b, uint8 c)
{
	int pa = b - c;
	int pb = a - c;
	int pc = pa + pb;
	
	if( pa < 0 ) { pa = -pa; }
	if( pb < 0 ) { pb = -pb; }
	if( pc < 0 ) { pc = -pc; }

	if( pa <= pb && pa <= pc )
	{
		return a;
	}

	if( pb <= pc )
	{
		return b;
	}
	return c;
}

bool Png::UnfilterAndUninterlace()
{
	// Keep the previous data to be used as input in the unfiltering process
	Buffer oldBuffer = *m_idiCurrent.pPixels;
	m_idiCurrent.pPixels->SetSize(0);
	
	// *pPixels is now empty, we reallocate it as it will be used as output in the unfiltering process
	// m_nUncompressedDataSize is modified too
	if( !AllocateImageBuffer(false) )
	{
		return false;
	}

	uint8* const pInterlaced = oldBuffer.GetWritePtr();
	const int32  interlacedSize = oldBuffer.GetSize();
	(void)interlacedSize; // Used in debug mode

	uint8* const pResult = m_idiCurrent.pPixels->GetWritePtr();
	const int32  resultSize = m_idiCurrent.uncompressedDataSize;

	static const int32 aStartingRow[7] = { 0, 0, 4, 0, 2, 0, 1 };
	static const int32 aStartingCol[7] = { 0, 4, 0, 2, 0, 1, 0 };
	static const int32 aRowIncrement[7] = { 8, 8, 8, 4, 4, 2, 2 };
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
	int32 dstShift = 0;

	if( m_sizeofPixelInBits < 8 )
	{
		Memory::Zero(pResult, resultSize);
		if( m_sizeofPixelInBits == 4 )
		{
			paMask = masks4;
			paShift = shifts4;
			dstShift = 1;
		}
		else if( m_sizeofPixelInBits == 2 )
		{
			paMask = masks2;
			paShift = shifts2;
			dstShift = 2;
		}
		else if( m_sizeofPixelInBits == 1 )
		{
			paMask = masks1;
			paShift = shifts1;
			dstShift = 3;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	const int32 width = m_idiCurrent.width;
	const int32 height = m_idiCurrent.height;
	const int32 byteWidth = m_idiCurrent.byteWidth;

	// Current byte position in the interlaced buffer
	int32 srcIndex = 0;
	
	// Sub index for pixel < 8 bits
	int32 srcSubIndex = 0;
	int32 maxSrcSubIndex = (8 / m_sizeofPixelInBits) - 1;
	
	for(int8 iPass = 0; iPass < 7; ++iPass)
	{
		//////////////////////////////////////////////////////////////////////////////
		// Unfilter the pass-pixel-block
		int32 localRowCount, pixelBytesPerRow;
		GetPassSize(iPass, localRowCount, pixelBytesPerRow, width, height, m_sizeofPixelInBits);

		if( !UnfilterBlock(pInterlaced + srcIndex, localRowCount, pixelBytesPerRow, m_sizeofPixel) )
		{
			m_lastError = errBadFilterType;  // Unknown filtering type
			return false;
		}
		//////////////////////////////////////////////////////////////////////////////
		if( localRowCount == 0 || pixelBytesPerRow == 0 )
		{
			// Empty pass
			continue;
		}

		// Unfilter the pass-pixel-block
		int32 row = aStartingRow[iPass];
		while(row < height)
		{
			int32 col = aStartingCol[iPass];
			if( col >= width )
			{
				// End of row reached
				break;
			}

			// Some data is available for this pass,
			const int32 dstRowOffset = row * byteWidth;
			do
			{
				ASSERT(srcIndex < interlacedSize);
				if( m_sizeofPixelInBits > 8 )
				{
					const int32 dstOffset = dstRowOffset + col * m_sizeofPixel;
					for(int i = 0; i < m_sizeofPixel; ++i)
					{
						pResult[dstOffset + i] = pInterlaced[srcIndex];
						srcIndex++;
					}
				}
				else if( m_sizeofPixelInBits == 8 )
				{
					pResult[dstRowOffset + col] = pInterlaced[srcIndex];
					srcIndex++;
				}
				else if( m_sizeofPixelInBits < 8 )
				{
					////////////////////////////////////
					// Extract the index value
					uint8 byte = pInterlaced[srcIndex];
					byte &= paMask[srcSubIndex];
					byte >>= paShift[srcSubIndex];
					////////////////////////////////////
					
					////////////////////////////////////
					int32 dstOffset = dstRowOffset + (col >> dstShift);
					int32 dstSubIndex = col & maxSrcSubIndex;
					byte <<= paShift[dstSubIndex];

					pResult[dstOffset] |= byte;

					srcSubIndex++;

					if( srcSubIndex > maxSrcSubIndex )
					{
						// End of byte
						srcIndex++;
						srcSubIndex = 0;
					}
				}
				
				col += aColIncrement[iPass];
			}
			while(col < width);

			if( srcSubIndex != 0 )
			{
				// Padding
				srcIndex++;
				srcSubIndex = 0; // Reset the sub pixel index
			}

			row += aRowIncrement[iPass];
		}
		// Jump to the next pass bytes (only if pass not empy)
		// (localRowCount == number of filtering bytes for this pass)
		// so we jump over the bytes which gave the sub-filtering info
		srcIndex += localRowCount;
	}

	return true;
}

int32 Png::ComputeInterlacedSize(int32 width, int32 height, int32 sizeofPixelInBits)
{
	int32 totalByteCount = 0;

	for(int8 iPass = 0; iPass < 7; ++iPass)
	{
		int32 rowCount, pixelBytesPerRow;
		GetPassSize(iPass, rowCount, pixelBytesPerRow, width, height, sizeofPixelInBits);

		if( pixelBytesPerRow > 0 )
		{
			// + 1 for each row to count the filtering byte
			totalByteCount += rowCount * (pixelBytesPerRow + 1);
		}
	}
	return totalByteCount;
}

// Gets the size of a block for a given pass
void Png::GetPassSize(int8 iPass, int32& rowCount, int32& pixelBytesPerRow,
                      int32 width, int32 height, int32 sizeofPixelInBits)
{
	ASSERT(0 <= iPass && iPass <= 6);

	static const int32 anAdders[8] =   { 7, 7, 3, 3, 1, 1, 0, 0 };
	static const int32 anDividers[8] = { 8, 8, 8, 4, 4, 2, 2, 1 };

	int32 pixelsPerRow = (width + anAdders[iPass + 1]) / anDividers[iPass + 1];
	int32 pixelBitsPerRow = pixelsPerRow * sizeofPixelInBits;
	
	pixelBytesPerRow = pixelBitsPerRow / 8 + ((pixelBitsPerRow % 8) > 0 ? 1 : 0);

	rowCount = (height + anAdders[iPass]) / anDividers[iPass];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// pBlock : intput/ouput
//    input format : each row starts with a filter type byte
//    Fxxxxxx
//    Fxxxxxx
//    Fxxxxxx
//    output format : linear version of the picture, without filter type byte
//    yyyyyy
//    yyyyyy
//    yyyyyy
//    xxx
// pixelBytesPerRow : number of bytes for one row of the picture, NOT counting the filter type byte
// bytesPerPixel : rounded to 1 if the pixel depth is less than 8
bool Png::UnfilterBlock(uint8* pBlock, int32 rowCount, int32 pixelBytesPerRow, int32 bytesPerPixel)
{
	if( pixelBytesPerRow == 0 )
		return true;
	// DBG
	// This is to output a loaded PNG to check how filtering is done
	/*
	static bool firstOcc = true;
	File file;
	File fileBin;
	uint32 mode = File::modeAppend;
	if( firstOcc )
	{
		mode = File::modeWrite;
		firstOcc = false;
		fileBin.Open("/Gfx/idat.bin", mode);
		fileBin.Write(pBlock, rowCount * (1+pixelBytesPerRow));
		fileBin.Close();
	}
	file.Open("/Gfx/png.txt", mode);
	if( file.GetPosition() == 0 )
	{
		file.Write16(uint16(0xFFFE));
	}
	for(int j = 0; j < rowCount; ++j)
	{
		wchar line[1000];
		wchar* tmpLine = line;
		for(int i = 0; i < pixelBytesPerRow; ++i)
		{
			int byteIndex = (1+pixelBytesPerRow)*j+i;
			FormatUInt32Hex(pBlock[byteIndex], tmpLine, 2, '0', false);
			tmpLine += 2;
			tmpLine[0] = ' ';
			tmpLine++;
		}
		tmpLine[0] = '\r';
		tmpLine[1] = '\n';
		tmpLine += 2;
		tmpLine[2] = 0;
		file.Write(line, (tmpLine-line)*2);
		
	}
	file.Close();
	*/

	// pRow points on the start of the filtered row
	// pUnfilteredRow points on the start of the unfiltered row
	// Then, for each row, pUnfilteredRow loses one byte in comparison with pRow

	uint8* pRow = pBlock;
	uint8* pUnfilteredRow = pBlock;

	// Create constant data for the loop
	const int32 sizeofPixel = bytesPerPixel;
	const int32 byteWidth = pixelBytesPerRow;

	for(int iRow = 0; iRow < rowCount; ++iRow)
	{
		// Initialize the index (iByte) of the current byte in the row
		int32 iByte = 0;
		
		// Get the filtering method used for this row
		const uint8 method = pRow[0];
				
		// Jump over the filtering method byte
		pRow++;
		
		switch(method)
		{
		case 0:
			// Method "None"
			{
				int counter = byteWidth;
				do
				{
					const uint8 pixel = pRow[0];
					pRow++;
					
					pUnfilteredRow[0] = pixel;
					pUnfilteredRow++;
					
				}
				while(--counter > 0);
			}
			break;
		case 1:
			// Method "Sub"
			for(;iByte < sizeofPixel; iByte++)
			{
				pUnfilteredRow[0] = pRow[0];
				pUnfilteredRow++;
				pRow++;
			}
			for(;iByte < byteWidth; iByte++)
			{
				uint8 sub = pRow[0];
				uint8 left = pUnfilteredRow[0 - sizeofPixel];
				uint8 res = uint8(sub + left);
				pUnfilteredRow[0] = res;
				pUnfilteredRow++;
				pRow++;
			}
			break;
		case 2:
			// Method "Up"
			if( iRow == 0 )
			{
				for(;iByte < byteWidth; iByte++)
				{
					uint8 nUp = pRow[0];
					uint8 above = 0;
					uint8 res = uint8(nUp + above);
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
			}
			else
			{
				for(;iByte < byteWidth; iByte++)
				{
					uint8 nUp = pRow[0];
					uint8 above = pUnfilteredRow[0 - byteWidth];
					uint8 res = uint8(nUp + above);
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
			}
			break;
		case 3:
			// Method "Average"
			if( iRow == 0 )
			{
				for(; iByte < sizeofPixel; iByte++)
				{
					uint8 average = pRow[0];
					uint8 left = 0;
					uint8 above = 0;
					uint8 res = uint8(average + (left + above) / 2);
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
				for(;iByte < byteWidth; iByte++)
				{
					uint8 average = pRow[0];
					uint8 left = pUnfilteredRow[0 - sizeofPixel];
					uint8 above = 0;
					uint8 res = uint8(average + (left + above) / 2);
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
			}
			else
			{
				for(; iByte < sizeofPixel; iByte++)
				{
					uint8 average = pRow[0];
					uint8 left = 0;
					uint8 above = pUnfilteredRow[0 - byteWidth];
					uint8 res = uint8(average + (left + above) / 2);
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
				for(;iByte < byteWidth; iByte++)
				{
					uint8 average = pRow[0];
					uint8 left = pUnfilteredRow[0 - sizeofPixel];
					uint8 above = pUnfilteredRow[0 - byteWidth];
					uint8 res = uint8(average + (left + above) / 2);
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
			}
			break;
		case 4:
			// Method "Paeth"
			if( iRow == 0 )
			{
				for(;iByte < sizeofPixel; iByte++)
				{
					uint8 paeth = pRow[0];
					uint8 left = 0;
					uint8 above = 0;
					uint8 upperLeft = 0;
					uint8 res = uint8(paeth + PaethPredictor(left, above, upperLeft));
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
				for(;iByte < byteWidth; iByte++)
				{
					uint8 paeth = pRow[0];
					uint8 left = pUnfilteredRow[0 - sizeofPixel];
					uint8 above = 0;
					uint8 upperLeft = 0;
					uint8 res = uint8(paeth + PaethPredictor(left, above, upperLeft));
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
			}
			else
			{
				for(;iByte < sizeofPixel; iByte++)
				{
					uint8 paeth = pRow[0];
					uint8 left = 0;
					uint8 above = pUnfilteredRow[0 - byteWidth];
					uint8 upperLeft = 0;
					uint8 res = uint8(paeth + PaethPredictor(left, above, upperLeft));
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
				for(;iByte < byteWidth; iByte++)
				{
					uint8 paeth = pRow[0];
					uint8 left = pUnfilteredRow[0 - sizeofPixel];
					uint8 above = pUnfilteredRow[0 - byteWidth];
					uint8 upperLeft = pUnfilteredRow[0 - sizeofPixel - byteWidth];
					uint8 res = uint8(paeth + PaethPredictor(left, above, upperLeft));
					pUnfilteredRow[0] = res;
					pUnfilteredRow++;
					pRow++;
				}
			}
			break;
		default:
			return false;
		}
	}
	return true;
}

String Png::StringFromChunkType(uint32 type)
{
	wchar szChunk[4];
	szChunk[0] = (type >> 24) & 0x00ff;
	szChunk[1] = (type >> 16) & 0x00ff;
	szChunk[2] = (type >> 8) & 0x00ff;
	szChunk[3] = (type) & 0x00ff;
	return String(szChunk, 4);
}

String Png::GetLastErrorString() const
{
	switch(m_lastError)
	{
	case Png::errNotAPngFile:
		return "Not a PNG file";

	case Png::errBadHeaderSize:
		return "Invalid header size";
		
	case Png::errBadBitDepth:
		return "Invalid bit depth";

	case Png::errBadColorType:
		return "Invalid or unsupported color format";

	case Png::errBadPixelFormat:
		return "Invalid pixel format";

	case Png::errBadCompressionMethod:
		return "Invalid or unsupported compression method";

	case Png::errBadFilterMethod:
		return "Invalid or unsupported filtering method";
		
	case Png::errBadFilterType:
		return "Bad filtering type";

	case Png::errInflateErr:
		return "Decompression error (zlib : " + String::FromAsciiSZ(m_deflateUncompressor.GetLastError()) + ")";

	case Png::errBadPicSize:
		return "Invalid image size";
	
	case Png::errNoIENDFound:
		return "No IEND chunk found at the end of the file data (file corrupted)";

	case Png::errBadInterlaceMethod:
		return "Unsupported interlacing method (Adam7 only)";

	case Png::errNotEnoughDataInChunk:
		return "Not enough data in chunk " + StringFromChunkType(m_currentChunkType);

	case Png::errIDATFoundBeforeacTL:
		return "IDAT found before acTL chunk";

	case Png::errFrameCountIsZero:
		return "Frame count in acTL chunk is zero";

	case Png::errBadFrameControlSequenceNumber:
		return "Bad sequence number in fcTL chunk";

	case Png::errBadFrameDataSequenceNumber:
		return "Bad sequence number in fdAT chunk";

	case Png::errIDATNotFound:
		return "No IDAT chunk found";

	case Png::errIDATfcTLNotConsistentWithIHDR:
		return "IDAT's fcTL mismatch with IHDR";

	case Png::erracTLMissing:
		return "acTL chunk missing";

	case Png::erracTLRepeated:
		return "acTL chunk repeated";
	
	case Png::errMissingfcTLBeforefdAT:
		return "fcTL chunk missing before fdAT";

	case Png::errUnexpectedChunk:
		return "Unexpected chunk: " + StringFromChunkType(m_currentChunkType);
		
	case Png::errFrameCountMismatch:
		return "Frame count mismatch with acTL";
	case Png::errFrameCountOutsideRange:
		return "Frame count outside range";
	case Png::errImageDataMissing:
		return "Image data missing";
	case Png::errTextMissingSeparator:
		return "Keyword/data separator missing in tEXt";
	case Png::errTextBadKeywordLength:
		return "Bad tEXt keyword length";
	}

	return ImageFormat::GetLastErrorString();
}

bool Png::IsAnimated() const
{
	// In GIF, everything is a frame, not with PNG, as there is the idea of default image
	if( HasDefaultImage() )
	{
		if( m_apFrames.GetSize() == 0 )
		{
			// No frame at all -> not animated
			return false;
		}
		// One default image + one frame at least
		return true;
	}

	// No default image
	if( m_apFrames.GetSize() == 1 )
	{
		// One single frame -> not animated
		return false;
	}
	// No default image + two frames at least
	return true;
}

bool Png::HasDefaultImage() const
{
	return !m_pixels.IsEmpty();
}

int32 Png::GetFrameCount() const
{
	return m_apFrames.GetSize();
}

const AnimFrame* Png::GetAnimFrame(int index) const
{
	int32 frameCount = m_apFrames.GetSize();
	if( !(0 <= index && index < frameCount) )
	{
		return null;
	}
	return m_apFrames[index];
}

int32 Png::GetLoopCount() const
{
	return m_acTL.loopCount;
}
