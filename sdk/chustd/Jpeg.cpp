///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jpeg.h"
#include "File.h"

#include "Math.h"
#include "FixArray.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;
//////////////////////////////////////////////////////////////////////
static const uint8 k_JpegSignature[2] = { 0xFF, 0xD8 };

int32 Jpeg::ms_anZigzag2dTo1d[64] = {-1};
int32 Jpeg::ms_anZigzag1dTo2d[64] = {-1};
int32 Jpeg::ms_anQuantIdctPreMultTable[64] = {-1};
//////////////////////////////////////////////////////////////////////

void Jpeg::HuffmanByteReader::Begin()
{
	m_nPos = 0;
	m_nSize = 0;
}

// Returns -1 upon error
int Jpeg::HuffmanByteReader::ReadByte(IFile& file)
{
	const int32 maxSize = sizeof(m_aBytes);

	if( m_nSize == m_nPos )
	{
		// Read from file
		m_nSize = file.Read(m_aBytes, maxSize);
		m_nPos = 0;

		if( m_nSize == 0 )
		{
			return -1; // No more bytes
		}
	}

	int byte = m_aBytes[m_nPos];
	m_nPos++;

	return byte;
}

// If we read too much data, go back to the last real huffman byte
void Jpeg::HuffmanByteReader::End(IFile& file)
{
	int32 tooMuchCount = m_nSize - m_nPos;
	ASSERT( tooMuchCount >= 0);

	file.SetPosition(-tooMuchCount, IFile::posCurrent);
}

//////////////////////////////////////////////////////////////////////
// Only used for Huffman decoding
// Note : the huffman stream in the jpeg file is a real mess, as we find
// from time to time special 0xff codes. The next byte after a 0xff value has a special
// meaning which must be interpreted during the stream read.
// If an error occurs, bOk is set to false
int Jpeg::ReadBits(IFile& file, int32 bitCount, bool& bOk)
{
	int32 byteCount = (m_nBitOffset + bitCount) / 8;
	int32 remainBitCount = (m_nBitOffset + bitCount) % 8;
	byteCount += (remainBitCount == 0) ? 0 : 1;

	uint8 byte = m_nLastByte;

	// Final return value
	int32 value = 0;

	for(int i = 0; i < byteCount; i++)
	{
		// Two solutions :
		// 1) Either we keep the last byte and use it as the first byte of the value to decode
		// 2) Either we read a new byte

		// If the byte position is not 0, we read new bytes
		if( m_nBitOffset == 0 || i > 0 )
		{
			// -1 on error means 0xff as an uint8 so the special treatment will apply
			byte = (uint8) m_huffbytereader.ReadByte(file);
						
			// Yuk ! We must take care of 0xff values in the Huffman stream
			
			// Warning ! We should do the comparison only if the byte has been freshly extracted of the stream
			if( byte == 0xff )
			{
				// Special treatment when we found a 0xff byte

				//uint8 nextByte;
				//file.Read(&nextByte, 1);
				int32 nextByte = m_huffbytereader.ReadByte(file);
				if( nextByte < 0 )
				{
					bOk = false;
					m_lastError = uncompleteFile;
					return 0;
				}
				else if( nextByte == 0 )
				{
					// The 0 means 0xff is not coding a special byte and must be considered as a valid
					// Huffman byte in the compressed stream
				}
				else if( 0xd0 <= nextByte && nextByte <= 0xd7 )
				{
					m_nMarkerFoundInImageData = mustResetMcu;
					m_nBitOffset = 0;
					return 0;
				}
				else if( nextByte == 0xd9 )
				{
					m_nMarkerFoundInImageData = endOfImage;
					return 0;
				}
				else
				{
					bOk = false;
					m_lastError = errInvalidMarkerInHuffmanData;
					return 0;
				}
			}
			value <<= 8;
		}

		value |= byte;
	}

	int32 shift = (8 - remainBitCount) % 8;
	value >>= shift;

	uint32 mask = ~(0xffffffff << bitCount);
	value &= mask;

	m_nLastByte = byte; // Last byte
	m_nBitOffset = remainBitCount;

	bOk = true;

	return value;
}

//////////////////////////////////////////////////////////////////////


Jpeg::Jpeg()
{
	Initialize();
}

Jpeg::~Jpeg()
{

}

PixelFormat Jpeg::GetPixelFormat() const
{
	return PF_24bppRgb;
}

const Palette& Jpeg::GetPalette() const
{
	return Palette::Null;
}

//////////////////////////////////////////
void Jpeg::Initialize()
{
	m_width = 0;
	m_height = 0;
	m_lastError = 0;
	
	m_nRestartInterval = 0;
	m_nBitOffset = 0;
}

//////////////////////////////////////////
void Jpeg::BuildZigzag2dTo1d()
{
	int32 nV = 0;
	for(int iIp = 0; iIp <= 7; iIp++)
	{
		if( iIp % 2 == 0 )
		{
			int32 y = iIp;
			int32 x = 0;

			for(int iIs = 0; iIs <= iIp; iIs++)
			{
				ms_anZigzag2dTo1d[y * 8 + x] = nV;
				nV++;
				y--;
				x++;
			}
		}
		else
		{
			int32 y = 0;
			int32 x = iIp;

			for(int iIs = 0; iIs <= iIp; iIs++)
			{
				ms_anZigzag2dTo1d[y * 8 + x] = nV;
				nV++;
				y++;
				x--;
			}
		}
	}

	for(int iRow = 1; iRow <= 7; iRow++)
	{
		for(int iCol = 8 - iRow; iCol <= 7; iCol++)
		{
			const int32 destOffset = iCol * 8 + iRow;
			const int32 srcOffset = (7 - iCol) * 8 + (7 - iRow);
			
			ms_anZigzag2dTo1d[destOffset] = 63 - ms_anZigzag2dTo1d[srcOffset];
		}
	}
}

void Jpeg::BuildZigzag1dTo2d()
{
	int32 x = 0;
	int32 y = 0;
	for(int i = 0; i < 64; i++ )
	{
		ms_anZigzag1dTo2d[i] = y * 8 + x;
		if( (x + y) % 2 )
		{
			y++;
			if( y > 7 )
			{
				y = 7;
				x++;
			}
			else
			{
				x--;
				if( x < 0 )
					x = 0;
			}
		}
		else
		{
			x++;
			if( x > 7 )
			{
				x = 7;
				y++;
			}
			else
			{
				y--;
				if( y < 0 )
					y = 0;
			}
		}
	}
}

void Jpeg::BuildTables()
{
	BuildZigzag1dTo2d();
	BuildZigzag2dTo1d();
	BuildQuantIdctPreMultTable();
}

bool Jpeg::IsJpeg(IFile& file)
{
	const int32 signSize = sizeof(k_JpegSignature);
	uint8 aRead[signSize];
	
	if( file.Read(aRead, signSize) != signSize )
	{
		return false;
	}
	
	if( !chustd::Memory::Equals(k_JpegSignature, aRead, signSize) )
	{
		return false;
	}
	return true;
}

