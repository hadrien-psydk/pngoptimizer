///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DibBitmap.h"

/////////////////////////////////////////////////////////////////////////////
using namespace chuwin32;
/////////////////////////////////////////////////////////////////////////////
class RetrieveAlphaBlend
{
private:
	PFNAlphaBlend m_pfnAlphaBlend;
	HMODULE m_hModule;
public:
	PFNAlphaBlend GetAlphaBlendFunction()
	{
		if( m_hModule == nullptr )
		{
			m_hModule = LoadLibraryA("MSIMG32.dll");
			if( m_hModule )
			{
				m_pfnAlphaBlend = (PFNAlphaBlend) GetProcAddress(m_hModule, "AlphaBlend");
			}
		}
		return m_pfnAlphaBlend;
	}

	RetrieveAlphaBlend()
	{
		m_hModule = nullptr;
		m_pfnAlphaBlend = nullptr;
	}
	~RetrieveAlphaBlend()
	{
		if( m_hModule )
		{
			FreeLibrary(m_hModule);
		}
	}
};

//////////////////////////////////////////////////////////////////////
DibBitmap::DibBitmap()
{
	m_nWidth = 0;
	m_nHeight = 0;
	
	m_hBitmap = nullptr;
	m_hDC = nullptr;

	m_pBits = 0;
}

DibBitmap::~DibBitmap()
{
	Destroy();
}

bool DibBitmap::Create(HDC hWndDC, int nWidth, int nHeight, const uint32* pBits/*=nullptr*/)
{
	Destroy();

	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = nWidth;
	bi.bmiHeader.biHeight = -(nHeight);
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0;
    bi.bmiHeader.biXPelsPerMeter = 0;
    bi.bmiHeader.biYPelsPerMeter = 0;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;
		
	m_hBitmap = CreateDIBSection(
		hWndDC,  // handle to device context
		&bi, // pointer to structure containing bitmap size, format, and color data
		DIB_RGB_COLORS,	// color data type indicator: RGB values or palette indices
		(void**) &m_pBits, // pointer to variable to receive a pointer to the bitmap's bit values
		nullptr,	// optional handle to a file mapping object
		0	// offset to the bitmap bit values within the file mapping object
		);
	
	if( m_pBits == nullptr )
	{
		return false;
	}

	// Copie les pixels de l'image vers le dib
	if( pBits )
	{
		const int nImageSizeInBytes = nWidth * nHeight * 4;
		chustd::Memory::Copy(m_pBits, pBits, nImageSizeInBytes);
	}
	
	/////////////////////////
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	// Crée un device context pour notre buffer
	m_hDC = CreateCompatibleDC(hWndDC);
	if( m_hDC == nullptr )
	{
		return false;
	}

	SelectObject(m_hDC, m_hBitmap);

	return true;
}

void DibBitmap::Destroy()
{
	if( m_hDC != 0 )
	{
		BOOL b = DeleteDC(m_hDC) != FALSE;
		ASSERT(b); b;
		m_hDC = 0;
	}

	if( m_hBitmap != 0 )
	{
		BOOL b = DeleteObject(m_hBitmap);
		ASSERT(b); b;
		m_hBitmap = 0;
	}
	
	m_nWidth = 0;
	m_nHeight = 0;
	m_pBits = nullptr;
}

void DibBitmap::BlitPiece(HDC hDestDC, int nDestX, int nDestY,
						   int nPieceX, int nPieceY, int nPieceWidth, int nPieceHeight) const
{
	ASSERT( (nPieceX + nPieceWidth) <= GetWidth());
	ASSERT( (nPieceY + nPieceHeight) <= GetHeight());

	BitBlt(hDestDC,  // handle to destination device context 
		nDestX,      // x-coordinate of destination rectangle's upper-left corner
		nDestY,      // y-coordinate of destination rectangle's upper-left corner
		nPieceWidth,    // width of destination rectangle 
		nPieceHeight,   // height of destination rectangle 
		m_hDC,       // handle to source device context 
		nPieceX,           // x-coordinate of source rectangle's upper-left corner  
		nPieceY,           // y-coordinate of source rectangle's upper-left corner
		SRCCOPY      // raster operation code 
		);
}

void DibBitmap::StretchBlitPiece(HDC hDestDC, int nDestX, int nDestY, int nDestWidth, int nDestHeight,
								  int nPieceX, int nPieceY, int nPieceWidth, int nPieceHeight) const
{
	ASSERT( (nPieceX + nPieceWidth) <= GetWidth());
	ASSERT( (nPieceY + nPieceHeight) <= GetHeight());

	StretchBlt(hDestDC,   // handle to destination device context 
		nDestX,    // x-coordinate of destination rectangle's upper-left corner
		nDestY,     // y-coordinate of destination rectangle's upper-left corner
		nDestWidth,   // width of destination rectangle 
		nDestHeight,  // height of destination rectangle 
		m_hDC,  // handle to source device context 
		nPieceX,            // x-coordinate of source rectangle's upper-left corner  
		nPieceY,            // y-coordinate of source rectangle's upper-left corner
		nPieceWidth,  // width of source rectangle
		nPieceHeight, // height of source rectangle 
		SRCCOPY       // raster operation code 
		);
}


