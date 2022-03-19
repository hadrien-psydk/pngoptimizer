///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_JPEG_H
#define CHUSTD_JPEG_H

#include "ImageFormat.h"

namespace chustd {

/////////////////////////////////////////////////////////////////
// Manages an image in jpeg (jfif) format
class Jpeg : public ImageFormat
{
public:
	/////////////////////////////////////////////////////////////
	virtual bool LoadFromFile(IFile& file);
	virtual PixelFormat GetPixelFormat() const;
	virtual const Palette& GetPalette() const;

	virtual String GetLastErrorString() const;
	/////////////////////////////////////////////////////////////

	Jpeg();
	virtual ~Jpeg();

	// m_lastError values
	enum
	{
		errNotAJpegFile = firstErrEnumDerived, // Not a jpeg file (mismatch in header)
		errForbiddenMarkerFound,          // 0x00 and 0xff are forbidden markers
		errStartOfImageMarkerFoundTwice,  // 0xd8 is to be found once only
		errUnsupportedFormatC1,				// 0xc1 = Extended Sequential DCT
		errUnsupportedFormatC2,				// 0xc2 = Progressive DCT
		errUnsupportedFormatC3,				// 0xc3 = Lossless (sequential
		errUnsupportedFormatC5,				// 0xc5 = Differential Sequential DCT
		errUnsupportedFormatC6,				// 0xc6 = Differential Progressive DCT
		errUnsupportedFormatC7,				// 0xc7 = Differential Lossless (sequential)
		errUnsupportedSegmentC8,				// Unsupported segment
		errUnsupportedFormatC9,
		errUnsupportedFormatCA,
		errUnsupportedFormatCB,
		errUnsupportedSegmentCC,
		errUnsupportedFormatCD,
		errUnsupportedFormatCE,
		errUnsupportedFormatCF,
		errUnexpectedTerminationOfRestartInterval,
		errUnsupportedSegmentDE,
		errUnsupportedSegmentDF,

		errUnsupportedSegmentF0_FD,

		errUnsupportedSegmentBF,
		errUnsupportedSegment01,
		errUnsupportedSegment02,

		errUnsupportedPlaneSize,
		errInvalidMarkerInHuffmanData,

		errInvalidHuffmanTableClass,
		errInvalidHuffmanTableDestinationIdentifier,
		errInvalidHuffmanTable,
		errInvalidHuffmanCode,

		errUnexpectedEndOfFileWhileReadingHuffmanTable,

		errInvalidSegmentSize
	};

	static bool IsJpeg(IFile& file);

protected:
	void Initialize();

	bool ReadSegment(IFile& file);
	uint8 ReadMarker(IFile& file);
	
	bool ReadSegment_JFIF(IFile& file);
	bool ReadSegment_QuantizationTables(IFile& file);
	bool ReadSegment_StartOfFrame(IFile& file);
	bool ReadSegment_HuffmanTable(IFile& file);
	bool ReadSegment_StartOfScan(IFile& file);
	void ReadCompressedImageData(IFile& file);

	void ReadComment(IFile& file, int32 length);
	void ReadScanHeader(IFile& file, int32 length);

	static void BuildTables();
	static void BuildZigzag2dTo1d();
	static void BuildZigzag1dTo2d();
	static void BuildQuantIdctPreMultTable();

	void DecodeAllMcus(IFile& file);
	void DecodeOneMcu(IFile& file);
	bool BlockDecodeHuffman(IFile& file, int32 component, int32* pBlock);
	void BlockInverseQuantization(int32 component, int32* pBlock);
	void BlockFastIdct(int32* pBlock);
	void ConvertMcuToRgb(int32 mcuPixelX, int32 mcuPixelY, int32 mcuWidth, int32 mcuHeight);

	void PrepareQuantizationTableForFastIdct(int32* paTable);

	// Decode one Huffman encoded byte
	uint8 DecodeHuffman(IFile& file, int32 acdc, int32 component, bool& bOk);
	int   ReadBits(IFile& file, int32 bitCount, bool& bOk);

	// Read ahead optimization for Huffman decoding
	class HuffmanByteReader
	{
	public:
		void  Begin();
		int   ReadByte(IFile& file);
		void  End(IFile& file);
	
	private:
		uint8 m_aBytes[256]; // Read ahead buffer to avoid reading the file byte by byte
		int32 m_nSize; // Size of the array (can be less than the max)
		int32 m_nPos;  // Current position in the array
	};

protected:
	HuffmanByteReader m_huffbytereader;

	///////////////////////////////////////////
	int32 m_aanQuantizationTables[4][64]; // [Table identifier][Element number in the table]

	int8 m_nSamplePrecision; // Number of component bits
	int8 m_nComponentCount;  // Number of components per pixel

	uint8 m_anXSampling[3];  // [Component] Horizontal sampling for a given component
	uint8 m_anYSampling[3];  // [Component] Vertical sampling for a given component

	uint8 m_nMaxXSampling;   // Max sampling value in m_anXSampling
	uint8 m_nMaxYSampling;   // Max sampling value in m_anYSampling

	int8 m_anShiftX[3];		// Precomputed shift value
	int8 m_anShiftY[3];		// Precomputed shift value

	// [component] Scale factors table for each component
	uint8 m_anQuantizationTableSelectors[3];
	
	typedef struct SHuffmanCode
	{
		int size;
		int32 code;
		uint8 value;
	} SHuffmanCode;

	// [Table class (DC or AC)][Table number]
	Array<SHuffmanCode> m_aaaHuffmanTables[2][4];

	// 8x8 block for a given component (like Y, U, V) of a MCU
	int32* m_aaapMcuBlocks[3][4][4]; // [component][blockY][blockX]
	/////////////////////
	// Lecture de bits
	enum{ mustResetMcu = 1, endOfImage = 2};
	uint8 m_nMarkerFoundInImageData;

	uint8 m_nLastByte;
	int32 m_nBitOffset; // [0..7] Shift within the first byte
	/////////////////////

	int32 m_anLastDCValues[3];
	/////////////////////
	
	int16 m_nXDensity; // JFIF
	int16 m_nYDensity; // JFIF

	/////////////////////
	int32 m_nRestartInterval;

	int8 m_nScanComponentCount;
	int8 m_anScanComponentSelector[4];
	int8 m_anScanHuffmanTableSelector[2][4];
			
	int32 m_nExtension;					//	For JFXX

	/////////////////////////////////////////////////
	static int32 ms_anZigzag2dTo1d[64]; // Used for compression
	static int32 ms_anZigzag1dTo2d[64]; // Used for decompression
	static int32 ms_anQuantIdctPreMultTable[64]; // Premultiplication of the scale factors table
};

} // namespace chustd

#endif // ndef CHUSTD_JPEG_H
