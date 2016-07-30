///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_IMAGETYPE_H
#define CHUSTD_IMAGETYPE_H

#include "File.h"
#include "FixArray.h"
#include "Buffer.h"

namespace chustd {

enum PixelFormat
{
	PF_Unknown,
	
	PF_1bppGrayScale,
	PF_2bppGrayScale,
	PF_4bppGrayScale,
	PF_8bppGrayScale,
	PF_16bppGrayScale,

	PF_16bppGrayScaleAlpha,
	PF_32bppGrayScaleAlpha,

	PF_1bppIndexed,
	PF_2bppIndexed,
	PF_4bppIndexed,
	PF_8bppIndexed,

	PF_16bppArgb1555,
	PF_16bppArgb4444,
	PF_16bppRgb555,
	PF_16bppRgb565,

	PF_24bppRgb,
	PF_24bppBgr,

	PF_32bppRgba,
	PF_32bppBgra,

	PF_48bppRgb,

	PF_64bppRgba
};

// Color stores bytes in a compatible way with Surface32
// Native color format is plateform dependant. For instance PF_32bppBgra is the
// best suitable in Windows because it is compatible with DIBs.
struct Color
{
public:
	static Color Black;
	static Color White;
	static Color Red;
	static Color Blue;
	static Color Green;
	static Color Yellow;
	static Color Cyan;
	static Color Magenta;

public:
	inline void ToRgb(uint8& rx, uint8& gx, uint8& bx) const
	{
		uint32 color = value32;

		bx = uint8((color >> 0) & 0x00ff);
		gx = uint8((color >> 8) & 0x00ff);
		rx = uint8((color >> 16) & 0x00ff);
	}

	inline void ToRgb(uint32& rx, uint32& gx, uint32& bx) const
	{
		uint32 color = value32;

		bx = uint32((color >> 0) & 0x00ff);
		gx = uint32((color >> 8) & 0x00ff);
		rx = uint32((color >> 16) & 0x00ff);
	}

	inline void ToRgba(uint8& rx, uint8& gx, uint8& bx, uint8& ax) const
	{
		uint32 color = value32;

		bx = uint8((color >> 0) & 0x00ff);
		gx = uint8((color >> 8) & 0x00ff);
		rx = uint8((color >> 16) & 0x00ff);
		ax = uint8((color >> 24) & 0x00ff);
	}

	inline void ToRgba(uint32& rx, uint32& gx, uint32& bx, uint32& ax) const
	{
		uint32 color = value32;

		bx = ((color >> 0) & 0x00ff);
		gx = ((color >> 8) & 0x00ff);
		rx = ((color >> 16) & 0x00ff);
		ax = ((color >> 24) & 0x00ff);
	}

	Color()
	{
		value32 = 0xff000000;
	}

	Color(uint32 r, uint32 g, uint32 b, uint32 a = 255)
	{
		value32 = (a << 24) | (r << 16) | (g << 8) | b;
	}

	void SetRgb(uint8 rx, uint8 gx, uint8 bx)
	{
		value32 = (value32 & 0xff000000) | (rx << 16) | (gx << 8) | bx;
	}

	void SetRgba(uint8 rx, uint8 gx, uint8 bx, uint8 ax)
	{
		value32 = (ax << 24) | (rx << 16) | (gx << 8) | bx;
	}

	void SetAlpha(uint8 ax)
	{
		value32 = (value32 & 0x00ffffff) | (ax << 24);
	}

	uint8 GetAlpha() const
	{
		return uint8(value32 >> 24);
	}

	bool IsEqualRgb(Color other)
	{
		return (value32 & 0x00ffffff) == (other.value32 & 0x00ffffff);
	}

	Color& operator = (const Color& color)
	{
		value32 = color.value32;
		return *this;
	}

	bool operator == (const Color& color) const
	{
		return (value32 == color.value32);
	}

	// Get the native pixel format. May change depending on the plateform.
	// (Windows = PF_32bppBgra)
	static PixelFormat GetInternalFormat()
	{
		if( k_ePlatformByteOrder == boLittleEndian )
			return PF_32bppBgra;

		return PF_32bppRgba;
	}

	static Color From16bppRgb555(uint16 pixel)
	{
		uint8 r = uint8( (pixel & 0x7c00) >> 10 );
		uint8 g = uint8( (pixel & 0x03e0) >> 5 );
		uint8 b = uint8( (pixel & 0x001f) );

		r = uint8( (r << 3) + ((r >> 2) & 0x07));
		g = uint8( (g << 3) + ((g >> 2) & 0x07));
		b = uint8( (b << 3) + ((b >> 2) & 0x07));

		return Color(r, g, b, 255);
	}
public:
	union
	{
		uint32 value32; // Win32-x86: BGRA in memory, ARGB in register
		struct
		{
			uint8 b, g, r, a;
		};
	};
};

/////////////////////////////
class Palette
{
public:
	Color& operator[](int index) { return m_colors[index]; }
	Color operator[](int index) const { return m_colors[index]; }

	bool  HasNonOpaqueColor() const;
	bool  AllAlphasAreOpaque() const;
	void  SetAlphaFullOpaque();
	int   FindColor(Color col) const;
	int   GetFirstFullyTransparentColor() const;

	Palette() : m_count(0) {}

	bool operator==(const Palette& pal) const { return Memory::Equals(this, &pal, sizeof(Palette)); }

public:
	int32   m_count;       // Number of color in the palette
	Color m_colors[256]; // Colors

public:
	static const Palette Null;
};


/////////////////////////////
struct Point
{
	int x, y;

	Point() : x(0), y(0) {}
	Point(int xa, int ya) : x(xa), y(ya) {}
};

/////////////////////////////
struct Rect
{
	int x1, y1;
	int x2, y2;

