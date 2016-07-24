///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_TRACECTL_H
#define CHUWIN32_TRACECTL_H

#if !defined(CHUSTD_VERSION)
#error "TraceCtl : you need the chustd library to build this class"
#endif

#include "Window.h" 
#include "DibBitmap.h"

// Class Version : 1.4
// 1.4
// - better thread safetey (corrected a deadlock with Vista)
// - better anti-aliased font rendering
// - return of the dib for one line of text in order to avoid flickering
// 1.3
// - thread safe by avoiding SendMessage in drawing functions
// - no more use of dib
// 1.2
// - chustd instead of dkstd
// 1.1 
// - support for unicode
// - use of the system icon font

namespace chuwin32 {\

///////////////////////////////////////////////////////////////////////////////////////////////////
class TraceCtl : public Window
{
public:
	bool Create(int x, int y, int w, int h, HWND hParentWnd, int id,
	            const chustd::String& emptyText = "");

	struct TextPiece
	{
		chustd::String text;
		chustd::String url;
		COLORREF color;
		int16 minWidthEx; // In ex
		int16 justify;

		enum
		{
			JustifyLeft,
			JustifyRight
		};

		TextPiece() : color(0), minWidthEx(0), justify(0) {}
	};

	void AddText(const chustd::String& text, COLORREF cr = 0);
	void AddLine(const chustd::String& text, COLORREF cr = 0);
	void AddTextAtLine(int32 nLine, const chustd::String& text, COLORREF cr = 0);
	void AddLink(const chustd::String& text, const chustd::String& strUrl);
	void AddText(const TextPiece& tct);

	void ClearLineAt(int32 nLine);
	void ClearLine();
	void Clear();

	void Resize(int cx, int cy);
	void ResetHrzPos();

	void SetFont(const HFONT& hFont);
	const HFONT& GetFont() const;

	TraceCtl();
	virtual ~TraceCtl();

protected:
	class Line
	{
	public:
		chustd::Array<TextPiece> m_pieces;

		TextPiece* AddOrReusePiece();
	};

protected:

	enum DragResult
	{
		dragNone,
		dragMove,
		dragCopy
	};
	virtual int OnLinkDragBegin(const chustd::String& strUrl);
	virtual void OnLinkDoubleClick(const chustd::String& strUrl);

	int GetLineHeight() const { return m_lineHeight; }
	int ComputeLineWidth(const Line& line);
	uint32 GetNewScrollPos32(int nSBType, HWND hScrollBar = 0);

	TextPiece* GetLinkFromPoint(int x, int y, RECT* pLocalRect = null);

	///////////////////////////////////////////////////////////////	
	enum ScrollUpdateFlag
	{
		SUF_UpdatePosition = 1,
		SUF_Redraw = 2
	};
	void UpdateScrollSizeVert(int flags = SUF_UpdatePosition | SUF_Redraw);
	void InitScrollSizeVert();

	void UpdateScrollSizeHorz(int flags = SUF_Redraw);
	void InitScrollSizeHorz();

	void Draw(HDC hDC);
	void Refresh();

	///////////////////////////////////////////////////////
	virtual LRESULT WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

	int OnEraseBkgnd(WPARAM wParam, LPARAM lParam);
	int OnPaint(WPARAM wParam, LPARAM lParam);
	int OnVScroll(WPARAM wParam, LPARAM lParam);
	int OnHScroll(WPARAM wParam, LPARAM lParam);
	int OnSize(WPARAM wParam, LPARAM lParam);
	int OnRButtonDown(WPARAM wParam, LPARAM lParam);
	int OnLButtonDown(WPARAM wParam, LPARAM lParam);
	int OnLButtonUp(WPARAM wParam, LPARAM lParam);
	int OnMouseMove(WPARAM wParam, LPARAM lParam);
	int OnLButtonDblClk(WPARAM wParam, LPARAM lParam);
	int OnKeyDown(WPARAM wParam, LPARAM lParam);
	int OnMouseWheel(WPARAM wParam, LPARAM lParam);
	int OnAddText(WPARAM wParam, LPARAM lParam);
	int OnDestroy(WPARAM wParam, LPARAM lParam);
	///////////////////////////////////////////////////////

private:
	chustd::String m_emptyText; // Text displayed when the content is empty
	HFONT   m_hFont;     // Current font
	LOGFONT m_iconLogFont;
	HFONT   m_hIconFont; // Font used by desktop icons

	chustd::Array<Line> m_lines;

	int m_lineHeight;
	int m_nLargestLineWidth; // Max width in pixels
	int m_nMarginLeft;
	int m_exWidth;;

	bool m_bDragStart;
	TextPiece* m_pDragText;
	POINT m_ptDrag;

	chustd::CriticalSection m_cs;

	static uint32 ms_nWmAddText;

	enum Operation { opAddText, opAddTextAtLine, opAddLink, opClearLineAt, opClearAll };

	struct AddTextInfo
	{
		Operation operation;
		int line;
		TextPiece tp;

		AddTextInfo() : line(-1) {}
	};

	DibBitmap m_dib;

private:
	void DoAddText(const TextPiece& tp, int line = -1);
	void DoClearLineAt(int32 nLine);
	void DoClear();
	void CheckFont();

	int GetTextPieceWidth(HDC hDC, const TextPiece& tp);
	int DrawTextPiece(HDC hDC, int x, int y, const TextPiece& tp);
};

} // namespace chuwin32

#endif // ndef CHUWIN32_TRACECTL_H