bool Jpeg::LoadFromFile(IFile& file)
{
	//////////////////////////////////////////////
	// Build zig zag lookup table
	if( ms_anZigzag2dTo1d[0] == -1
		|| ms_anZigzag1dTo2d[0] == -1
		|| ms_anQuantIdctPreMultTable[0] == -1)
	{
		BuildTables();
	}
	//////////////////////////////////////////////

	// Reset the object
	FreeBuffer();
	Initialize();

	//////////////////////////////////////////////////////
	// Read the header first
	if( !IsJpeg(file) )
	{
		m_lastError = errNotAJpegFile;
		return false;	
	}
	//////////////////////////////////////////////////////

	/////////////////////////////////////////////////////
	// Jpeg works in big endian mode
	TmpEndianMode tem(file, boBigEndian);

	uint32 segmentSize;
	uint16 tmp16 = 0;

	while(true)
	{
		uint8 marker = ReadMarker(file);
		if( marker == 0x00 || marker == 0xff )
		{
			// Error or Forbidden marker numbers
			m_lastError = errForbiddenMarkerFound;
			
			// Jump out of the loop and returns an error
			break;
		}

		switch(marker)
		{
		case 0xd8:
			m_lastError = errStartOfImageMarkerFoundTwice;
			return false; // Start Of Image, already read

		case 0xe0:
			// APP0: JFIF Application Segment
			if( !ReadSegment_JFIF(file) )
				return false;
			break;

		case 0xdb:
			// DQT: Quantization table(s)
			if( !ReadSegment_QuantizationTables(file) )
				return false;
			break;

		case 0xc0:
			// SOF0: Start Of Frame
			if( !ReadSegment_StartOfFrame(file) )
				return false;
			break;

		case 0xc4:
			// DHT: Define Huffman Table
			if( !ReadSegment_HuffmanTable(file) )
				return false;
			break;

		case 0xda:
			// SOS: Start Of Scan
			if( !ReadSegment_StartOfScan(file) )
				return false;

			if( m_nMarkerFoundInImageData )
				return true;
			break;
		
		case 0xd9:
			// End of file reached
			return true;
			
		///////////////////////////////////////////////////////////////////////
		case 0xc1:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatC1;
			return false;

		case 0xc2:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatC2;
			return false;

		case 0xc3:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatC3;
			return false;

		case 0xc5:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatC5;
			return false;

		case 0xc6:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatC6;
			return false;

		case 0xc7:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatC7;
			return false;

		case 0xc8:
			m_lastError = errUnsupportedSegmentC8;
			return false;

		case 0xc9:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatC9;
			return false;

		case 0xca:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatCA;
			return false;

		case 0xcb:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatCB;
			return false;

		case 0xcc:
			m_lastError = errUnsupportedSegmentCC;
			return false;

		case 0xcd:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatCD;
			return false;

		case 0xce:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatCE;
			return false;

		case 0xcf:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedFormatCF;
			return false;

		case 0xd0:
		case 0xd1:
		case 0xd2:
		case 0xd3:
		case 0xd4:
		case 0xd5:
		case 0xd6:
		case 0xd7:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnexpectedTerminationOfRestartInterval;
			return false;
	
		// 0xd9: EOI: End Of Image

		case 0xdc:
			// ?? La documentation says "Usually unsupported, ignore" !
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			
			file.Read16(tmp16);
			m_height = tmp16;
			break;

		case 0xdd:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			
			file.Read16(tmp16);
			m_nRestartInterval = tmp16;
			break;

		case 0xde:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedSegmentDE;
			return false;

		case 0xdf:
			file.Read16(tmp16);
			segmentSize = tmp16 - 2;
			m_lastError = errUnsupportedSegmentDF;
			return false;

		////////////////////////////////////////////////////////
		// APPn : Application Segment
		case 0xe1:
		case 0xe2:
		case 0xe3:
		case 0xe4:
		case 0xe5:
		case 0xe6:
		case 0xe7:
		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:
			file.Read16(tmp16);
			if( tmp16 < 2 )
			{
				m_lastError = errInvalidSegmentSize;
				return false;
			}
			segmentSize = tmp16 - 2;
			file.SetPosition(segmentSize, IFile::posCurrent);
			break;
		////////////////////////////////////////////////////////

		case 0xf0:
		case 0xf1:
		case 0xf2:
		case 0xf3:
		case 0xf4:
		case 0xf5:
		case 0xf6:
		case 0xf7:
		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
			m_lastError = errUnsupportedSegmentF0_FD;
			return false;

		case 0xfe:
			file.Read16(tmp16);
			if( tmp16 < 2 )
			{
				m_lastError = errInvalidSegmentSize;
				return false;
			}

			segmentSize = tmp16 - 2;
			ReadComment(file, segmentSize);
			break;

		case 0x01:
			m_lastError = errUnsupportedSegment01;
			return false;

		case 0x02:
			m_lastError = errUnsupportedSegment02;
			return false;

		case 0xbf:
			m_lastError = errUnsupportedSegmentBF;
			return false;
		}
	}

	return false;
}

uint8 Jpeg::ReadMarker(IFile& file)
{
	while(true)
	{
		uint8 aMarker[2];
		int32 read = file.Read(aMarker, 2);
		if( read < 2 )
			break;

		if( aMarker[0] == 0xff )
		{
			// Marker lead byte.
			if( aMarker[1] != 0x00 )
				return aMarker[1];
		}
	}
	return 0x00; // Unreachable code
}