void DibBitmap::Blit(HDC hDestDC, int nDestX /* = 0*/, int nDestY /* = 0*/) const
{
	BitBlt(hDestDC,  // handle to destination device context 
		nDestX,      // x-coordinate of destination rectangle's upper-left corner
		nDestY,      // y-coordinate of destination rectangle's upper-left corner
		GetWidth(),    // width of destination rectangle 
		GetHeight(),   // height of destination rectangle 
		m_hDC,       // handle to source device context 
		0,           // x-coordinate of source rectangle's upper-left corner  
		0,           // y-coordinate of source rectangle's upper-left corner
		SRCCOPY      // raster operation code 
		);
}

void DibBitmap::Clear(COLORREF col)
{
	if( m_pBits == nullptr )
		return;

	int r = GetRValue(col);
	int g = GetGValue(col);
	int b = GetBValue(col);
	int a = 255;

	uint32 nPixel = (a << 24) | (r << 16) | (g << 8) | b;

	const int nPixelCount = GetWidth() * GetHeight();
	uint32* pDest = (uint32*) m_pBits;

	chustd::Memory::Set32(pDest, nPixel, nPixelCount);
}

void DibBitmap::ClearRect(int nSrcX, int nSrcY, int nSrcWidth, int nSrcHeight, COLORREF col)
{
	if( m_pBits == nullptr )
		return;

	int r = GetRValue(col);
	int g = GetGValue(col);
	int b = GetBValue(col);
	int a = 255;

	uint32 nPixel = (a << 24) | (r << 16) | (g << 8) | b;
	const int32 nDibWidth = GetWidth();
	const int32 nDibHeight = GetHeight();

	if( nSrcX < 0 )
	{
		nSrcX = 0;
	}

	if( nSrcY < 0 )
	{
		nSrcY = 0;
	}

	if( nSrcX >= nDibWidth || nSrcY >= nDibHeight )
	{
		return;
	}

	if( nSrcX + nSrcWidth > nDibWidth )
	{
		nSrcWidth = nDibWidth - nSrcX;
	}

	if( nSrcY + nSrcHeight > nDibHeight )
	{
		nSrcHeight = nDibHeight - nSrcY;
	}

	for(int y = 0; y < nSrcHeight; ++y)
	{
		for(int x = 0; x < nSrcWidth; ++x)
		{
			m_pBits[(y + nSrcY) * nDibWidth + (x + nSrcX)] = nPixel;
		}
	}
}

void DibBitmap::StretchBlit(HDC hDestDC, int nDestWidth, int nDestHeight, int nDestX /* = 0*/, int nDestY /* = 0*/) const
{
	StretchBlt(hDestDC,   // handle to destination device context 
		nDestX,    // x-coordinate of destination rectangle's upper-left corner
		nDestY,     // y-coordinate of destination rectangle's upper-left corner
		nDestWidth,   // width of destination rectangle 
		nDestHeight,  // height of destination rectangle 
		m_hDC,  // handle to source device context 
		0,            // x-coordinate of source rectangle's upper-left corner  
		0,            // y-coordinate of source rectangle's upper-left corner
		GetWidth(),  // width of source rectangle
		GetHeight(), // height of source rectangle 
		SRCCOPY       // raster operation code 
		);
}

void DibBitmap::AlphaBlit(HDC hDestDC, int nDestX /* = 0*/, int nDestY /* = 0*/) const
{
	const int nDestWidth = GetWidth();
	const int nDestHeight = GetHeight();

	AlphaStretchBlit(hDestDC, nDestWidth, nDestHeight, nDestX, nDestY);
}

void DibBitmap::AlphaStretchBlit(HDC hDestDC, int nDestWidth, int nDestHeight,
								  int nDestX /* = 0*/, int nDestY /* = 0*/) const
{
	static RetrieveAlphaBlend rab;

	PFNAlphaBlend pfnAlphaBlend = rab.GetAlphaBlendFunction();
	if( pfnAlphaBlend == 0 )
	{
		StretchBlit(hDestDC, nDestWidth, nDestHeight, nDestX, nDestY);
		return;
	}

	BLENDFUNCTION blendFunction;
	blendFunction.BlendOp = AC_SRC_OVER;
	blendFunction.BlendFlags = 0;
	blendFunction.SourceConstantAlpha = 255;
	blendFunction.AlphaFormat = 1; // AC_SRC_ALPHA

	const int nSourceWidth = GetWidth();
	const int nSourceHeight = GetHeight();

	pfnAlphaBlend(
		hDestDC,      // handle to destination DC
		nDestX,       // x-coord of upper-left corner
		nDestY,       // y-coord of upper-left corner
		nDestWidth,   // destination width
		nDestHeight,    // destination height
		m_hDC,        // handle to source DC
		0,            // x-coord of upper-left corner
		0,            // y-coord of upper-left corner
		nSourceWidth, // source width
		nSourceHeight,    // source height
		blendFunction // alpha-blending function
		);
}

