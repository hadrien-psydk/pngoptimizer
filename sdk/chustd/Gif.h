///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_GIF_H
#define CHUSTD_GIF_H

#include "ImageFormat.h"
#include "PtrArray.h"

namespace chustd {

class Gif;

class GifAnimFrame : public AnimFrame
{
public:
	virtual int32 GetWidth() const { return m_width; }
	virtual int32 GetHeight() const { return m_height; }
	virtual const Palette& GetPalette() const;

	virtual int32 GetOffsetX() const { return m_offsetX; }
	virtual int32 GetOffsetY() const { return m_offsetY; }

	virtual int32 GetDelayFracNumerator() const { return m_delayCs; }
	virtual int32 GetDelayFracDenominator() const { return 100; }

	virtual Disposal GetDisposal() const { return Disposal(m_disposal); }
	virtual Blending GetBlending() const { return BlendOver; }

	virtual const Buffer& GetPixels() const { return m_pixels; }

public:
	uint16 m_width;
	uint16 m_height;
	uint16 m_offsetX;
	uint16 m_offsetY;
	uint16 m_delayCs;  // Centi-seconds
	uint8  m_disposal; // To be done after rendering this frame

	uint8  m_clearIndex; // In palette

	Buffer    m_pixels;
	Palette*  m_pPalette;

	bool  m_interlaced;
	int16 m_transparentIndex;

public:
	GifAnimFrame(Gif* pGif);
	virtual ~GifAnimFrame();
};


// Manages an image in Gif format
class Gif : public ImageFormat
{
public:
	/////////////////////////////////////////////////////////////
	virtual bool LoadFromFile(IFile& file);
	virtual PixelFormat GetPixelFormat() const;
	virtual const Palette& GetPalette() const;

	virtual String GetLastErrorString() const;
	/////////////////////////////////////////////////////////////

	virtual bool IsInterlaced() const;

	virtual bool  IsAnimated() const;
	virtual int32 GetFrameCount() const;
	virtual const AnimFrame* GetAnimFrame(int index) const;
	virtual int32 GetLoopCount() const;

	Gif();
	virtual ~Gif();

	enum
	{
		errNotAGifFile = firstErrEnumDerived, // Not a Gif file (mismatch in header)
		errBadEntryInStringTable,
		errBadGifVersion,
		errBlockTerminatorNotFound,
		errBadLzwMinimumCodeSize,
		errBadImageRect
	};

	static bool IsGif(IFile& file);
	

protected:
	int32  m_outputIndex;

	// A Local Color Table supersedes a Global Color
	// The global color table is used if no local color table is found
	Palette m_globalColorTable;
	
	enum Version
	{
		gifverNotSet = 0,
		gifver87a = 1,
		gifver89a = 2
	};

	Version m_version;
	
	uint8 m_backgroundColorIndex;
	
	enum GifDisposalMethod
	{
		gdmNotSpecified = 0,
		gdmDoNotDispose = 1,
		gdmRestoreToBkColor = 2,
		gdmRestoreToPrevious = 3
	};

	int16  m_transparentColorIndex; // Transparent color index for next frame
	bool   m_transparencyUsed;      // true if one of the frames use transparency
	uint8  m_disposalMethod;        // Disposal method for next frame
	uint16 m_delayTime;             // Delay time for next frame in 100th of seconds
	
	///////////////////////////////////////////////
	int32 m_loopCount;    // Animated Gif. 0 = infinite
	
	///////////////////////////////////////////////
	// Decoding
	struct LzwCode
	{
		int32 length;
		int16 previous;
		uint8 value;
	};

	LzwCode m_lzwStringTable[4096];
	int32 m_nextEntry;

	PtrArray<GifAnimFrame> m_apFrames;

protected:
	void Initialize();

	bool ReadLogicalScreenDescriptor(IFile& file);
	
	bool ReadExtensionBlock(IFile& file);
	bool ReadImageDescriptor(IFile& file, GifAnimFrame* pFrame);
	bool ReadImageData(IFile& file, GifAnimFrame* pFrame);
	bool UncompressLzw(IFile& file, uint8* pPixelBuffer, bool& terminatorFound);

	int   InitTable(int codeSize);
	uint8 OutputString(int index, uint8* pPixelBuffer);
	int   ReadBits(uint8* pBuffer, int32 length, int32& bitPosition);
	bool  UninterlaceBuffer(GifAnimFrame* pFrame);

	bool ReadTerminator(IFile& file);
	bool ReadColorTable(IFile& file, Palette& pal);

	void CopyFramePixels(const GifAnimFrame* pFrame, bool bRestore);
	uint8 GetClearIndex(int32 frameIndex);

	static AnimFrame::Disposal GenDisposalFromGifDisposal(uint8 gifDisposal);
};

} // namespace chustd

#endif // ndef CHUSTD_GIF_H

