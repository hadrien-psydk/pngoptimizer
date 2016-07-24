///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_PNG_H
#define CHUSTD_PNG_H

#include "ImageFormat.h"
#include "ChunkedFile.h"

#include "DeflateUncompressor.h"
#include "PtrArray.h"
#include "Buffer.h"

namespace chustd {

///////////////////////////////////////////////////////////
struct PngChunk_IHDR
{
	int32  width;
	int32  height;
	uint8  bitDepth;          // Bit count for each RGB component or for each index
	uint8  colorType;         // Color flags: default: grey, flags: 0x1=palette, 0x2=color, 0x4=alpha
	uint8  compressionMethod; // 0=deflate/inflate (zlib)
	uint8  filterMethod;      // 0=Adaptative: 5 types=(none,sub,up,average,paeth)
	uint8  interlaceMethod;   // 0=no interlace, 1=Adam7

	enum { Name = MAKE32('I','H','D','R') };
	enum { DataSize = 13 }; // Size when serialized

	// colorType flags
	enum { CTF_Grey = 0, CTF_Palette = 1, CTF_Color = 2, CTF_Alpha = 4 };

	PngChunk_IHDR() { Clear(); }
	void Clear()
	{
		chustd::Memory::Zero(this, sizeof(PngChunk_IHDR));
	}
	void SwapBytes()
	{
		width = IFile::Swap32(width);
		height = IFile::Swap32(height);
	}
};

///////////////////////////////////////////////////////////
struct PngChunk_tRNS
{
	uint16  grey;
	uint16  red;
	uint16  green;
	uint16  blue;

	enum { Name = MAKE32('t','R','N','S') };

	PngChunk_tRNS() { Clear(); }

	void Clear()
	{
		grey = 0;
		red = 0;
		green = 0;
		blue = 0;
	}
};

///////////////////////////////////////////////////////////
struct PngChunk_bkGD
{
	uint8   index;
	uint16  grey;
	uint16  red;
	uint16  green;
	uint16  blue;

	PngChunk_bkGD() { Clear(); }

	void Clear()
	{
		index = 0;
		grey = 0;
		red = 0;
		green = 0;
		blue = 0;
	}
};

///////////////////////////////////////////////////////////
struct PngChunk_tEXt
{
	String keyword;
	String data;

	enum { Name = MAKE32('t','E','X','t') };
};

///////////////////////////////////////////////////////////
struct PngChunk_fcTL
{
	int32 sequenceNumber;
	int32 width;
	int32 height;
	int32 offsetX;
	int32 offsetY;
	uint16 delayFracNumerator;
	uint16 delayFracDenominator;
	uint8  disposal; // What to do before rendering the next frame
	uint8  blending;

	enum { Name = MAKE32('f','c','T','L') };
	enum { DataSize = 26 };

	PngChunk_fcTL() { Clear(); }
	void Clear()
	{
		sequenceNumber = 0;
		width = 0;
		height = 0;
		offsetX = 0;
		offsetY = 0;
		delayFracNumerator = 0;
		delayFracDenominator = 100;
		disposal = 0;
		blending = 0;
	}
	void SwapBytes()
	{
		sequenceNumber = IFile::Swap32(sequenceNumber);
		width = IFile::Swap32(width);
		height = IFile::Swap32(height);
		offsetX = IFile::Swap32(offsetX);
		offsetY = IFile::Swap32(offsetY);
		delayFracNumerator = IFile::Swap16(delayFracNumerator);
		delayFracDenominator = IFile::Swap16(delayFracDenominator);
	}
};

///////////////////////////////////////////////////////////
struct PngChunk_pHYs
{
	uint32  pixelsPerUnitX;
	uint32  pixelsPerUnitY;
	uint8   unit; // 0: unknown, 1: meter

	enum { Name = MAKE32('p','H','Y','s') };

	PngChunk_pHYs() { Clear(); }