// APP0
bool Jpeg::ReadSegment_JFIF(IFile& file)
{
	uint16 segmentSize;
	if( !file.Read16(segmentSize) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	if( segmentSize < 2 )
	{
		m_lastError = errInvalidSegmentSize;
		return false;
	}

	segmentSize -= 2;

	if( segmentSize >= 5 )
	{
		char szId[5];
		file.Read(szId, 5);
		
		szId[4] = 0;
		if( Memory::Equals(szId, "JFIF", 4) )
		{
			uint8 majorVersion, minorVersion;
			if( !(file.Read8(majorVersion) && file.Read8(minorVersion)) )
			{
				m_lastError = uncompleteFile;
				return false;
			}
			
			uint8 units; // 0=none
			if( !file.Read8(units) )
			{
				m_lastError = uncompleteFile;
				return false;
			}

			// x/y-density specify the aspect ratio instead. 1=dots/inch. 2=dots/cm
			if( !(file.Read16(m_nXDensity) && file.Read16(m_nYDensity)) )
			{
				m_lastError = uncompleteFile;
				return false;
			}

			// Thumbnail size
			uint8 thumbWidth, thumbHeight;
			if( !(file.Read8(thumbWidth) && file.Read8(thumbHeight)) )
			{
				m_lastError = uncompleteFile;
				return false;
			}
				

			// Incoming data contains the thumbnail picture data
			// - 9 bytes for the 7 values we just read
			// - 5 bytes for the "JFIF" string
			const int32 advance = segmentSize - 5 - 9;
			if( !file.SetPosition(advance, IFile::posCurrent) )
			{
				// Cannot seek
				m_lastError = uncompleteFile;
				return false;
			}
		}
		else if( Memory::Equals(szId, "JFXX", 4) )
		{
			uint8 extension;
			if( !file.Read8(extension) )
			{
				m_lastError = uncompleteFile;
				return false;
			}
			
			m_nExtension= extension;	//	0x10:JPEG 0x11:byte/pixel 0x13:3byte/pixel
			const int32 advance = segmentSize - 5 - 1;
			
			// Thumbnail
			if( !file.SetPosition(advance, IFile::posCurrent) )
			{
				// Cannot seek
				m_lastError = uncompleteFile;
				return false;
			}
		}
		else
		{
			const int32 advance = segmentSize - 5;
			if( !file.SetPosition(advance, IFile::posCurrent) )
			{
				// Cannot seek
				m_lastError = uncompleteFile;
				return false;
			}
		}
	}
	else
	{
		if( !file.SetPosition(segmentSize, IFile::posCurrent) )
		{
			// Cannot seek
			m_lastError = uncompleteFile;
			return false;
		}
	}
	return true;
}

// DQT
bool Jpeg::ReadSegment_QuantizationTables(IFile& file)
{
	uint16 segmentSize;
	if( !file.Read16(segmentSize) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	if( segmentSize < 2 )
	{
		m_lastError = errInvalidSegmentSize;
		return false;
	}

	segmentSize -= 2;

	const uint32 maxRead = uint32(file.GetPosition()) + segmentSize;
	
	while( file.GetPosition() < maxRead )
	{
		// Pq: Quantization table element precision
		// Tq: Quantization table destination identifier
		uint8 nPqTq;
		if( !file.Read8(nPqTq) )
		{
			m_lastError = uncompleteFile;
			return false;
		}

		const int32 elementPrecision = (nPqTq >> 4) & 0x03; // Pq
		const int32 tableIdentifier = nPqTq & 0x0f;         // Tq
		
		// The value 0 for Pq means 8 bits per element
		// The value 1 for Pq means 16 bits per element
		if( elementPrecision == 0 )
		{
			int8 aElements[64];
			if( file.Read(aElements, 64) != 64 )
			{
				m_lastError = uncompleteFile;
				return false;
			}

			for(int i = 0; i < 64; ++i )
			{
				const int8 element = aElements[i];
				m_aanQuantizationTables[tableIdentifier][ms_anZigzag1dTo2d[i]] = element;
			}
		}
		else
		{
			int16 aElements[64];
			const int32 wantedSize = sizeof(aElements);
			if( file.Read(aElements, wantedSize) != wantedSize )
			{
				m_lastError = uncompleteFile;
				return false;
			}

			if( file.ShouldSwapBytes() )
			{
				IFile::Swap16(aElements, 64);
			}

			for(int i = 0; i < 64; ++i )
			{
				const int16 element = aElements[i];
				m_aanQuantizationTables[tableIdentifier][ms_anZigzag1dTo2d[i]] = element;
			}
		}

		// Change the scale factors in order to prepare the fast IDCT routine
		PrepareQuantizationTableForFastIdct(m_aanQuantizationTables[tableIdentifier]);
	}

	return true;
}

// SOF0
// Returns true if sucess, false otherwise with m_lastError filled
bool Jpeg::ReadSegment_StartOfFrame(IFile& file)
{
	uint16 segmentSize;
	if( !file.Read16(segmentSize) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	if( segmentSize < 2 )
	{
		m_lastError = errInvalidSegmentSize;
		return false;
	}

	segmentSize -= 2;


	// Number of bit for each component
	if( !file.Read8(m_nSamplePrecision) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	// Size of the picture
	uint16 height; // the height comes first !
	uint16 width;
	if( !(file.Read16(height) && file.Read16(width)) )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	m_width = int32(width);
	m_height = int32(height);

	// Number of component per pixel
	// 1 = grey scaled, 3 = color YCbCr or YIQ, 4 = color CMYK
	if( !file.Read8(m_nComponentCount) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	if( m_nComponentCount != 3 && m_nComponentCount != 1 )
	{
		m_lastError = errUnsupportedPlaneSize;
		return false;
	}

	m_nMaxXSampling = 0;
	m_nMaxYSampling = 0;

	////////////////////////////////////////////////////////////////////////////
	// Gather the components information
	uint8 aCompoInfo[5 * 3];
	const int32 compoSize =  m_nComponentCount * 3;
	if( file.Read(aCompoInfo, compoSize) != compoSize )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	const uint8* pCompo = aCompoInfo;
	for(int iComponent = 0; iComponent < m_nComponentCount; ++iComponent )
	{
		// 1 = Y, 2 = Cb, 3 = Cr, 4 = I, 5 = Q
		uint8 componentIdentifier = pCompo[0];
		(void)componentIdentifier; // Debug

		// Hi: Horizontal sampling factor
		// Vi: Vertical sampling factor
		uint8 nHiVi = pCompo[1];

		const uint8 horizontalSamplingFactor = uint8(nHiVi >> 4); // Hi
		const uint8 verticalSamplingFactor = uint8(nHiVi & 0x0f); // Vi

		m_anXSampling[iComponent] = horizontalSamplingFactor;
		m_anYSampling[iComponent] = verticalSamplingFactor;

		if( horizontalSamplingFactor > m_nMaxXSampling )
			m_nMaxXSampling = horizontalSamplingFactor;

		if( verticalSamplingFactor > m_nMaxYSampling )
			m_nMaxYSampling = verticalSamplingFactor;

		uint8 quantizationTableDestinationSelector = pCompo[2];
				
		m_anQuantizationTableSelectors[iComponent] = quantizationTableDestinationSelector;

		// Precompute the shift factor for fast multiplication in ConvertMcuToRgb()
		static const FixArray<uint8, 4> anPossibleShifts = { { 0, 0, 1, 2 } };

		// We should not go out of range of the array
		m_anShiftX[iComponent] = anPossibleShifts[ m_nMaxXSampling / horizontalSamplingFactor];
		m_anShiftY[iComponent] = anPossibleShifts[ m_nMaxYSampling / verticalSamplingFactor];

		pCompo += 3;
	}

	return true;
}

// DHT
// Returns true if no error
bool Jpeg::ReadSegment_HuffmanTable(IFile& file)
{
	uint16 segmentSize;
	if( !file.Read16(segmentSize) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	if( segmentSize < 2 )
	{
		m_lastError = errInvalidSegmentSize;
		return false;
	}
	
	segmentSize -= 2;

	/////////////////////////////////////
	// Set the read buffer
	uint8 aStackBuffer[512];

	uint8* pBuffer;
	if( segmentSize < sizeof(aStackBuffer) )
	{
		pBuffer = aStackBuffer;
	}
	else
	{
		pBuffer = new uint8[segmentSize];
	}
	/////////////////////////////////////

	int32 read = file.Read(pBuffer, segmentSize);
	if( read != segmentSize )
	{
		m_lastError = errUnexpectedEndOfFileWhileReadingHuffmanTable;
		return false;
	}

	int32 iByte = 0;

	while(iByte < segmentSize)
	{
		// Tc: Table class, 0 = DC, 1 = AC
		// Th: Huffman table destination identifier
		uint8 nTcTh = pBuffer[iByte];
		iByte++;
		
		const uint8 tableClass = uint8(nTcTh >> 4); // Tc
		const uint8 tableDestinationIdentifier = uint8(nTcTh & 0x0f); // Th
		
		if( tableClass > 1 )
		{
			m_lastError = errInvalidHuffmanTableClass;
			return false;
		}

		if( tableDestinationIdentifier > 3 )
		{
			m_lastError = errInvalidHuffmanTableDestinationIdentifier;
			return false;
		}
		
		int32 anCodenum[16];

		// Total count of codes for the current Huffman table
		int32 totalCodeCount = 0;

		// L1, L2, ..., L16
		// Li: Number of Huffman codes of length i
		for(int i = 0; i < 16; i++ )
		{
			uint8 nLi = pBuffer[iByte + i];
			anCodenum[i] = nLi;

			totalCodeCount += nLi;
		}
		iByte += 16;
		
		// Store the number of Huffman code for this table
		Array<SHuffmanCode>& aTable = m_aaaHuffmanTables[tableClass][tableDestinationIdentifier];
		aTable.SetSize(totalCodeCount);

		// Assignement of the length of each Huffman code
		int32 iCode = 0;
		for(int i = 0; i < 16; i++ )
		{
			for(int32 j = 0; j < anCodenum[i]; j++)
			{
				if( iCode >= totalCodeCount )
				{
					m_lastError = errInvalidHuffmanTable;
					return false;
				}

				aTable[iCode].size = i + 1;
				iCode++;
			}
		}
		
		// Computes Huffman codes
		int32 code = 0;
		int size = 0;
		
		for(int iCodeNum = 0; iCodeNum < totalCodeCount; iCodeNum++ )
		{
			while( aTable[iCodeNum].size != size )
			{
				code = code << 1;
				size++;
			}
			aTable[iCodeNum].code = code++;
		}
		
		// Gets the values coded by each Huffman code
		for( iCode = 0; iCode < totalCodeCount; iCode++ )
		{
			uint8 value = pBuffer[iByte + iCode];
			aTable[iCode].value = value;
		}
		iByte += totalCodeCount;
	}

	if( pBuffer != aStackBuffer )
	{
		delete[] pBuffer;
	}

	return true;
}

// SOS: Start Of Scan
bool Jpeg::ReadSegment_StartOfScan(IFile& file)
{
	uint16 segmentSize;
	if( !file.Read16(segmentSize) )
	{
		m_lastError = uncompleteFile;
		return false;
	}

	if( segmentSize < 2 )
	{
		m_lastError = errInvalidSegmentSize;
		return false;
	}
	
	segmentSize -= 2;

	// Number of components in scan
	// must be 1 <= and <=4 (otherwise error), usually 1 or 3
	uint8 scanNum;
	if( !file.Read8(scanNum) )
	{
		m_lastError = uncompleteFile;
		return false;
	}
	m_nScanComponentCount = scanNum;

	// For each component: 2 bytes
	for(int i = 0; i < scanNum; i++ )
	{
		// Component id (1 = Y, 2 = Cb, 3 = Cr, 4 = I, 5 = Q)
		// Y = luminence, Cb & Cr = chrominance
		uint8 scanComponentSelector;
		if( !file.Read8(scanComponentSelector) )
		{
			m_lastError = uncompleteFile;
			return false;
		}
		m_anScanComponentSelector[i] = scanComponentSelector;

		// Huffman table to use
		// bit 0..3: AC table (0..3)
		// bit 4..7: DC table (0..3) 
		uint8 huffmanTableToUse;
		if( !file.Read8(huffmanTableToUse) )
		{
			m_lastError = uncompleteFile;
			return false;
		}

		uint8 nAcTable = uint8(huffmanTableToUse >> 4);
		uint8 nDcTable = uint8(huffmanTableToUse & 0x0f);

		m_anScanHuffmanTableSelector[0][i] = nAcTable;
		m_anScanHuffmanTableSelector[1][i] = nDcTable;
	}
	
	// 3 bytes to be ignored (???) 
	// 0: Spectral Start
	// 1: Spectral End
	uint8 aTmp[3];
	file.Read(aTmp, 3);

	// Remarks:
	// The image data (scans) is immediately following the SOS segment.

	// m_huffbytereader is used for optimization
	m_huffbytereader.Begin();

	ReadCompressedImageData(file);

	m_huffbytereader.End(file);

	return true;
}

void Jpeg::ReadCompressedImageData(IFile& file)
{
	const int32 pixelBufferSize = m_width * m_height * 3;
	m_pixels.SetSize(pixelBufferSize);
	
	/////////////////////////////////////////////////////////////
	// Initialize the block array of the MCU
	for(int iComponent = 0; iComponent < m_nComponentCount; iComponent++ )
	{
		const int32 blockCountX = m_anXSampling[iComponent];
		const int32 blockCountY = m_anYSampling[iComponent];

		for(int iBlockY = 0; iBlockY < blockCountY; iBlockY++)
		{
			for(int iBlockX = 0; iBlockX < blockCountX; iBlockX++)
			{
				m_aaapMcuBlocks[iComponent][iBlockY][iBlockX] = new int32[64];
			}
		}
	}
	
	DecodeAllMcus(file);

	/////////////////////////////////////////////////////////////
	// Destroy the temporary blocks of MCU
	for(int iComponent = 0; iComponent < m_nComponentCount; iComponent++ )
	{
		const int32 blockCountX = m_anXSampling[iComponent];
		const int32 blockCountY = m_anYSampling[iComponent];
	
		for(int iBlockY = 0; iBlockY < blockCountY; iBlockY++)
		{
			for(int iBlockX = 0; iBlockX < blockCountX; iBlockX++)
			{
				delete[] m_aaapMcuBlocks[iComponent][iBlockY][iBlockX];
			}
		}
	}
	/////////////////////////////////////////////////////////////
}

void Jpeg::DecodeAllMcus(IFile& file)
{
	// Number of MCU horizontally and vertically
	const int32 mcuWidth = 8 * m_nMaxXSampling;
	const int32 mcuHeight = 8 * m_nMaxYSampling;

	const int32 mcuCountX = m_width / mcuWidth + ( (m_width % mcuWidth) ? 1 : 0);
	const int32 mcuCountY = m_height / mcuHeight + ( (m_height % mcuHeight) ? 1 : 0);

	/////////////////////////////////////////////////////////////
	// Initialize the previous step DC values for each component type of the block
	m_anLastDCValues[0] = 0;
	m_anLastDCValues[1] = 0;
	m_anLastDCValues[2] = 0;

	// TODO : support for component count == 1
	if( m_nComponentCount != 3 )
		return;

	// MCU position in pixels in the destination pixel buffer
	int32 iMcuPixelY = 0;
	int32 iMcuPixelX = 0;

	for(int iMcuY = 0; iMcuY < mcuCountY; iMcuY++ )
	{
		for(int iMcuX = 0; iMcuX < mcuCountX; iMcuX++ )
		{
			m_nMarkerFoundInImageData = 0;

			DecodeOneMcu(file);
			if( m_nMarkerFoundInImageData == mustResetMcu )
			{
				m_anLastDCValues[0] = 0;
				m_anLastDCValues[1] = 0;
				m_anLastDCValues[2] = 0;
				
				m_nMarkerFoundInImageData = 0;
				DecodeOneMcu(file);
			}
			else if( m_nMarkerFoundInImageData == endOfImage )
			{
				return;
			}

			ConvertMcuToRgb(iMcuPixelX, iMcuPixelY, mcuWidth, mcuHeight);

			iMcuPixelX += mcuWidth;
		}
		iMcuPixelX = 0;
		iMcuPixelY += mcuHeight;
	}
}

void Jpeg::DecodeOneMcu(IFile& file)
{
	// For each "color" component (not exactly color as they are Y, U=Cb and V=Cr)
	for(int iComponent = 0; iComponent < m_nComponentCount; iComponent++ )
	{
		const int32 blockCountX = m_anXSampling[iComponent];
		const int32 blockCountY = m_anYSampling[iComponent];
		
		for(int iBlockY = 0; iBlockY < blockCountY; iBlockY++)
		{
			for(int iBlockX = 0; iBlockX < blockCountX; iBlockX++)
			{
				int32* pBlock = m_aaapMcuBlocks[iComponent][iBlockY][iBlockX];
				
				// Decoding of the 8x8 block for the current color component (which number is iComponent)
				// m_aanCurrentBlocks[iComponent] is assigned with scaled DCT values
				BlockDecodeHuffman(file, iComponent, pBlock);

				if( m_nMarkerFoundInImageData != 0 )
					return;
				
				// Unscale the DCT values
				BlockInverseQuantization(iComponent, pBlock);
				
				// Do the reverse DCT computation in order to get the initial values
				// for the current "color" component (Y, U=Cb or V=Cr)
				BlockFastIdct(pBlock);
			}
		}
	}
}

// Decode the Huffman/RLE part of a 8x8 block
// component: index of the component the block is associated to (Y, U=Cb, V=Cr)
// pPreIdctBlock: pointer to an array which receives the 64 decoded values (8x8 = 64)
// Returns true if the decoding was successfull
bool Jpeg::BlockDecodeHuffman(IFile& file, int32 component, int32* pPreIdctBlock)
{
	// DEBUG
	static int32 cc = 0; cc++; //ASSERT(cc != 2);

	// Get _one_ byte encoded with the Huffman algorithm
	// The byte represents : 1) the number of previous zeros and 2) the category

	// The next value after this byte is the DC delta corresponding to the 8x8 block
	// Note: the high nibble is always 0, so we don't need to apply a 0xf0 mask
	bool bOk;
	uint8 category = DecodeHuffman(file, 0, component, bOk);
	if( !bOk )
		return false;

	if( m_nMarkerFoundInImageData != 0 )
		return true;

	// Gets the value which length is N bits, with N = category. That balue is the DC delta
	int32 deltaDCValue = 0;
	if( category > 0 )
	{
		deltaDCValue = ReadBits(file, category, bOk);
		if( !bOk )
			return false;

		// Take care of negative numbers
		// Negative numbers start with 0
		const int32 mask = 1 << (category - 1);

		if( (deltaDCValue & mask) == 0 )
		{
			// The far-left bit is 0, this means the DC delta value is negative !
			// We must convert this strange negative number encoding into
			// a more usable negative number.

			// Method : add 1 and fill the left remaining bits with 1:
			// 0 1101 -> 0 1110 -> ... 1110 1110

			const int32 ones = 0xffffffff << category;
			deltaDCValue++;
			deltaDCValue |= ones;
		}
	}
	
	m_anLastDCValues[component] += deltaDCValue;
	pPreIdctBlock[ ms_anZigzag1dTo2d[0] ] = m_anLastDCValues[component];
	
	////////////////////////////////////////////////////////////////////
	// AC values decoding
	for(int i = 1; i < 64; i++ )
	{
		uint8 zeroCountAndCategory = DecodeHuffman(file, 1, component, bOk);
		if( !bOk )
			return false;

		if( zeroCountAndCategory == 0 )
		{
			// Special byte (0, 0) means the remaining values are 0
			for( ; i < 64; i++ )
			{
				pPreIdctBlock[ ms_anZigzag1dTo2d[i] ] = 0;
			}
			break;
		}
		
		// Number of previous 0
		uint8 zeroCount = uint8(zeroCountAndCategory >> 4);
		
		// Number of bits to code the AC value
		category = uint8(zeroCountAndCategory & 0x0f);
		
		// Get the AC value
		int32 nACValue = 0;
		if( category != 0 )
		{
			// Do the same trick for negative numbers
			// (read some row upper)
			nACValue = ReadBits(file, category, bOk);
			if( !bOk )
				return false;

			const int32 mask = 1 << (category - 1);

			if( (nACValue & mask) == 0 )
			{
				const int32 ones = 0xffffffff << category;
				nACValue++;
				nACValue |= ones;
			}
		}

		// Fill the array with 0
		while( zeroCount > 0 )
		{
			ASSERT(i < 64);
			pPreIdctBlock[ ms_anZigzag1dTo2d[i] ] = 0;
			i++;
			zeroCount--;
		}

		// Assign the array element with the final decoded value
		ASSERT(i < 64);
		pPreIdctBlock[ ms_anZigzag1dTo2d[i] ] = nACValue;
	}

	return true;
}

// Decodes one Huffman encoded byte
// Sets bOk to true if no error, false otherwise
uint8 Jpeg::DecodeHuffman(IFile& file, int32 acdc, int32 component, bool& bOk)
{
	bOk = true;

	static int32 cc = 0; cc++; //ASSERT(cc != 539);

	const int32 tableNum = m_anScanHuffmanTableSelector[acdc][component];
	const Array<SHuffmanCode>& aTable = m_aaaHuffmanTables[acdc][tableNum];
	
	int32 curLength = 0; // Length of the currently decoded code
	int32 curCode = 0;   // Current retrieved code in the encoded stream

	int32 iHC = 0;
	const int32 nHCCount = aTable.GetSize();
	while( curLength < 16 )
	{
		curLength++;
		curCode = curCode << 1;
		curCode |= ReadBits(file, 1, bOk);

		if( !bOk )
			return 0;

		if( m_nMarkerFoundInImageData != 0 )
			return 0;
		
		// Look for the code in the table
		for(; iHC < nHCCount; iHC++)
		{
			if(aTable[iHC].size == curLength)
			{
				if( aTable[iHC].code == curCode )
					return aTable[iHC].value;
			}
			else
			{
				break;
			}

		}
	}
	
	m_lastError = errInvalidHuffmanCode;
	bOk = false;

	return 0;
}

// Update DCT scaled values
// This update includes a multiplication of the DCT needed by the IDCT routine
void Jpeg::BlockInverseQuantization(int32 component, int32* pBlock)
{
	int32* paQT = m_aanQuantizationTables[ m_anQuantizationTableSelectors[component] ];
	for(int i = 0; i < 64; i++)
	{
		pBlock[i] *= paQT[i];
	}
}

const int IDCT_BIT_PRECISION = 11;

// Converts a float number into a N:IDCT_BIT_PRECISION fixed point integer
#define TOFIX_IDCT(f) int32((1<<IDCT_BIT_PRECISION)*f)

// IDCT Algorithm AAN modified Feig
void Jpeg::BlockFastIdct(int32* paDctCoeffs)
{
	// Compute B1 (horizontal / vertical algoritm):
	// (the vertical part is in tensor product)
	
	int32 matr1[64];

	for( int32 k = 0; k < 64; k += 8)
	{
		matr1[k] = paDctCoeffs[k + 0];
		matr1[k + 1] = paDctCoeffs[k + 4];
		
		matr1[k + 2] = matr1[k + 3] = paDctCoeffs[k + 2];
		matr1[k + 2] -= paDctCoeffs[k + 6];
		matr1[k + 3] += paDctCoeffs[k + 6];
		
		int32 coef5 = paDctCoeffs[k + 5];
		matr1[k + 4] = coef5;
		matr1[k + 4] -= paDctCoeffs[k + 3];
		coef5 += paDctCoeffs[k + 3];
		
		matr1[k + 5] = matr1[k + 6] = paDctCoeffs[k + 1];
		matr1[k + 7] = (matr1[k + 5] += paDctCoeffs[k + 7]);
		matr1[k + 6] -= paDctCoeffs[k + 7];
		
		matr1[k + 7] += coef5;
		matr1[k + 5] -= coef5;
	}

	// double fC6   = 2 * sin(pi / 8);
	// double fC4C6 = 2 * sqrt(2) * sin(pi / 8);
	// double fC4   = sqrt(2);
	// double fQ    = 2 * (cos(pi / 8) - sin(pi / 8));
	// double fC4Q  = 2 * sqrt(2) * (cos(pi / 8) - sin(pi / 8));
	// double fR    = 2 * (cos(pi / 8) + sin(pi / 8));
	// double fC4R  = 2 * sqrt(2) * (cos(pi / 8) + sin(pi / 8));

	const int32 C6   = TOFIX_IDCT( 0.7653668647);
	//const int32 C4C6 = TOFIX_IDCT( 1.0823922);
	const int32 C4   = TOFIX_IDCT( 1.414213562);
	const int32 Q    = TOFIX_IDCT( 1.0823922);
	const int32 C4Q  = TOFIX_IDCT( 1.530733729);
	const int32 R    = TOFIX_IDCT( 2.61312593);
	const int32 C4R  = TOFIX_IDCT( 3.69551813);

	int32 tmp, tmp1, tmp2, tmp3, tmp4, tmp6, tmp7;
	int32 co1, co2, co3, co5, co6, co7, co35, co17;
	
	int32 p = 0;
	int32 matr2[64];

	// line 0,	M x M
	tmp4 = (co3 = matr1[24]) - (co5 = matr1[40]);
	tmp6 = (co1 = matr1[8]) - (co7 = matr1[56]);
	tmp = C6 * (tmp6 - tmp4);
	matr2[p++] = matr1[0] << IDCT_BIT_PRECISION;
	matr2[p++] = matr1[32] << IDCT_BIT_PRECISION;
	matr2[p++] = ((co2 = matr1[16]) - (co6 = matr1[48])) * C4;
	matr2[p++] = (co2 + co6) << IDCT_BIT_PRECISION;
	matr2[p++] = Q * tmp4 - tmp;
	matr2[p++] = ((co17 = co1 + co7) - (co35 = co3 + co5)) * C4;
	matr2[p++] = R * tmp6 - tmp;
	matr2[p++] = (co17 + co35) << IDCT_BIT_PRECISION;
	
	// line 1,	M x M
	tmp4 = (co3 = matr1[25]) - (co5 = matr1[41]);
	tmp6 = (co1 = matr1[9]) - (co7 = matr1[57]);
	tmp = C6 * (tmp6 - tmp4);
	matr2[p++] = matr1[1] << IDCT_BIT_PRECISION;
	matr2[p++] = matr1[33] << IDCT_BIT_PRECISION;
	matr2[p++] = ((co2 = matr1[17]) - (co6 = matr1[49])) * C4;
	matr2[p++] = (co2 + co6) << IDCT_BIT_PRECISION;
	matr2[p++] = Q * tmp4 - tmp;
	matr2[p++] = ((co17 = co1 + co7) - (co35 = co3 + co5)) * C4;
	matr2[p++] = R * tmp6 - tmp;
	matr2[p++] = (co17 + co35) << IDCT_BIT_PRECISION;
	
	const int32 ALLBITS = 22;
	const int32 TWO = 1 + IDCT_BIT_PRECISION;

	// line 2,	M x M
	tmp4 = (co3 = matr1[26]) - (co5 = matr1[42]);
	tmp6 = (co1 = matr1[10]) - (co7 = matr1[58]);
	tmp = Q * (tmp6 - tmp4);
	matr2[p++] = C4 * matr1[2];
	matr2[p++] = C4 * matr1[34];
	matr2[p++] = ((co2 = matr1[18]) - (co6 = matr1[50])) << TWO;
	matr2[p++] = C4 * (co2 + co6);
	matr2[p++] = C4Q * tmp4 - tmp;
	matr2[p++] = ((co17 = co1 + co7) - (co35 = co3 + co5)) << TWO;
	matr2[p++] = C4R * tmp6 - tmp;
	matr2[p++] = C4 * (co17 + co35);
	
	// line 3,	M x M
	tmp4 = (co3 = matr1[27]) - (co5 = matr1[43]);
	tmp6 = (co1 = matr1[11]) - (co7 = matr1[59]);
	tmp = C6 * (tmp6 - tmp4);
	matr2[p++] = matr1[3] << IDCT_BIT_PRECISION;
	matr2[p++] = matr1[35] << IDCT_BIT_PRECISION;
	matr2[p++] = ((co2 = matr1[19]) - (co6 = matr1[51])) * C4;
	matr2[p++] = (co2 + co6) << IDCT_BIT_PRECISION;
	matr2[p++] = Q * tmp4 - tmp;
	matr2[p++] = ((co17 = co1 + co7) - (co35 = co3 + co5)) * C4;
	matr2[p++] = R * tmp6 - tmp;
	matr2[p++] = (co17 + co35) << IDCT_BIT_PRECISION;
	
	int32 l0 = 0;
	int32 l1 = 0;
	int32 l2 = 0;
	int32 l3 = 0;

	// line 4,	M x M
	matr2[p++] = matr1[4];
	matr2[p++] = matr1[36];
	matr2[p++] = (co2 = matr1[20]) - (co6 = matr1[52]);
	matr2[p] = co2 + co6;
	l0 = l2 = -(co3 = matr1[28]) + (co5 = matr1[44]);
	p += 2;
	matr2[p] = (co17 = (co1 = matr1[12]) + (co7 = matr1[60])) - (co35 = co3 + co5);
	l3 = -(l1 = co1 - co7);
	p += 2;
	matr2[p++] = co17 + co35;
	
	// line 5,	M x M
	tmp4 = (co3 = matr1[29]) - (co5 = matr1[45]);
	tmp6 = (co1 = matr1[13]) - (co7 = matr1[61]);
	tmp = Q * (tmp6 - tmp4);
	matr2[p++] = C4 * matr1[5];
	matr2[p++] = C4 * matr1[37];
	matr2[p++] = ((co2 = matr1[16 + 5]) - (co6 = matr1[48 + 5])) << TWO;
	matr2[p++] = C4 * (co2 + co6);
	matr2[p++] = C4Q * tmp4 - tmp;
	matr2[p++] = ((co17 = co1 + co7) - (co35 = co3 + co5)) << TWO;
	matr2[p++] = C4R * tmp6 - tmp;
	matr2[p++] = C4 * (co17 + co35);
	
	// line 6,	M x M
	matr2[p++] = matr1[6];
	matr2[p++] = matr1[38];
	matr2[p++] = (co2 = matr1[22]) - (co6 = matr1[54]);
	matr2[p] = co2 + co6;
	l1 += (tmp4 = -(co3 = matr1[30]) + (co5 = matr1[46]));
	l3 += tmp4;
	p += 2;
	matr2[p] = (co17 = (co1 = matr1[14]) + (co7 = matr1[62])) - (co35 = co3 + co5);
	l2 += (tmp6 = co1 - co7);
	l0 -= tmp6;
	p += 2;
	matr2[p++] = co17 + co35;
	
	// line 7,	M x M
	tmp4 = (co3 = matr1[24 + 7]) - (co5 = matr1[40 + 7]);
	tmp6 = (co1 = matr1[8 + 7]) - (co7 = matr1[56 + 7]);
	tmp = C6 * (tmp6 - tmp4);
	matr2[p++] = matr1[7] << IDCT_BIT_PRECISION;
	matr2[p++] = matr1[32 + 7] << IDCT_BIT_PRECISION;
	matr2[p++] = ((co2 = matr1[16 + 7]) - (co6 = matr1[48 + 7])) * C4;
	matr2[p++] = (co2 + co6) << IDCT_BIT_PRECISION;
	matr2[p++] = Q * tmp4 - tmp;
	matr2[p++] = ((co17 = co1 + co7) - (co35 = co3 + co5)) * C4;
	matr2[p++] = R * tmp6 - tmp;
	matr2[p++] = (co17 + co35) << IDCT_BIT_PRECISION;
	
	
	// Completing line 4 and 6,  O = J(NxM)
	const int32 g0 = C4 * (l0 + l1);
	const int32 g1 = C4 * (l0 - l1);
	const int32 g2 = l2 << TWO;
	const int32 g3 = l3 << TWO;
	
	matr2[36] = g0 + g2;
	matr2[38] = g1 + g3;
	matr2[52] = g1 - g3;
	matr2[54] = g2 - g0;
	
	tmp = C6 * (matr2[32] + matr2[48]);
	matr2[32] = -Q * matr2[32] - tmp;
	matr2[48] = R * matr2[48] - tmp;
	
	tmp = C6 * (matr2[33] + matr2[49]);
	matr2[33] = -Q * matr2[33] - tmp;
	matr2[49] = R * matr2[49] - tmp;
	
	tmp = Q * (matr2[34] + matr2[50]);
	matr2[34] = -C4Q * matr2[34] - tmp;
	matr2[50] = C4R * matr2[50] - tmp;
	
	tmp = C6 * (matr2[35] + matr2[51]);
	matr2[35] = -Q * matr2[35] - tmp;
	matr2[51] = R * matr2[51] - tmp;
	
	tmp = Q * (matr2[37] + matr2[53]);
	matr2[37] = -C4Q * matr2[37] - tmp;
	matr2[53] = C4R * matr2[53] - tmp;
	
	tmp = C6 * (matr2[39] + matr2[55]);
	matr2[39] = -Q * matr2[39] - tmp;
	matr2[55] = R * matr2[55] - tmp;
	
	int32 n1, n2, n3;

	for( p = 0; p < 64; p += 8)
	{
		matr1[p] = (tmp4 = (n3 = matr2[p] + matr2[p + 1]) + matr2[p + 3]) + matr2[p + 7];
		matr1[p + 3] = (tmp6 = n3 - matr2[p + 3]) - (tmp7 = matr2[p + 4] -
			(tmp1 = (tmp2 = matr2[p + 6] - matr2[p + 7]) - matr2[p + 5]));
		matr1[p + 4] = tmp6 + tmp7;
		matr1[p + 1] = (tmp3 = (n1 = matr2[p] - matr2[p + 1]) + (n2 = matr2[p + 2] - matr2[p + 3])) + tmp2;
		matr1[p + 5] = (n1 - n2) + tmp1;	// no tmp because of the caching of (n1 -n2)
		matr1[p + 2] = (n1 - n2) - tmp1;
		matr1[p + 6] = tmp3 - tmp2;
		matr1[p + 7] = tmp4 - matr2[p + 7];
	}
	
	
	int32 i;
	for( p = i = 0; p < 64; p += 8, i++)
	{
		paDctCoeffs[p] = ((tmp4 = (n3 = matr1[i] + matr1[8 + i]) + matr1[24 + i]) + matr1[56 + i]) >> ALLBITS;
		paDctCoeffs[p + 3] = ((tmp6 = n3 - matr1[24 + i]) - (tmp7 = matr1[32 + i] -
			(tmp1 = (tmp2 = matr1[48 + i] - matr1[56 + i]) - matr1[40 + i]))) >> ALLBITS;
		paDctCoeffs[p + 4] = (tmp6 + tmp7) >> ALLBITS;
		paDctCoeffs[p + 1] = ((tmp3 = (n1 = matr1[i] - matr1[8 + i]) +
			(n2 = matr1[16 + i] - matr1[24 + i])) + tmp2) >> ALLBITS;
		paDctCoeffs[p + 5] = ((n1 - n2) + tmp1) >> ALLBITS;
		paDctCoeffs[p + 2] = ((n1 - n2) - tmp1) >> ALLBITS;
		paDctCoeffs[p + 6] = (tmp3 - tmp2) >> ALLBITS;
		paDctCoeffs[p + 7] = (tmp4 - matr1[56 + i]) >> ALLBITS;
	}
}

// Converts a float number into a 8:24 fixed point integer
#define TOFIX24(f) int32((1<<24)*f)

void Jpeg::BuildQuantIdctPreMultTable()
{
	static const int32 k_aCosValues[8] = {
		TOFIX24(1.00000000000000000), // cos(pi * 0 / 16.0)
		TOFIX24(0.98078528040323043), // cos(pi * 1 / 16.0)
		TOFIX24(0.92387953251128674), // cos(pi * 2 / 16.0)
		TOFIX24(0.83146961230254524), // cos(pi * 3 / 16.0)
		TOFIX24(0.70710678118654757), // cos(pi * 4 / 16.0)
		TOFIX24(0.55557023301960229), // cos(pi * 5 / 16.0)
		TOFIX24(0.38268343236508984), // cos(pi * 6 / 16.0)
		TOFIX24(0.19509032201612833)  // cos(pi * 7 / 16.0)
	};

	// sqrt(2)
	const int32 sqrt2 = TOFIX24(1.4142135623730950488016887242097);

    int iPos = 0;
	for(int32 j = 0; j < 8; j++)
	{
		for(int i = 0; i < 8; i++)
		{
			int64 value = k_aCosValues[i];
			value *= k_aCosValues[j];

			if( i == 0 && j == 0 )
			{
				value /= 8;
			}
			else if( i == 0 || j == 0 )
			{
				value >>= 24;
				value *= sqrt2;
				value /= 8;
			}
			else
			{
				value /= 4;
			}
			
			value >>= (24 + (24 - IDCT_BIT_PRECISION));

			ms_anQuantIdctPreMultTable[iPos] = int32(value);
			++iPos;
		}
    }
}

void Jpeg::PrepareQuantizationTableForFastIdct(int32* paTable)
{
	for(int i = 0; i < 64; i++)
	{
		paTable[i] *= ms_anQuantIdctPreMultTable[i];
	}
}

// Write RGB value into the buffer pointed by m_pPixelBuffer.
// Those RGB values come from the decoding of one MCU which coordinates are (mcuX, mcuY)
// mcuPixelX : MCU X position in pixels in the destination pixel buffer
// mcuPixelY : MCU Y position in pixels in the destination pixel buffer
void Jpeg::ConvertMcuToRgb(int32 mcuPixelX, int32 mcuPixelY, int32 mcuWidth, int32 mcuHeight)
{
	// TODO : support for component count == 1
	ASSERT(m_nComponentCount == 3);

	uint8* const pPixelBuffer = m_pixels.GetWritePtr();

	// const int32 maxBlockCountX = m_nMaxXSampling;
	// const int32 maxBlockCountY = m_nMaxYSampling;
	//const int32 maxBlockCountLum = m_anXSampling[0] * m_anYSampling[0];
	//const int32 maxBlockCountCb = m_anXSampling[1] * m_anYSampling[1];
	//const int32 maxBlockCountCr = m_anXSampling[2] * m_anYSampling[2];
	
	int32 pixelBufferStartX = mcuPixelX;
	int32 pixelBufferStartY = mcuPixelY;
	
	int32 maxPixelX = mcuWidth;
	if( pixelBufferStartX + mcuWidth >= m_width )
	{
		// Clipping will occur in X
		maxPixelX = m_width - pixelBufferStartX;
	}

	int32 maxPixelY = mcuHeight;
	if( pixelBufferStartY + mcuHeight >= m_height )
	{
		// Clipping will occur in Y
		maxPixelY = m_height - pixelBufferStartY;
	}

	int32 offset = 3 * (pixelBufferStartY * m_width + pixelBufferStartX);
	int32 offsetNextRow = 3 * (m_width - maxPixelX);

	int8 anShiftX[3] = {m_anShiftX[0], m_anShiftX[1], m_anShiftX[2]};
	int8 anShiftY[3] = {m_anShiftY[0], m_anShiftY[1], m_anShiftY[2]};

	const int32 componentCount = m_nComponentCount;

	for(int iPixelY = 0; iPixelY < maxPixelY; iPixelY++)
	{
		for(int iPixelX = 0; iPixelX < maxPixelX; iPixelX++)
		{
			int32 anComps[3];
			for(int iComp = 0; iComp < componentCount; iComp++)
			{
				int indexX = iPixelX >> anShiftX[iComp];
				int32 blockX = indexX / 8;
				indexX = indexX % 8;

				int indexY = iPixelY >> anShiftY[iComp];
				int32 blockY = indexY / 8;
				indexY = indexY % 8;

				int32 coef = indexY * 8 + indexX;
				anComps[iComp] = m_aaapMcuBlocks[iComp][blockY][blockX][coef];
			}
			
			// We remove the 128 scaling so we move from the
			// [-128..+127] range to the [0..255] range

			const int32 y = anComps[0] + 128;
			const int32 u = anComps[1];
			const int32 v = anComps[2];
			
			//int32 rs = y + 1.402 * v;
			//int32 gs = y - 0.34414 * u - 0.71414 * v;
			//int32 bs = y + 1.772 * u;
			
			int32 rs = ( (y << 16) + int32(1.402 * 65536) * v ) >> 16;
			int32 gs = ( (y << 16) - int32(0.34414 * 65536) * u - int32(0.71414 * 65536) * v ) >> 16;
			int32 bs = ( (y << 16) + int32(1.772 * 65536) * u ) >> 16;

			if( rs < 0) rs = 0;
			if( rs > 255) rs = 255;
			if( gs < 0) gs = 0;
			if( gs > 255) gs = 255;
			if( bs < 0) bs = 0;
			if( bs > 255) bs = 255;
			
			pPixelBuffer[offset] = uint8(rs);
			pPixelBuffer[offset + 1] = uint8(gs);
			pPixelBuffer[offset + 2] = uint8(bs);

			offset += 3;
		}
		offset += offsetNextRow;
	}
}

void Jpeg::ReadComment(IFile& file, int32 length)
{
	ASSERT(length >= 0);
	char* pszComment = new char[length + 1];
	file.Read(pszComment, length);
	pszComment[length] = 0;

	delete[] pszComment;
}

String Jpeg::GetLastErrorString() const
{
	switch(m_lastError)
	{
	case Jpeg::errNotAJpegFile:
		return "Not a JPEG file";

	case Jpeg::errForbiddenMarkerFound:
		return "Forbidden marker found : 0x00 and 0xff are forbidden markers";

	case Jpeg::errStartOfImageMarkerFoundTwice:
		return "Start Of Image marker found twice (0xd8 is to be found once only)";

	case Jpeg::errUnsupportedFormatC1:
		return "Unsupported Jpeg Format - 0xc1 = Extended Sequential DCT";
	
	case Jpeg::errUnsupportedFormatC2:
		return "Unsupported Jpeg Format - 0xc2 = Progressive DCT";
	case Jpeg::errUnsupportedFormatC3:
		return "Unsupported Jpeg Format - 0xc3 = Lossless (sequential)";
	case Jpeg::errUnsupportedFormatC5:
		return "Unsupported Jpeg Format - 0xc5 = Differential Sequential DCT";
	case Jpeg::errUnsupportedFormatC6:
		return "Unsupported Jpeg Format - 0xc6 = Differential Progressive DCT";
	case Jpeg::errUnsupportedFormatC7:
		return "Unsupported Jpeg Format - 0xc7 = Differential Lossless (sequential)";
	
	case Jpeg::errUnsupportedSegmentC8:
	case Jpeg::errUnsupportedSegmentDE:
	case Jpeg::errUnsupportedSegmentDF:
	case Jpeg::errUnsupportedSegmentF0_FD:
	case Jpeg::errUnsupportedSegmentBF:
	case Jpeg::errUnsupportedSegment01:
	case Jpeg::errUnsupportedSegment02:
	case Jpeg::errUnsupportedSegmentCC:
		return "Unsupported Segment (cc, c8, de, df, f0 to fd, bf, 01, 02)";

	case Jpeg::errUnsupportedFormatC9:
	case Jpeg::errUnsupportedFormatCA:
	case Jpeg::errUnsupportedFormatCB:
	case Jpeg::errUnsupportedFormatCD:
	case Jpeg::errUnsupportedFormatCE:
	case Jpeg::errUnsupportedFormatCF:
		return "Unsupported Jpeg Format (c9, ca, cb, cd, ce, cf)";
	
	case Jpeg::errUnexpectedTerminationOfRestartInterval:
		return "Unexpected Termination of restart interval";

	case Jpeg::errUnsupportedPlaneSize:
		return "Unsupported Plane Size";

	case Jpeg::errInvalidMarkerInHuffmanData:
		return "Invalid Marker In Huffman Data";

	case Jpeg::errInvalidHuffmanTableClass:
		return "Invalid Huffman Table Class";

	case Jpeg::errInvalidHuffmanTableDestinationIdentifier:
		return "Invalid Huffman Table Destination Identifier";

	case Jpeg::errInvalidHuffmanTable:
		return "Invalid Huffman Table";
	
	case Jpeg::errInvalidHuffmanCode:
		return "Invalid Huffman Code";

	case Jpeg::errUnexpectedEndOfFileWhileReadingHuffmanTable:
		return "Unexpected End Of File While Reading Huffman Table";

	case Jpeg::errInvalidSegmentSize:
		return "Invalid segment size";
	}

	return ImageFormat::GetLastErrorString();
}