	Rect() : x1(0), y1(0), x2(0), y2(0) {}
	Rect(int x1a, int y1a, int x2a, int y2a)
		: x1(x1a), y1(y1a), x2(x2a), y2(y2a) {
	}
	bool IsNull() const {
		return (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0);
	}
	bool IsEmpty() const {
		return ( Width() <= 0 || Height() <= 0 );
	}
	bool IsInside(Point pt) const {
		return (x1 <= pt.x && pt.x < x2) && (y1 <= pt.y && pt.y < y2);
	}
	int  Width() const { return x2 - x1; }
	int  Height() const { return y2 - y1; }
};

inline Point operator-(Point pt1, Point pt2) {
	return Point(pt1.x - pt2.x, pt1.y - pt2.y);
}

/////////////////////////////
class IImage
{
public:
	virtual int32 GetWidth() const = 0;
	virtual int32 GetHeight() const = 0;
	virtual PixelFormat GetPixelFormat() const = 0;
	virtual const Palette& GetPalette() const = 0;
	virtual const Buffer& GetPixels() const = 0;
	virtual ~IImage() {}
	
	///////////////////////////////////////////////////////
	// Implemented if needed in the derived class
	//
	// Returns true if one RGB color or one Grey value is used as a transparent color
	// Note: this does not apply for palettized pictures as the transparent information
	// is already given by the palette structure (in the alpha field for each color).
	virtual bool HasSimpleTransparency() const { return false; }
	virtual uint16 GetGreyTransIndex() const { return 0; }
	virtual void GetTransIndexes(uint16& red, uint16& green, uint16& blue) const	{ red = 0; green = 0; blue = 0; }
};

class ImageFormat;

class AnimFrame : public IImage
{
public:
	// What to do with the destination buffer before rendering the NEXT frame
	enum Disposal : uint8
	{
		DispNone = 0,              // Same as APNG_DISPOSE_OP_NONE
		DispClearToTransBlack = 1, // Same as APNG_DISPOSE_OP_BACKGROUND
		DispRestoreToPrevious = 2, // Same as APNG_DISPOSE_OP_PREVIOUS
		
		DispClearToBkColor, // (GIF) May become deprecated as browsers interpret that as DispClearToTransBlack
	};

	// How to apply THIS frame pixels
	enum Blending : uint8
	{
		BlendSource, // Pixel copy
		BlendOver    // Alpha blending
	};

	virtual int32 GetWidth() const = 0;
	virtual int32 GetHeight() const = 0;

	virtual int32 GetOffsetX() const = 0;
	virtual int32 GetOffsetY() const = 0;

	virtual int32 GetDelayFracNumerator() const = 0;
	virtual int32 GetDelayFracDenominator() const = 0;

	virtual Disposal GetDisposal() const = 0;
	virtual Blending GetBlending() const = 0;

	virtual const Buffer& GetPixels() const = 0;

	//////////////////////////////////
	virtual PixelFormat GetPixelFormat() const;
	virtual bool HasSimpleTransparency() const;
	virtual uint16 GetGreyTransIndex() const;
	virtual void GetTransIndexes(uint16& red, uint16& green, uint16& blue) const;
	virtual const Palette& GetPalette() const;

public:
	AnimFrame(ImageFormat* pOwner)
	{
		m_pOwner = pOwner;
	}

protected:
	 ImageFormat* m_pOwner;
};

/////////////////////////////
// Abstract class for image file type classes (png, gif, jpeg, tga, bmp)
class ImageFormat : public IImage
{
public:
	// Possible errors for m_lastError
	enum
	{
		noErr = 0,		// No error
		ioErr,			// Error accessing to the file
		badFileFormat,	// Bad file format
		notEnoughMemory,
		uncompleteFile,
		firstErrEnumDerived = 0x0100,
	};

	///////////////////////////////////////////////
	static bool IsIndexed(PixelFormat pf);
	static int32 SizeofPixelInBits(PixelFormat epf);
	static int32 ComputeByteWidth(PixelFormat epf, int32 width);
	static bool PackPixels(Buffer& pixels, int width, int height, PixelFormat pixelFormat);
	static bool UnpackPixels(Buffer& pixels, int width, int height, PixelFormat pixelFormat);
	///////////////////////////////////////////////

	friend class ImageLoader;

public:
	bool Load(const chustd::String& filePath);

	virtual int32 GetWidth() const;
	virtual int32 GetHeight() const;
	virtual const Buffer& GetPixels() const;

	void  FreeBuffer();

	int32 GetLastError() const;
	bool  IsIndexed() const;

	ImageFormat();
	virtual ~ImageFormat();

	///////////////////////////////////////////////////////
	// Must be implemented in the derived class
	virtual bool  LoadFromFile(IFile& file) = 0;
	///////////////////////////////////////////////////////

	///////////////////////////////////////////////////////
	// Would be nice to be implemented in the derived class, calling the base version on return
	virtual String GetLastErrorString() const;
	///////////////////////////////////////////////////////

	// Animation
	virtual bool  IsAnimated() const;
	virtual bool  HasDefaultImage() const;
	virtual int32 GetFrameCount() const;
	virtual const AnimFrame* GetAnimFrame(int index) const;
	virtual int32 GetLoopCount() const; // 0 = infinite
	
	///////////////////////////////////////////////////////
	
protected:
	int32 m_width;
	int32 m_height;
	int32 m_lastError; // Last error number

	Buffer m_pixels;
	
protected:
	void FlipVertical();
	void SetAlphaFullOpaque();
};

} // namespace chustd

#endif // ndef CHUSTD_IMAGETYPE_H