	void Clear()
	{
		pixelsPerUnitX = 0;
		pixelsPerUnitY = 0;
		unit = 0;
	}
};

struct PngChunk_PLTE { enum { Name = MAKE32('P','L','T','E') }; }; // Palette
struct PngChunk_IDAT { enum { Name = MAKE32('I','D','A','T') }; }; // Pixels
struct PngChunk_IEND { enum { Name = MAKE32('I','E','N','D') }; }; // End of file
struct PngChunk_gAMA { enum { Name = MAKE32('g','A','M','A') }; }; // Gamma
struct PngChunk_bKGD { enum { Name = MAKE32('b','K','G','D') }; }; // Background color
struct PngChunk_acTL { enum { Name = MAKE32('a','c','T','L') }; }; // Animation control
struct PngChunk_fdAT { enum { Name = MAKE32('f','d','A','T') }; }; // Frame data

class ApngFrame;

//////////////////////////////////////////////////////////////////////////////////////
// This class is used to load a PNG picture
class Png : public ImageFormat
{
public:
	// Possible errors for m_lastError
	enum LastError
	{
		errNotAPngFile = firstErrEnumDerived, // Not a png file (mismatch in header)
		errBadHeaderSize,      // Invalid header size
		errBadBitDepth,        // Invalid bit depth (should be 1, 2, 4, 8 or 16)
		errBadColorType,       // Invalid or not supported color format
		errBadPixelFormat,     // Invalid combination of bit depth and color type
		errBadCompressionMethod,
		errBadFilterMethod,    // Invalid or unsupported filtering method
		errBadFilterType,      // Bad filtering type
		errInflateErr,         // Decompression error (zlib)
		errBadInterlaceMethod, // Unsupported interlacing method (Adam7 only)
		errBadPicSize,         // Invalid image size
		errNoIENDFound,        // No IEND chunk found at the end of the stream
		errNotEnoughDataInChunk,
		errIDATFoundBeforeacTL,
		errFrameCountIsZero,
		errBadFrameControlSequenceNumber,
		errBadFrameDataSequenceNumber,
		errIDATNotFound,
		errIDATfcTLNotConsistentWithIHDR,
		erracTLMissing,
		erracTLRepeated,
		errMissingfcTLBeforefdAT,
		errUnexpectedChunk,
		errFrameCountMismatch,
		errFrameCountOutsideRange,
		errImageDataMissing,
		errTextMissingSeparator,
		errTextBadKeywordLength
	};

public:
	/////////////////////////////////////////////////
	virtual bool LoadFromFile(IFile& file);
	virtual PixelFormat GetPixelFormat() const;
	virtual const Palette& GetPalette() const;
	virtual const Buffer& GetPixels() const;
	
	/////////////////////////////////////////////////
	virtual String GetLastErrorString() const;

	/////////////////////////////////////////////////////////////////////////////////////
	static int Read_IHDR(IFile& file, int32 dataSizeof, PngChunk_IHDR& out);
	static PixelFormat GetPixelFormat(const PngChunk_IHDR& pngIHDR);
	static PixelFormat GetPixelFormat(uint8 colorType, uint8 bitDepth);

	/////////////////////////////////////////////////////////////////////////////////////
	virtual bool HasSimpleTransparency() const { return IsChunkFound(cf_tRNS); }
	virtual uint16 GetGreyTransIndex() const { return m_tRNS.grey; }
	virtual void GetTransIndexes(uint16& red, uint16& green, uint16& blue) const
	{ red = m_tRNS.red; green = m_tRNS.green; blue = m_tRNS.blue; }

	/////////////////////////////////////////////////////////////////////////////////////
	bool   HasGamma() const { return IsChunkFound(cf_gAMA); }
	uint32 GetGamma() const { return m_gamma; }

	/////////////////////////////////////////////////////////////////////////////////////
	bool   HasBackgroundColor() const { return IsChunkFound(cf_bKGD); }
	uint8  GetBkIndex() const { return m_bkGD.index; }
	uint16 GetBkGrey() const  { return m_bkGD.grey; }
	uint16 GetBkRed() const   { return m_bkGD.red; }
	uint16 GetBkGreen() const { return m_bkGD.green; }
	uint16 GetBkBlue() const  { return m_bkGD.blue; }

	/////////////////////////////////////////////////////////////////////////////////////
	bool   HasPhysicalPixelDimensions() const { return IsChunkFound(cf_pHYs); }
	uint32 GetPhysPpuX() const { return m_pHYs.pixelsPerUnitX; }
	uint32 GetPhysPpuY() const { return m_pHYs.pixelsPerUnitY; }
	uint8  GetPhysUnit() const { return m_pHYs.unit; }

	bool   HasText() const { return m_tEXts.GetSize() > 0; }
	const Array<PngChunk_tEXt>& GetTexts() const { return m_tEXts; }

	virtual bool IsInterlaced() const { return m_IHDR.interlaceMethod != 0; }

	/////////////////////////////////////////////////////////////////////////////////////
	virtual bool  IsAnimated() const;
	virtual bool  HasDefaultImage() const;
	virtual int32 GetFrameCount() const;
	virtual const AnimFrame* GetAnimFrame(int index) const;
	virtual int32 GetLoopCount() const;
	/////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////
	static String StringFromChunkType(uint32 type);
	static bool IsPng(IFile& file);
	static int32 ComputeInterlacedSize(int32 width, int32 height, int32 sizeofPixelInBits);
	static void GetPassSize(int8 iPass, int32& rowCount, int32& pixelBytesPerRow,
		int32 width, int32 height, int32 sizeofPixelInBits);
	static uint8 PaethPredictor(uint8 a, uint8 b, uint8 c);

