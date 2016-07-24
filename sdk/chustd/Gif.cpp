///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Gif.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;

//////////////////////////////////////////////////////////////////////
GifAnimFrame::GifAnimFrame(Gif* pGif) : AnimFrame(pGif)
{
	m_width = 0;
	m_height = 0;
	m_offsetX = 0;
	m_offsetY = 0;
	m_delayCs = 0;
	m_disposal = 0;

	m_clearIndex = 0;

	m_pPalette = null;
	m_interlaced = false;
	m_transparentIndex = -1;
}

GifAnimFrame::~GifAnimFrame()
{
	delete m_pPalette;
}

const Palette& GifAnimFrame::GetPalette() const
{
	if( m_pPalette == null )
	{
		// Use global palette
		return m_pOwner->GetPalette();
	}
	// Use local palette
	return *m_pPalette;
}

//////////////////////////////////////////////////////////////////////
static const uint8 k_GifSignature[3] = {'G', 'I', 'F'};
//////////////////////////////////////////////////////////////////////

Gif::Gif()
{
	Initialize();
}

Gif::~Gif()
{

}

void Gif::Initialize()
{
	m_width = 0;
	m_height = 0;
	m_lastError = 0;

	m_backgroundColorIndex = 0;
	m_transparencyUsed = false;
	m_transparentColorIndex = -1;
	m_disposalMethod = 0;
	m_delayTime = 0;
	m_version = gifverNotSet;
	
	// For GIF the default is one iteration, in case
	// the NETSCAPE block is not found
	m_loopCount = 1;

	m_globalColorTable.m_count = 0;
	
	m_apFrames.SetSize(0);
}

PixelFormat Gif::GetPixelFormat() const
{
	// Even for depths < 8, each pixel takes one byte in memory
	return PF_8bppIndexed;
}

const Palette& Gif::GetPalette() const
{
	const int32 frameCount = m_apFrames.GetSize();
	
	if( frameCount > 0 )
	{
		const GifAnimFrame* pFrame = m_apFrames[0];
		if( pFrame->m_pPalette != null )
		{
			return *(pFrame->m_pPalette);
		}
	}

	return m_globalColorTable;
}

bool Gif::IsInterlaced() const
{ 
	const int32 frameCount = m_apFrames.GetSize();
	
	if( frameCount > 0 )
	{
		const GifAnimFrame* pFrame= m_apFrames[0];
		return pFrame->m_interlaced;
	}

	// Warn the user
	ASSERT(0);
	return false;
}

int32 Gif::GetFrameCount() const
{
	return m_apFrames.GetSize();
}

const AnimFrame* Gif::GetAnimFrame(int32 frameIndex) const
{
	const int32 frameCount = m_apFrames.GetSize();
	if( !(0 <= frameIndex && frameIndex < frameCount) )
	{
		ASSERT(0); // Bad index
		return null;
	}

	if( !(0 <= frameIndex && frameIndex < frameCount) )
	{
		ASSERT(0); // Bad index
		return null;
	}
	
	const GifAnimFrame* pFrame = m_apFrames[frameIndex];
	return pFrame;
}

bool Gif::IsAnimated() const
{
	// In GIF, everything is a frame, so a GIF is animated if frame count >= 2
	return m_apFrames.GetSize() >= 2;
}

AnimFrame::Disposal Gif::GenDisposalFromGifDisposal(uint8 gifDisposal)
{
	switch( gifDisposal )
	{
	case gdmNotSpecified:
		return AnimFrame::DispNone;
	case gdmDoNotDispose:
		return AnimFrame::DispNone;
	case gdmRestoreToBkColor:
		return AnimFrame::DispClearToTransBlack;
	case gdmRestoreToPrevious:
		return AnimFrame::DispRestoreToPrevious;
	default:
		break;
	}
	return AnimFrame::DispNone;
}

int32 Gif::GetLoopCount() const
{
	return m_loopCount;
}

bool Gif::ReadTerminator(IFile& file)
{
	uint8 blockTerminator = 0xff;
	if( !file.Read8(blockTerminator) )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	if( blockTerminator != 0 )
	{
		m_lastError = errBlockTerminatorNotFound;
		return false;
	}
	return true;
}

