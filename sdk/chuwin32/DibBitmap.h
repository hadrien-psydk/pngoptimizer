///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_DIBBITMAP_H
#define CHUWIN32_DIBBITMAP_H

// Note : a copy of this class is present in the chuwin32 library aswell (but in the chuwin32 namespace)

namespace chuwin32 {

typedef BOOL (WINAPI *PFNAlphaBlend)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);

// ATTENTION !!!!!!!!!!!!!!!!!!!
// Sous Windows 98 le nombre de DIB Bitmaps créées maximum tourne autour de 700
// (Même si les bitmaps sont très petites et qu'elles tiennent *largement* en mémoire)
// Si ce nombre dépasse, la création échoue

class DibBitmap  
{
public:
	HBITMAP m_hBitmap;
	HDC m_hDC;
	uint32* m_pBits;

public:
	bool Create(HDC hWndDC, int nWidth, int nHeight, const uint32* pBits = nullptr);
	void Destroy();
	
	void Blit(HDC hDestDC, int nDestX = 0, int nDestY = 0) const;
	void StretchBlit(HDC hDestDC, int nDestWidth, int nDestHeight, int nDestX = 0, int nDestY = 0) const;

	void BlitPiece(HDC hDestDC, int nDestX, int nDestY, int nPieceX, int nPieceY, int nPieceWidth, int nPieceHeight) const;

	void StretchBlitPiece(HDC hDestDC, int nDestX, int nDestY, int nDestWidth, int nDestHeight,
		int nPieceX, int nPieceY, int nPieceWidth, int nPieceHeight) const;
	
	void AlphaBlit(HDC hDestDC, int nDestX = 0, int nDestY = 0) const;
	void AlphaStretchBlit(HDC hDestDC, int nDestWidth, int nDestHeight, int nDestX = 0, int nDestY = 0) const;

	void Clear(COLORREF col);
	void ClearRect(int nSrcX, int nSrcY, int nSrcWidth, int nSrcHeight, COLORREF col);

	int GetWidth() const { return m_nWidth; }
	int GetHeight() const { return m_nHeight; }

	DibBitmap();
	~DibBitmap();

protected:
	int m_nWidth;
	int m_nHeight;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_DIBBITMAP_H