	Png();
	virtual ~Png();
	
private:
	// To manage expected chunks in the stream
	enum ChunkFlag
	{
		cf_IHDR = 0x0001,
		cf_PLTE = 0x0002,
		cf_IDAT = 0x0004,
		cf_IEND = 0x0008,
		cf_gAMA = 0x0010,
		cf_tRNS = 0x0020,
		cf_bKGD = 0x0040,
		cf_tEXt = 0x0080,
		cf_acTL = 0x0100,
		cf_fcTL = 0x0200,
		cf_fdAT = 0x0400,
		cf_pHYs = 0x0800,
		
		cf_ALL = 0xffffffff
	};
	uint32 m_expectedChunks;
	uint32 m_currentChunkType;

	uint32 m_foundChunks; // Bit flags of chunks found during decoding

	//////////////////////////////////////////////////////////
	PngChunk_IHDR m_IHDR;
	
	Palette m_PLTE;  // Palette information

	int32 m_sizeofPixel;       // Pixel size in bytes; if nbbpp < 8 => 1 byte
	int32 m_sizeofPixelInBits; // Pixel size in bits
	
	/////////////////////////////////////////////////////////////////////////////////
	// Information needed to process image data
	struct ImageDataInfo
	{
		Buffer* pPixels;
		int32 width;
		int32 height;
		int32 byteWidth; // Size of one row not counting the filter byte
		int32 uncompressedDataSize;

		void Clear();
	};
	ImageDataInfo m_idiCurrent; // Current image data information: default image or current anim frame

	/////////////////////////////////////////////////////////////////////////////////
	PngChunk_tRNS m_tRNS;
	/////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////
	uint32 m_gamma;       // Gamma * 100000
	/////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////
	PngChunk_bkGD m_bkGD;
	PngChunk_pHYs m_pHYs;
	/////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////
	Array<PngChunk_tEXt> m_tEXts; // Non null size if found
	/////////////////////////////////////////////////////////////////////////////////

	int32     m_counter_IDAT;
	int32     m_counter_fdAT;

	DeflateUncompressor m_deflateUncompressor;
	uint32    m_outputOffset;
	ByteArray m_compressedBuffer;

	struct AnimationControl
	{
		int32 frameCount;
		int32 loopCount;
	};
	AnimationControl m_acTL;

	PtrArray<ApngFrame> m_apFrames;
	int32 m_currentSequenceNumber;

private:
	void Initialize();

	bool Handle_IDAT(IFile& file, int32 dataSizeof);
	bool Handle_PLTE(IFile& file, int32 dataSizeof);
	bool Handle_IHDR(IFile& file, int32 dataSizeof);
	bool Handle_gAMA(IFile& file, int32 dataSizeof);
	bool Handle_tRNS(IFile& file, int32 dataSizeof);
	bool Handle_bKGD(IFile& file, int32 dataSizeof);
	bool Handle_tEXt(IFile& file, int32 dataSizeof);
	bool Handle_acTL(IFile& file, int32 dataSizeof);
	bool Handle_fcTL(IFile& file, int32 dataSizeof);
	bool Handle_fdAT(IFile& file, int32 dataSizeof);
	bool Handle_pHYs(IFile& file, int32 dataSizeof);

	bool AllocateImageBuffer(bool interlacedBuffer);
	bool BeginImageDataProcessing();
	bool ProcessImageData(IFile& file, int32 dataSizeof);
	bool EndImageDataProcessing();
	
	bool UnfilterAndUninterlace();
	
	static bool UnfilterBlock(uint8* pBlock, int32 rowCount, int32 pixelBytesPerRow, int32 bytesPerPixel);
	
	bool IsChunkFound(ChunkFlag cf) const { return (m_foundChunks & cf) ? true : false; }
	static bool IsValidBitDepth(uint8 bitDepth);
	static bool IsValidColorType(uint8 colorType);
};

///////////////////////////////////////////////////////////////////////////////
class ApngFrame : public AnimFrame
{
public:
	PngChunk_fcTL m_fctl;
	Buffer        m_pixels;

public:
	ApngFrame(Png* pPng) : AnimFrame(pPng) {}

	virtual int32 GetWidth() const { return m_fctl.width; }
	virtual int32 GetHeight() const { return m_fctl.height; }

	virtual int32 GetOffsetX() const { return m_fctl.offsetX; }
	virtual int32 GetOffsetY() const { return m_fctl.offsetY; }

	virtual int32 GetDelayFracNumerator() const { return m_fctl.delayFracNumerator; }
	virtual int32 GetDelayFracDenominator() const { return m_fctl.delayFracDenominator; }

	virtual Disposal GetDisposal() const { return Disposal(m_fctl.disposal); }
	virtual Blending GetBlending() const { return Blending(m_fctl.blending); }

	virtual const Buffer& GetPixels() const { return m_pixels; }
};

extern const uint8 k_PngSignature[8];

} // namespace chustd;

#endif // ndef CHUSTD_PNG_H