bool Gif::IsGif(IFile& file)
{
	const int32 signSize = sizeof(k_GifSignature);
	uint8 aRead[signSize];

	if( file.Read(aRead, signSize) != signSize )
	{
		return false;
	}

	if( !chustd::Memory::Equals(k_GifSignature, aRead, signSize) )
	{
		return false;
	}
	return true;
}

bool Gif::LoadFromFile(IFile& file)
{
	/////////////////////////////////////////////////////
	// Reset the object
	FreeBuffer();
	Initialize();

	/////////////////////////////////////////////////////
	// Read the header first
	if( !IsGif(file) )
	{
		m_lastError = errNotAGifFile;
		return false;
	}

	/////////////////////////////////////////////////////
	// Read version (ex: "87a")
	uint8 aVersion[3];
	file.Read(aVersion, 3);

	if( Memory::Equals(aVersion, "87a", 3) )
	{
		m_version = gifver87a;
	}
	else if( Memory::Equals(aVersion, "89a", 3) )
	{
		m_version = gifver89a;
	}
	else
	{
		m_lastError = errBadGifVersion;
		return false;
	}
	/////////////////////////////////////////////////////

	/////////////////////////////////////////////////////
	// Gif works in little endian mode
	TmpEndianMode tem(file, boLittleEndian);

	// Read common header to all gif files
	if( !ReadLogicalScreenDescriptor(file) )
	{
		return false;
	}
	
	////////////////////////////////////////////////
	// The remaining data is splitted into blocks
	
	// Number of frames (> 1 means animated gif)
	bool bReadBlocks = true;
	bool bBlockOk = false;
	do
	{
		uint8 blockType;
		if( !file.Read8(blockType) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		
		if( blockType == 0x2c )
		{
			// 0x2c = ',' = ImageDescriptor
			
			GifAnimFrame* pFrame = new GifAnimFrame(this);
			if( pFrame == null )
			{
				// Allocation failure
				m_lastError = notEnoughMemory;
				return false;
			}

			int32 addOk = m_apFrames.Add(pFrame);
			if( addOk < 0 )
			{
				// Allocation failure
				m_lastError = notEnoughMemory;
				return false;
			}

			if( !ReadImageDescriptor(file, pFrame) )
			{
				return false;
			}
			
			// Read pixels
			bBlockOk = ReadImageData(file, pFrame);

			// Modify palette according to a retrieved transparent index
			if( m_transparentColorIndex >= 0 )
			{
				// Set global flag, used when clearing the screen
				m_transparencyUsed = true;

				pFrame->m_transparentIndex = m_transparentColorIndex;

				if( pFrame->m_pPalette != null )
				{
					pFrame->m_pPalette->m_colors[ m_transparentColorIndex ].SetAlpha(0);
				}
				else
				{
					// The frame may use the global palette, but as the transparent index may change from frame to frame,
					// we may have to create a new local palette for this frame, based on the global palette.
					if( m_apFrames.GetSize() == 1 )
					{
						// Set the alpha of the global palette, once for ever
						m_globalColorTable.m_colors[ m_transparentColorIndex ].SetAlpha(0);
					}
					else
					{
						// Check if we should reuse the global palette or make our own
						if( m_globalColorTable.m_colors[ m_transparentColorIndex ].GetAlpha() == 0x00 )
						{
							// Ok, reuse the global palette
						}
						else
						{
							// Transparent index not the same than in global palette, make our own palette
							Palette* pNewPalette = new Palette;
							if( pNewPalette == null )
							{
								m_lastError = notEnoughMemory;
								return false;
							}
							const int colorCount = m_globalColorTable.m_count;
							pNewPalette->m_count = colorCount;
							for(int i = 0; i < colorCount; ++i)
							{
								pNewPalette->m_colors[i] = m_globalColorTable.m_colors[i];
							}
							pNewPalette->SetAlphaFullOpaque();
							pNewPalette->m_colors[ m_transparentColorIndex ].SetAlpha(0);
							
							pFrame->m_pPalette = pNewPalette;
						}
					}
				}
				m_transparentColorIndex = -1; // We are done with this value
			}

			pFrame->m_disposal = GenDisposalFromGifDisposal(m_disposalMethod);
			m_disposalMethod = 0;

			// Set delay time
			pFrame->m_delayCs = m_delayTime; // Gif is in 100th of second
		}
		else if( blockType == 0x21 )
		{
			// 0x21 = '!'
			bBlockOk = ReadExtensionBlock(file);
		}
		else if( blockType == 0x3b )
		{
			// End of image 0x3b = ';'
			bReadBlocks = false;
		}
		else
		{
			// Error, unknown block
			bReadBlocks = false;
		}
	}while( bReadBlocks && bBlockOk);

	if( !bBlockOk )
	{
		return false;
	}

	////////////////////////////////////////////////
	// Uninterlace frames pixel buffers
	foreach(m_apFrames, i)
	{
		if( m_apFrames[i]->m_interlaced )
		{
			if( !UninterlaceBuffer(m_apFrames[i]) )
			{
				return false;
			}
		}
	}

	////////////////////////////////////////////////
	// Prepare the main pixel buffer : background color and first frame pixels
	
	// Optimization for single frame GIFs
	const int frameCount = m_apFrames.GetSize();

	if( frameCount == 1 )
	{
		GifAnimFrame* pFrame = m_apFrames[0];
		if( pFrame->m_offsetX == 0 && pFrame->m_offsetY == 0 
			&& pFrame->m_width == m_width && pFrame->m_height == m_height )
		{
			// Zero copy optimization
			m_pixels = pFrame->m_pixels;
			pFrame->m_pixels.Clear();
			return true;
		}
	}

	// Prepare the main pixel buffer and copy the first frame pixels into it
	const int32 pixelCount = m_width * m_height;
	if( !m_pixels.SetSize(pixelCount) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	
	// Clear background
	uint8 clearIndex = GetClearIndex(0);
	Memory::Set(m_pixels.GetWritePtr(), clearIndex, m_width * m_height);

	if( frameCount == 0 )
	{
		// No image found
		return true;
	}

	// Prepare the display of the first frame
	CopyFramePixels(m_apFrames[0], false);
	return true;
}

bool Gif::ReadLogicalScreenDescriptor(IFile& file)
{
	// Screen size when the gif was created. Usually ignored.
	uint16 logicalScreenWidth = 0;
	uint16 logicalScreenHeight = 0;
	uint8  fields = 0;
	uint8  backgroundColorIndex = 0;
	uint8  pixelAspectRatio = 0;
	
	bool bReadOk = ( 
		file.Read16(logicalScreenWidth)
	 && file.Read16(logicalScreenHeight)
	 && file.Read8(fields)
	 && file.Read8(backgroundColorIndex)
	 && file.Read8(pixelAspectRatio));

	if( !bReadOk )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	bool bGlobalColorTableFlag = false;
	if( fields & 0x80 )
	{
		bGlobalColorTableFlag = true;
	}

	if( bGlobalColorTableFlag )
	{
		//uint8 sortFlag = ((fields & 0x08) >> 3);
		const uint8 lsl = uint8( (fields & 0x07) + 1);
		m_globalColorTable.m_count = (1 << lsl);
	}
	else
	{
		m_globalColorTable.m_count = 0;
	}

	bool bOk = true;
	if( m_globalColorTable.m_count != 0 )
	{
		bOk = ReadColorTable(file, m_globalColorTable);
	}

	m_backgroundColorIndex = backgroundColorIndex;
	m_width = logicalScreenWidth;
	m_height = logicalScreenHeight;

	return bOk;
}

bool Gif::ReadColorTable(IFile& file, Palette& pal)
{
	const int32 colorCount = pal.m_count;
	
	uint8 aBuf[256 * 3];
	const int32 wantedRead = colorCount * 3;
	if( file.Read(aBuf, wantedRead) != wantedRead )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	uint8* pSrc = aBuf;
	for(int i = 0; i < colorCount; ++i )
	{
		uint8 r = pSrc[0];
		uint8 g = pSrc[1];
		uint8 b = pSrc[2];
				
		pal.m_colors[i].SetRgb(r, g, b);

		pSrc += 3;
	}
	return true;
}

bool Gif::ReadExtensionBlock(IFile& file)
{
	uint8 functionCode;
	if( !file.Read8(functionCode) )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	
	if( functionCode == 0xf9 )
	{
		// Graphic Control Extension
		uint8  byteCount;
		uint8  packedField;
		uint16 delayTime;
		uint8  transparentColorIndex;
		
		if( file.Read8(byteCount )
		 && file.Read8(packedField)
		 && file.Read16(delayTime)
		 && file.Read8(transparentColorIndex) )
		{
			if( packedField & 0x01 )
			{
				// The transparent color index is valid
				m_transparentColorIndex = transparentColorIndex;
			}

			m_disposalMethod = (packedField >> 2) & 0x07;
			m_delayTime = delayTime;

			return ReadTerminator(file);
		}
		else
		{
			m_lastError = uncompleteFile;
			return false;
		}
	}
	else if( functionCode == 0xff )
	{
		// Application Extension
		uint8  byteCount;
		uint8  extName[12] = { 0 }; // "NETSCAPE2.0"
		uint8  subBlockByteCount;

		if( file.Read8(byteCount )
		 && file.Read(extName, 11)
		 && file.Read8(subBlockByteCount) )
		{
			if(    Memory::Equals(extName, "NETSCAPE2.0", 11)
				&& subBlockByteCount == 3 )
			{
				uint8  one;
				uint16 iterationCount;

				if( !(file.Read8(one) && file.Read16(iterationCount)) )
				{
					m_lastError = uncompleteFile;
					return false;
				}
				if( one == 1 )
				{
					m_loopCount = iterationCount;
				}
				return ReadTerminator(file);
			}

			// Unknown extension
			uint8 bytes[256]; // 255 bytes max + 1 more byte
			for(;;)
			{
				int readAmount = subBlockByteCount + 1; // +1 for the next byte count or terminator
				if( file.Read(bytes, readAmount) != readAmount )
				{
					m_lastError = uncompleteFile;
					return false;
				}
				uint8 nextByte = bytes[subBlockByteCount];
				if( nextByte == 0 )
				{
					// Terminator found, end of extension
					return true;
				}
				subBlockByteCount = nextByte;
			}
		}
		else
		{
			m_lastError = uncompleteFile;
			return false;
		}
	}

	for(;;)
	{
		uint8 byteCount = 0;
		if( !file.Read8(byteCount) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		if( byteCount == 0 )
			break;
			
		file.SetPosition(byteCount, IFile::posCurrent);
	}
	return true;
}

// The byte type of this block (0x2c = ',') has already been read
bool Gif::ReadImageDescriptor(IFile& file, GifAnimFrame* pFrame)
{
	uint16 imageLeft, imageTop, imageWidth, imageHeight;
	imageLeft = imageTop = imageWidth = imageHeight = 0;
	
	bool bReadOk = file.Read16(imageLeft) && file.Read16(imageTop)
		&& file.Read16(imageWidth) && file.Read16(imageHeight);
	
	if( !bReadOk )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	// Check that the frame rect is inside the main rect
	// If the frame goes off the logical screen, this should be an error.
	// But I found many garbled GIFs on the web with such an error, and they are correcly
	// accepted by browsers, so we just enlarge the logical screen.

	// I would prefer enlarge the logical screen by taking into account the offset, but
	// popular browsers look at the size only
	if( imageWidth > m_width )
	{
		m_width = imageWidth;
	}
	
	if( imageHeight > m_height )
	{
		m_height = imageHeight;
	}

	pFrame->m_offsetX = imageLeft;
	pFrame->m_offsetY = imageTop;
	pFrame->m_width = imageWidth;
	pFrame->m_height = imageHeight;

	uint8 fields;
	if( !file.Read8(fields) )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	
	bool bUseLocalColorMap = false;
	if( fields & 0x80 )
	{
		bUseLocalColorMap = true;
	}

	pFrame->m_interlaced = ((fields & 0x40) != 0);

	if( !bUseLocalColorMap )
	{
		// This frame uses the global palette
		ASSERT(pFrame->m_pPalette == null);
		pFrame->m_pPalette = null;
		return true;
	}

	// A palette is following
	pFrame->m_pPalette = new Palette();
	if( pFrame->m_pPalette == null )
	{
		m_lastError = notEnoughMemory;
		return false;
	}
	const int32 lsl = (fields & 0x07) + 1;
	pFrame->m_pPalette->m_count = (1 << lsl);

	bool bOk = true;
	if( pFrame->m_pPalette->m_count != 0 )
	{
		bOk = ReadColorTable(file, *(pFrame->m_pPalette));
	}

	return bOk;
}

// Returns the compression size
int Gif::InitTable(int codeSize)
{
	// Next empty cell index in the string table.
	// The first cells ( 2^m_nCodeSize) are used by the roots.
	// The two nexts are used by specific codes (that's why we do + 2).
	const int32 rootCount = (1 << codeSize);
	m_nextEntry = rootCount + 2;

	// Initialize the string table
	for(int i = 0; i < rootCount; i++)
	{
		m_lzwStringTable[i].value = uint8(i);
		m_lzwStringTable[i].length = 1;
		m_lzwStringTable[i].previous = -1;
	}

	// Compute the initial code sizes
	int compressionSize = codeSize + 1;
	return compressionSize;
}

// Returns the first character of the string
uint8 Gif::OutputString(int index, uint8* pPixelBuffer)
{
	// As we store the previous values, we scan the string in reverse order
	uint8 firstChar = 0;
	int32 length = m_lzwStringTable[index].length;
	for(int i = length - 1; i >= 0; i--)
	{
		ASSERT(index >= 0 && index < 4096);
		firstChar = m_lzwStringTable[index].value;
		
		const int32 offset = m_outputIndex + i;
		ASSERT(offset >= 0);

		pPixelBuffer[offset] = firstChar;

		index = m_lzwStringTable[index].previous;
	}
	m_outputIndex += length;

	return firstChar;
}

int Gif::ReadBits(uint8* pBuffer, int32 length, int32& bitPosition)
{
	// <abcde> <fghij> <klmno>
	// is coded:
	// hijabcde xklmnofg

	int32 bytePos1 = bitPosition / 8;
	int32 bytePos2 = (bitPosition + length) / 8;
	if( ((bitPosition + length) % 8) == 0 )
	{
		// Do not count this byte
		bytePos2--;
	}

	int32 value = 0;
	for(int i = bytePos2; i >= bytePos1; i--)
	{
		value <<= 8;
		value |= pBuffer[i];
	}

	const uint32 mask = ~(0xffffffff << length);
	const int32 bitOffset = bitPosition % 8;

	value >>= bitOffset;
	value &= mask;

	bitPosition += length;

	if( !(value <= m_nextEntry) )
	{
		// Corrupted GIF
		// Note: it is safe to notify an error in the return value
		// as normal codes are limited to 12 bits, here -1 is 32 bits
		return -1;
	}

	return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Gif::ReadImageData(IFile& file, GifAnimFrame* pFrame)
{
	const int32 pixelCount = pFrame->m_width * pFrame->m_height;

	ASSERT(pFrame->m_pixels.GetSize() == 0);
	if( !pFrame->m_pixels.SetSize(pixelCount) )
	{
		// Error in file
		m_lastError = notEnoughMemory;
		return false;
	}
	
	// More direct access to the pixels
	uint8* const pPixelBuffer = pFrame->m_pixels.GetWritePtr();

	bool terminatorFound = false;
	if( !UncompressLzw(file, pPixelBuffer, terminatorFound) )
	{
		return false;
	}

	if( !terminatorFound )
	{
		// Read the terminator here
		return ReadTerminator(file);
	}
	// Terminator already read
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns true upon success
// terminatorFound [out] true if a successfull decoding stopped because of a terminator found
bool Gif::UncompressLzw(IFile& file, uint8* pPixelBuffer, bool& terminatorFound)
{
	terminatorFound = false;
	// Pixel to be written index
	m_outputIndex = 0;
	
	// This field meaning is tricky. The value is 2 for 2 colors pictures, otherwise it gives
	// the number of bits per pixel
	uint8 lzwMinimumCodeSize;
	if( !file.Read8(lzwMinimumCodeSize) )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	if( lzwMinimumCodeSize > 12 )
	{
		m_lastError = errBadLzwMinimumCodeSize;
		return false;
	}
	const int codeSize = lzwMinimumCodeSize;
	
	// The LZW algorithm used in GIF is slightly modified.
	// It adds two special codes : "clear code" & "end of information"
	const int32 clearCode = (1 << codeSize);
	const int32 endOfInformationCode = (1 << codeSize) + 1;
	
	int compressionSize = InitTable(codeSize);
	
	// Compressed data are made of blocks with 255 bytes maximum in each of them.
	// The anBlocks array contains one block of compressed data.
	// The last bits of the previous block may be used with the first bits of the current block, in
	// order to form a complete code. That's why we allocate two more bytes. The two first bytes
	// of anBlock are the two last bytes of the previous block.
	// Two bytes allow to store 16 bits, which is enough in the worst cases (a 12 bits code overlapping
	// several bytes).
	uint8 anBlock[2 + 256];
	Memory::Zero(anBlock, sizeof(anBlock));
	
	// Position in bits in the data block
	// We start at 16 because the first two bytes are from the previous block.
	int32 bitPosition = 16;

	// The last code read
	int32 lastCodeWord = -1;

	// The last character written
	uint8 currentFirstChar = 0;

	// We loop on all the blocks
	for(;;)
	{
		uint8 blockSize;
		if( !file.Read8(blockSize) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		
		if( blockSize == 0 )
		{
			// The first OK stop reason
			terminatorFound = true;
			break;
		}
		
		// Read the compressed block
		if( file.Read( anBlock + 2, blockSize) != blockSize )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		
		const int32 maxBitSize = (blockSize + 2) * 8;

		// Uncompress the block
		while( (bitPosition + compressionSize) <= maxBitSize)
		{
			int32 currentCodeWord = ReadBits(anBlock, compressionSize, bitPosition);
			
			if( currentCodeWord == -1 )
			{
				// Error in file
				m_lastError = errBadEntryInStringTable;
				return false;
			}

			// Does the read code has a special meaning ?
			if( currentCodeWord == endOfInformationCode )
			{
				// Yes, end of the compressed data (second OK stop reason)
				return true;
			}
			else if( currentCodeWord == clearCode )
			{
				// Yes, wants to re-initialize the table
				compressionSize = InitTable(codeSize);
				lastCodeWord = -1;
			}
			else
			{
				// Is the code in the string table ?
				if( lastCodeWord == -1 )
				{
					// First code read
					currentFirstChar = OutputString(currentCodeWord, pPixelBuffer);
					lastCodeWord = currentCodeWord;
				}
				else if( currentCodeWord < m_nextEntry )
				{
					// The code IS in the table
					
					currentFirstChar = OutputString(currentCodeWord, pPixelBuffer);
					
					if( m_nextEntry < 4096 )
					{
						// Create a new entry, where the string is 
						// lastCodeWord + the first character of the string of currentCodeWord
					
						m_lzwStringTable[m_nextEntry].previous = int16(lastCodeWord);
						m_lzwStringTable[m_nextEntry].value = currentFirstChar;
						m_lzwStringTable[m_nextEntry].length = m_lzwStringTable[lastCodeWord].length + 1;
						m_nextEntry++;
					
						lastCodeWord = currentCodeWord;
						currentCodeWord = -1;
					}
				}
				else
				{
					// The code is NOT in the table
					
					if( m_nextEntry < 4096 )
					{
						// Create a new entry
						// From the old string + the first character of the old string
						m_lzwStringTable[m_nextEntry].previous = int16(lastCodeWord);
						m_lzwStringTable[m_nextEntry].value = currentFirstChar;
						m_lzwStringTable[m_nextEntry].length = m_lzwStringTable[lastCodeWord].length + 1;
					
						OutputString(m_nextEntry, pPixelBuffer);
					
						m_nextEntry++;
					
						lastCodeWord = currentCodeWord;
					}
				}

				// Check if the codes length change
				// Warning : the length does not increase if the last code is 4095
				// Note that we test with 4096 because we just incremented m_nextEntry
				if( m_nextEntry == (1 << compressionSize) && m_nextEntry != 4096 )
				{
					compressionSize++;
				}
			}
		}
		// End of decompression for one block

		// Copy the two last bytes to the first bytes
		anBlock[0] = anBlock[0 + blockSize];
		anBlock[1] = anBlock[1 + blockSize];

		// Initialize the buffer position
		// "16" because of the 2 special first bytes
		bitPosition = 16 - (maxBitSize - bitPosition);
	}
	return true;
}

bool Gif::UninterlaceBuffer(GifAnimFrame* pFrame)
{
	const int32 width = pFrame->m_width;
	const int32 height = pFrame->m_height;

	const int32 bufferSize = width * height;
	ByteArray aTmpBuffer;
	if( !aTmpBuffer.SetSize(bufferSize) )
	{
		m_lastError = notEnoughMemory;
		return false;
	}

	uint8* const pTmpBuffer = aTmpBuffer.GetPtr();
	uint8* const pPixelBuffer = pFrame->m_pixels.GetWritePtr();
	Memory::Copy(pTmpBuffer, pPixelBuffer, bufferSize);

	// Pass 1
	uint8* pSource = pTmpBuffer;
	uint8* pDest = pPixelBuffer;
	for(int i = 0; i < height; i += 8)
	{
		Memory::Copy(pDest, pSource, width);
		pSource += width;
		pDest += 8 * width;
	}

	// Pass 2
	pDest = pPixelBuffer + 4 * width;
	for(int i = 4; i < height; i += 8)
	{
		Memory::Copy(pDest, pSource, width);
		pSource += width;
		pDest += 8 * width;
	}

	// Pass 3
	pDest = pPixelBuffer + 2 * width;
	for(int i = 2; i < height; i += 4)
	{
		Memory::Copy(pDest, pSource, width);
		pSource += width;
		pDest += 4 * width;
	}

	// Pass 4
	pDest = pPixelBuffer + 1 * width;
	for(int i = 1; i < height; i += 2)
	{
		Memory::Copy(pDest, pSource, width);
		pSource += width;
		pDest += 2 * width;
	}
	return true;
}

String Gif::GetLastErrorString() const
{
	switch(m_lastError)
	{
	case Gif::errNotAGifFile:
		return "Not a GIF file";

	case Gif::errBadEntryInStringTable:
		return "Bad entry in string table (file may by corrupted)";
	
	case Gif::errBadGifVersion:
		return "Bad GIF version, should be 87a or 89a";

	case Gif::errBlockTerminatorNotFound:
		return "Block terminator not found (0x00)";
	
	case Gif::errBadLzwMinimumCodeSize:
		return "Bad LZW minimum code size";
	
	case Gif::errBadImageRect:
		return "Bad image rect";
	}

	return ImageFormat::GetLastErrorString();
}

void Gif::CopyFramePixels(const GifAnimFrame* pFrame, bool bRestore)
{
	const uint8* pSrc = pFrame->m_pixels.GetReadPtr();
	if( bRestore )
	{
		//pSrc = frame.m_aRestoreBuffer.GetPtr();
	}

	const int32 frameWidth = pFrame->m_width;
	const int32 frameHeight = pFrame->m_height;

	uint8* pDst = m_pixels.GetWritePtr();

	// Small rect copy into big rect
	pDst += (pFrame->m_offsetY * m_width) + pFrame->m_offsetX;

	// Transparency checking is done here
	const int16 transparentIndex = pFrame->m_transparentIndex;

	for(int iY = 0; iY < frameHeight; ++iY)
	{
		for(int iX = 0; iX < frameWidth; ++iX)
		{
			int16 index = pSrc[iX];
			if( index != transparentIndex )
			{
				pDst[iX] = uint8(index);
			}
		}
		pDst += m_width;
		pSrc += frameWidth;
	}
}

uint8 Gif::GetClearIndex(int32 frameIndex)
{
	// How does GIF defines a transparent background ?
	// The old Netscape navigator used this method:
	// if a Graphic Control Extension block is present and its transparency is turned on,
	// then the background will be transparent.
		
	// Use the transparent color of the current frame.
	uint8 clearIndex = m_backgroundColorIndex;
	if( m_apFrames.GetSize() >= 1 )
	{
		int16 transIndex = m_apFrames[frameIndex]->m_transparentIndex;
		if( transIndex >= 0 )
		{
			clearIndex = uint8(transIndex);
		}
	}

	return clearIndex;
}

