///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TraceCtl.h"
#include "gui.h"

//////////////////////////////////////////////////////////////////////
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////
#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif
//////////////////////////////////////////////////////////////////////
uint32 TraceCtl::ms_nWmAddText = 0;
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
TraceCtl::TextPiece* TraceCtl::Line::AddOrReusePiece()
{
	const int pieceCount = m_pieces.GetSize();
	if( pieceCount == 0 )
	{
		// Add one
		if( !m_pieces.SetSize(1) )
		{
			return null;
		}
	}
	else
	{
		const TextPiece& lastpiece = m_pieces.GetLast();
		if( lastpiece.text.IsEmpty() )
		{
			// Reuse the last piece
		}
		else
		{
			// Add a new piece
			if( m_pieces.Add() < 0 )
			{
				return null;
			}
		}
	}
	return &(m_pieces.GetLast());
}

//////////////////////////////////////////////////////////////////////
TraceCtl::TraceCtl()
{
	m_hWnd = NULL;
	m_hFont = NULL;
	m_hIconFont = NULL;
	m_nMarginLeft = 0;
	m_bDragStart = false;
	m_nLargestLineWidth = 0;
	m_lineHeight = 0;
	m_exWidth = 0;
	memset(&m_iconLogFont, 0, sizeof(m_iconLogFont));
}

TraceCtl::~TraceCtl()
{
	if( m_hIconFont != NULL )
	{
		DeleteObject(m_hIconFont);
		m_hIconFont = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
bool TraceCtl::Create(int x, int y, int width, int height, HWND hParentWnd, int id,
                      const chustd::String& emptyText)
{
	m_emptyText = emptyText;

	static const wchar szClassName[] = L"chuwin32 TraceCtl Window Class";
	
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASSEXW wcex;
	wcex.cbSize         = sizeof(WNDCLASSEX); 
	wcex.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc    = (WNDPROC)WndProcStatic;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = NULL;
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = szClassName;
	wcex.hIconSm        = NULL;

	ATOM atom = RegisterClassExW(&wcex);
	if( atom == 0 )
	{
		return false;
	}

	int style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL;
	int styleEx = WS_EX_TRANSPARENT | WS_EX_STATICEDGE;
	
	static const wchar szWindowName[] = L"TraceCtl";

	if( !CreateEx(styleEx, szClassName, szWindowName, style, x, y, width, height, hParentWnd, id) )
	{
		return false;
	}

	// Create the window message involved in writting text
	if( ms_nWmAddText == 0 )
	{
		ms_nWmAddText = RegisterWindowMessageW(L"TraceCtl_AddText_WindowMessage");
	}

	//////////////////////////////////////////////////
	// Create the font
	CheckFont();

	InitScrollSizeVert();
	InitScrollSizeHorz();

	return true;
}

//////////////////////////////////////////////////////////////////////
bool LogFontEquals(const LOGFONT& a, const LOGFONT& b)
{
	if( a.lfHeight != b.lfHeight )
	{
		return false;
	}
	if( a.lfWeight != b.lfWeight )
	{
		return false;
	}
	if( a.lfItalic != b.lfItalic )
	{
		return false;
	}
	if( wcscmp(a.lfFaceName, b.lfFaceName) != 0 )
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Assign a System font to the TraceCtl and check for System settings 
// update (WM_SETTINGCHANGE is never received)
void TraceCtl::CheckFont()
{
	LOGFONTW lf = { 0 };
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);

	if( m_hFont != NULL )
	{
		if( LogFontEquals(m_iconLogFont, lf) )
		{
			// No change
			return;
		}
		if( m_hIconFont != NULL )
		{
			DeleteObject(m_hIconFont);
			m_hIconFont = NULL;
		}
	}
	m_iconLogFont = lf;
	m_hIconFont = CreateFontIndirect(&lf);
	if( m_hIconFont == NULL )
	{
		SetFont( (HFONT) GetStockObject(DEFAULT_GUI_FONT));
	}
	else
	{
		SetFont(m_hIconFont);
	}
	if( m_lines.IsEmpty() )
	{
		return;
	}
	// Update m_nLargestLineWidth for horizontal scrollbar 
	m_nLargestLineWidth = 0;
	foreach(m_lines, i)
	{
		Line& line = m_lines[i];
		const int width = ComputeLineWidth(line);
		if( width > m_nLargestLineWidth )
		{
			m_nLargestLineWidth = width;
		}
	}

	// Refresh scrollbars and redraw
	Refresh();

}

//////////////////////////////////////////////////////////////////////
LRESULT TraceCtl::WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if( nMsg == TraceCtl::ms_nWmAddText )
	{
		return OnAddText(wParam, lParam);
	}
	switch(nMsg)
	{
	case WM_PAINT:
		return OnPaint(wParam, lParam);
		
	case WM_ERASEBKGND:
		return OnEraseBkgnd(wParam, lParam);

	case WM_VSCROLL:
		return OnVScroll(wParam, lParam);

	case WM_HSCROLL:
		return OnHScroll(wParam, lParam);

	case WM_SIZE:
		return OnSize(wParam, lParam);

	case WM_RBUTTONDOWN:
		return OnRButtonDown(wParam, lParam);

	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);

	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);

	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);

	case WM_KEYDOWN:
		return OnKeyDown(wParam, lParam);
	
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);

	case WM_DESTROY:
		return OnDestroy(wParam, lParam);

	default:
		break;
	}
	return DefWindowProc(m_hWnd, nMsg, wParam, lParam);
}

int TraceCtl::OnKeyDown(WPARAM wParam, LPARAM)
{
	if( wParam == VK_DOWN )
	{
		OnVScroll(SB_LINEDOWN, LPARAM(m_hWnd));
	}
	else if( wParam == VK_UP )
	{
		OnVScroll(SB_LINEUP, LPARAM(m_hWnd));
	}
	else if( wParam == VK_LEFT )
	{
		OnHScroll(SB_LINEUP, LPARAM(m_hWnd));
	}
	else if( wParam == VK_RIGHT )
	{
		OnHScroll(SB_LINEDOWN, LPARAM(m_hWnd));
	}
	else if( wParam == VK_NEXT )
	{
		OnVScroll(SB_PAGEDOWN, LPARAM(m_hWnd));
	}
	else if( wParam == VK_PRIOR )
	{
		OnVScroll(SB_PAGEUP, LPARAM(m_hWnd));
	}
	else if( wParam == VK_HOME )
	{
		if( GetAsyncKeyState(VK_CONTROL) != 0 )
		{
			OnVScroll(SB_TOP, LPARAM(m_hWnd));
		}
		else
		{
			OnHScroll(SB_TOP, LPARAM(m_hWnd));
		}
	}
	else if( wParam == VK_END )
	{
		if( GetAsyncKeyState(VK_CONTROL) != 0 )
		{
			OnVScroll(SB_BOTTOM, LPARAM(m_hWnd));
		}
		else
		{
			OnHScroll(SB_BOTTOM, LPARAM(m_hWnd));
		}
	}

	return 0;
}

int TraceCtl::OnMouseWheel(WPARAM wParam, LPARAM)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

	int32 nPos = GetNewScrollPos32(SB_VERT);
	nPos -= (zDelta * m_lineHeight) / 120;
	SetScrollPos(SB_VERT, nPos, TRUE); // TRUE bRedraw
	OnVScroll(SB_THUMBTRACK, LPARAM(m_hWnd));

	return 0;
}

TraceCtl::TextPiece* TraceCtl::GetLinkFromPoint(int x, int y, RECT* /*pLocalRect*/)
{
	uint32 nScrollStart = GetNewScrollPos32(SB_VERT);
	uint32 nLinePixY = nScrollStart + y;
	int32 nLine = nLinePixY / m_lineHeight;
	
	// Ligne inexistante ?
	if( nLine >= m_lines.GetSize() )
		return 0;

	Line& line = m_lines[nLine];

	// Parcours des morceaux de texte
	HDC hResDC = GetDC();
	SelectObject(hResDC, m_hFont);

	TextPiece* pResult = null;

	int32 nX1 = m_nMarginLeft;
	int32 nX2 = 0;
	foreach(line.m_pieces, iPiece)
	{
		TextPiece& tp = line.m_pieces[iPiece];
		nX2 = nX1 + GetTextPieceWidth(hResDC, tp);

		if( nX1 <= x && x < nX2 )
		{
			// Dedans !
			if( !tp.url.IsEmpty() )
			{
				// Un lien !
				pResult = &tp;

				break;
			}
		}
		nX1 = nX2;
	}
	
	ReleaseDC(hResDC);

	return pResult;
}

int TraceCtl::OnLinkDragBegin(const chustd::String& /*strUrl*/)
{
	return dragNone;
}

void TraceCtl::OnLinkDoubleClick(const chustd::String& /*strUrl*/)
{

}

int TraceCtl::OnLButtonDown(WPARAM, LPARAM lParam)
{
	// On va chercher si on clique sur un lien
	int x = GET_X_LPARAM(lParam); // Inclure <windowsx.h> dans stdafx.h
	int y = GET_Y_LPARAM(lParam); 

	TextPiece* pTP = GetLinkFromPoint(x, y);
	if( pTP != null )
	{
		// Transforme le curseur
		HCURSOR hCurs = LoadCursor(NULL, IDC_HAND);
		SetCursor(hCurs);

		m_bDragStart = true;
		m_ptDrag.x = x;
		m_ptDrag.y = y;
		m_pDragText = pTP;
	}

	return 0;
}

int TraceCtl::OnLButtonUp(WPARAM, LPARAM lParam)
{
	// On va chercher si on clique sur un lien
	int x = GET_X_LPARAM(lParam); // Inclure <windowsx.h> dans stdafx.h
	int y = GET_Y_LPARAM(lParam);

	m_bDragStart = false;
	m_pDragText = null;

	TextPiece* pTP = GetLinkFromPoint(x, y);
	if( pTP != null )
	{
		// Transforme le curseur
		HCURSOR hCurs = LoadCursor(NULL, IDC_HAND);
		SetCursor(hCurs);
	}

	return 0;
}

int TraceCtl::OnLButtonDblClk(WPARAM, LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam); // Inclure <windowsx.h> dans stdafx.h
	int y = GET_Y_LPARAM(lParam);

	TextPiece* pTP = GetLinkFromPoint(x, y);
	if( pTP != null )
	{
		// Transforme le curseur
		HCURSOR hCurs = LoadCursor(NULL, IDC_HAND);
		SetCursor(hCurs);

		OnLinkDoubleClick(pTP->url);
	}
	return 0;
}

int TraceCtl::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	// On va chercher si on clique sur un lien
	int x = GET_X_LPARAM(lParam); // Inclure <windowsx.h> dans stdafx.h
	int y = GET_Y_LPARAM(lParam); 

	if( (wParam & MK_LBUTTON) && m_bDragStart )
	{
		int nDeltaX = chustd::Math::Abs( int32(m_ptDrag.x - x));
		int nDeltaY = chustd::Math::Abs( int32(m_ptDrag.y - y));

		if( nDeltaX > 4 || nDeltaY > 4 )
		{
			COLORREF crPrevious = m_pDragText->color;
			m_pDragText->color = RGB(127, 127, 255);
			
			HDC hDC = GetDC();
			Draw(hDC);
			ReleaseDC(hDC);

			int nDragResult = OnLinkDragBegin(m_pDragText->url);
			
			m_bDragStart = false;
			m_pDragText->color = crPrevious;

			if( nDragResult == dragMove )
			{
				// Le lien n'est plus valide
				m_pDragText->color = RGB(20, 20, 30);
				m_pDragText->url.Empty();
			}
			else
			{
				m_pDragText->color = crPrevious;
			}

			hDC = GetDC();
			Draw(hDC);
			ReleaseDC(hDC);
		}
	}
	else
	{
		m_bDragStart = false;
	}

	TextPiece* pTP = GetLinkFromPoint(x, y);
	if( pTP != null )
	{
		// Transforme le curseur
		HCURSOR hCurs = LoadCursor(NULL, IDC_HAND);
		SetCursor(hCurs);
	}
	return 0;
}

int TraceCtl::OnRButtonDown(WPARAM, LPARAM)
{
	int nControlID = 1000;
	NMHDR nm;
	nm.code = NM_RCLICK; // Inclure <commctrl.h> dans stdafx.h
	nm.hwndFrom = m_hWnd;
	nm.idFrom = nControlID;

	SendMessage( GetParent(), WM_NOTIFY, nControlID, (LPARAM) &nm);
	return 0;
}

void TraceCtl::Draw(HDC hdcWindow)
{
	// One thread at a time can draw
	chustd::TmpLock tmplock(m_cs);

	RECT rectClient = GetClientRect();
	const int nClientHeight = rectClient.bottom - rectClient.top;
	//const int nClientWidth = rectClient.right - rectClient.left;
	
	///////////////////////////////////////////////

	DeviceContext dcDib(m_dib.m_hDC);

	COLORREF colBk = GetSysColor(COLOR_WINDOW);
	dcDib.SelectObject(m_hFont);
	
	// For correct anti-aliasing, do not render any background when drawing the font,
	// we will draw the whole background ourselves
	dcDib.SetBkMode(TRANSPARENT);

	const int nStartX = -GetScrollPos(SB_HORZ);
	
	int y = 0;

	int32 nPos = GetScrollPos(SB_VERT);

	y -= nPos;

	// Find out the start index to begin drawing
	int indexStart = 0;
	int nHeightBefore = -y;
	if( nHeightBefore > 0 )
	{
		indexStart = nHeightBefore / m_lineHeight;
		const int nRndHeight = indexStart * m_lineHeight;
		y += nRndHeight;
	}

	const int lineCount = m_lines.GetSize();

	// m_lineHeight peut être à 0 à l'initialisation du contrôle
	if( m_lineHeight > 0 )
	{
		const int visibleIndexCount = (nClientHeight / m_lineHeight) + 1;
		int indexEnd = indexStart + visibleIndexCount;
		if( indexEnd >= lineCount )
		{
			indexEnd = lineCount - 1;
		}

		for(int iLine = indexStart; iLine <= indexEnd; ++iLine)
		{
			const Line& line = m_lines[iLine];
			
			RECT rectClearLine;
			rectClearLine.top = 0;
			rectClearLine.bottom = m_dib.GetHeight();
			rectClearLine.left = 0;
			rectClearLine.right = rectClient.right - rectClient.left;
			dcDib.FillSolidRect(rectClearLine, colBk);

			int textStartX = nStartX + m_nMarginLeft;
			int nDibX = textStartX;
			int nLineWidth = m_nMarginLeft;

			const int pieceCount = line.m_pieces.GetSize();
			for(int iPiece = 0; iPiece < pieceCount; ++iPiece)
			{
				const TextPiece& tp = line.m_pieces[iPiece];
				//SetBkColor(dcDib.m_hDC, RGB(iPiece * 127+100, 255, 200)); // DEBUG
				int width = DrawTextPiece(dcDib.m_hDC, nDibX, 0, tp);
				if( !tp.url.IsEmpty() )
				{
					/*
					// Underline links ?
					POINT ptOld;
					int nLineY = y + m_lineHeight - 1;
					::MoveToEx(hResDC, x , nLineY, &ptOld);
					::LineTo(hResDC, x + width, nLineY);
					*/
				}
				nDibX += width;
				nLineWidth += width;
			}

			// Blit the line to the window
			m_dib.Blit(hdcWindow, 0, y);

			y += m_lineHeight;
		}
	}

	// Clear the remaining client area
	DeviceContext dcWindow(hdcWindow);
	RECT rectRemaining = rectClient;
	rectRemaining.top = y;
	dcWindow.FillSolidRect(rectRemaining, colBk);

	if( lineCount == 0 || (lineCount == 1 && m_lines[0].m_pieces.GetSize() == 0) )
	{
		// Draw empty text
		SetTextColor(hdcWindow, RGB(0,0,200));
		DrawTextW(hdcWindow, m_emptyText.GetBuffer(), m_emptyText.GetLength(), &rectClient, 
		          DT_CENTER|DT_SINGLELINE|DT_VCENTER);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Computes the width of a TextPiece.
int TraceCtl::GetTextPieceWidth(HDC hDC, const TextPiece& tp)
{
	int tabHints[1] = { m_exWidth * 2 };
	DWORD ret = GetTabbedTextExtent(hDC, tp.text.GetBuffer(), tp.text.GetLength(), 1, tabHints);
	int wret = LOWORD(ret);
	int minWidthPix = tp.minWidthEx * m_exWidth;
	if( wret < minWidthPix )
	{
		wret = minWidthPix;
	}
	return wret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Draws a TextPiece
int TraceCtl::DrawTextPiece(HDC hDC, int x, int y, const TextPiece& tp)
{
	int tabHints[1] = { m_exWidth * 2 };
	SetTextColor(hDC, tp.color);
	int minWidthPix = tp.minWidthEx * m_exWidth;
	int justifyBorder = 0;
	if( tp.justify != 0 )
	{
		DWORD ret = GetTabbedTextExtent(hDC, tp.text.GetBuffer(), tp.text.GetLength(), 1, tabHints);
		int textWidth = LOWORD(ret);
		if( textWidth < minWidthPix )
		{
			// Justification needed
			justifyBorder = minWidthPix - textWidth;
		}
	}
	LONG ret = TabbedTextOut(hDC, x + justifyBorder, y, tp.text.GetBuffer(), tp.text.GetLength(), 1, tabHints, 0);
	int wret = LOWORD(ret);
	if( wret < minWidthPix )
	{
		wret = minWidthPix;
	}
	return wret;
}

int TraceCtl::OnPaint(WPARAM, LPARAM) 
{
	CheckFont();

	PAINTSTRUCT ps;
	HDC hPaintDC = BeginPaint(m_hWnd, &ps);
	Draw(hPaintDC);
	EndPaint(m_hWnd, &ps);
	return 0;
}

void TraceCtl::Refresh()
{
	UpdateScrollSizeVert();
	UpdateScrollSizeHorz();

	HDC hDC = GetDC();
	Draw(hDC);
	ReleaseDC(hDC);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TraceCtl::AddText(const chustd::String& text, COLORREF cr)
{
	AddTextInfo* pAddTextInfo = new AddTextInfo;
	if( pAddTextInfo == null )
	{
		return;
	}
	pAddTextInfo->operation = opAddText;
	pAddTextInfo->tp.text = text;
	pAddTextInfo->tp.color = cr;
	::SendMessage(m_hWnd, ms_nWmAddText, (WPARAM) pAddTextInfo, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TraceCtl::AddText(const TextPiece& tp)
{
	AddTextInfo* pAddTextInfo = new AddTextInfo;
	if( pAddTextInfo == null )
	{
		return;
	}
	pAddTextInfo->operation = opAddText;
	pAddTextInfo->tp = tp;
	::SendMessage(m_hWnd, ms_nWmAddText, (WPARAM) pAddTextInfo, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TraceCtl::AddLine(const chustd::String& text, COLORREF cr)
{
	chustd::String strLine = text + L"\n";
	AddText(strLine, cr);
}

void TraceCtl::AddTextAtLine(int32 line, const chustd::String& text, COLORREF cr)
{
	AddTextInfo* pAddTextInfo = new AddTextInfo;
	if( pAddTextInfo == null )
	{
		return;
	}
	pAddTextInfo->operation = opAddTextAtLine;
	pAddTextInfo->tp.text = text;
	pAddTextInfo->tp.color = cr;
	pAddTextInfo->line = line;
	::SendMessage(m_hWnd, ms_nWmAddText, (WPARAM) pAddTextInfo, 0);
}

void TraceCtl::AddLink(const chustd::String& text, const chustd::String& strUrl)
{
	AddTextInfo* pAddTextInfo = new AddTextInfo;
	if( pAddTextInfo == null )
	{
		return;
	}
	pAddTextInfo->operation = opAddLink;
	pAddTextInfo->tp.text = text;
	pAddTextInfo->tp.url = strUrl;
	pAddTextInfo->tp.color = RGB(0, 0, 255);
	::SendMessage(m_hWnd, ms_nWmAddText, (WPARAM) pAddTextInfo, 0);
}

void TraceCtl::ClearLineAt(int32 line)
{
	AddTextInfo* pAddTextInfo = new AddTextInfo;
	if( pAddTextInfo == null )
	{
		return;
	}
	pAddTextInfo->operation = opClearLineAt;
	pAddTextInfo->line = line;
	::SendMessage(m_hWnd, ms_nWmAddText, (WPARAM) pAddTextInfo, 0);
}

void TraceCtl::ClearLine()
{
	ClearLineAt(-1);
}

void TraceCtl::Clear()
{
	AddTextInfo* pAddTextInfo = new AddTextInfo;
	if( pAddTextInfo == null )
	{
		return;
	}
	pAddTextInfo->operation = opClearAll;
	::SendMessage(m_hWnd, ms_nWmAddText, (WPARAM) pAddTextInfo, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BOOL TraceCtl::OnAddText(WPARAM wParam, LPARAM)
{
	AddTextInfo* pAddTextInfo = (AddTextInfo*) wParam;
	
	// Transfert params
	Operation operation = pAddTextInfo->operation;
	int line = pAddTextInfo->line;
	TextPiece tp = pAddTextInfo->tp;
	delete pAddTextInfo;
	pAddTextInfo = null;

	switch( operation )
	{
	case opAddText:
	case opAddTextAtLine:
	case opAddLink:
		DoAddText(tp, line);
		break;
	case opClearLineAt:
		DoClearLineAt(line);
		break;
	case opClearAll:
		DoClear();
		break;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Adds a piece of text to the document.
// [in] tp          Text piece to add
// [in] lineIndex   Index of the line to update, -1 to append to current line
///////////////////////////////////////////////////////////////////////////////////////////////////
void TraceCtl::DoAddText(const TextPiece& tp, int lineIndex)
{
	if( lineIndex >= 0 )
	{
		m_cs.Enter();
		Line& line = m_lines[lineIndex];

		int32 pieceIndex = line.m_pieces.Add();
		line.m_pieces[pieceIndex] = tp;

		const int width = ComputeLineWidth(line);
		if( width > m_nLargestLineWidth )
		{
			m_nLargestLineWidth = width;
		}

		Refresh();
		m_cs.Leave();
		return;
	}

	chustd::StringArray astr = tp.text.SplitByEndlines();

	chustd::TmpLock tmplock(m_cs);

	if( m_lines.GetSize() == 0 )
	{
		// Il n'y a pas encore de ligne courante (le tableau est vide)
		m_lines.SetSize(1);
	}

	// On reste sur la ligne courante
	const int nStartLine = m_lines.GetSize() - 1;

	const int nCount = astr.GetSize();
	if( nCount > 1 )
	{
		// Agrandit le tableau des lignes
		m_lines.SetSize( m_lines.GetSize() + nCount - 1);
	}
	else
	{

	}

	int32 nScrollStart = GetNewScrollPos32(SB_VERT);

	RECT rectToUpdate;
	::GetClientRect(m_hWnd, &rectToUpdate);
	
	rectToUpdate.top = nStartLine * m_lineHeight - nScrollStart;
	rectToUpdate.bottom = rectToUpdate.top + nCount * m_lineHeight;

	for(int i = 0; i < nCount; ++i)
	{
		Line& line = m_lines[nStartLine + i];

		// Add an empty piece, except if there is already one
		TextPiece* pPiece = line.AddOrReusePiece();
		if( pPiece == null )
		{
			break;
		}

		pPiece->color = tp.color;
		pPiece->text = astr[i];
		if( i == 0 )
		{
			// Min width and justify applies to first piece only
			pPiece->minWidthEx = tp.minWidthEx;
			pPiece->justify = tp.justify;
			pPiece->url = tp.url;
		}

		const int width = ComputeLineWidth(line);
		if( width > m_nLargestLineWidth )
		{
			m_nLargestLineWidth = width;
		}
	}

	UpdateScrollSizeVert();
	UpdateScrollSizeHorz();
	
	InvalidateRect(m_hWnd, NULL, FALSE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TraceCtl::DoClearLineAt(int32 nLine)
{
	m_cs.Enter();
	
	if( m_lines.GetSize() > 0 )
	{
		if( nLine < 0 )
		{
			Line& line = m_lines.GetLast();
			line.m_pieces.Clear();
		}
		else
		{
			Line& line = m_lines[nLine];
			line.m_pieces.Clear();
		}
		Refresh();
	}
	m_cs.Leave();
}

void TraceCtl::DoClear()
{
	m_cs.Enter();

	m_lines.Clear();
	m_nLargestLineWidth = 0;

	Refresh();

	m_cs.Leave();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int TraceCtl::OnDestroy(WPARAM, LPARAM)
{
	m_dib.Destroy();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BOOL TraceCtl::OnEraseBkgnd(WPARAM, LPARAM)
{
	return FALSE;
}

uint32 TraceCtl::GetNewScrollPos32(int nSBType, HWND hScrollBar)
{
    // First determine if the user scrolled a scroll bar control
    // on the window or scrolled the window itself
	HWND hWndScroll;
	if( hScrollBar == NULL )
		hWndScroll = m_hWnd;
	else
		hWndScroll = hScrollBar;
	
	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_TRACKPOS;
	::GetScrollInfo(hWndScroll, nSBType, &info);
	
	uint32 nPos = info.nTrackPos;
	return nPos;
}

int TraceCtl::OnVScroll(WPARAM wParam, LPARAM lParam) 
{
	UINT nSBCode = LOWORD(wParam);
	HWND hScrollBar = (HWND) lParam;

	int nGetPos = GetScrollPos(SB_VERT);
	//const int nBefore = nGetPos;

	int nMinPos = 0;
	int nMaxPos = 0;
	GetScrollRange(m_hWnd, SB_VERT, &nMinPos, &nMaxPos);

	switch(nSBCode)
	{
	case SB_TOP:
		nGetPos = nMinPos;
		break;
	case SB_BOTTOM:
		nGetPos = nMaxPos;
		break;
	case SB_ENDSCROLL:
		break;
	case SB_LINEDOWN:
		nGetPos += m_lineHeight;
		break;
	case SB_LINEUP:
		nGetPos -= m_lineHeight;
		break;
	case SB_PAGEDOWN:
		nGetPos += m_lineHeight * 4;
		break;
	case SB_PAGEUP:
		nGetPos -= m_lineHeight * 4;
		break;
	case SB_THUMBPOSITION:
		nGetPos = GetNewScrollPos32(SB_VERT, hScrollBar);
		break;
	case SB_THUMBTRACK:
		nGetPos = GetNewScrollPos32(SB_VERT, hScrollBar);
		break;
	}

	SetScrollPos(SB_VERT, nGetPos, TRUE); // TRUE bRedraw
	
	//const int nAfter = GetScrollPos(SB_VERT);
	//const int nDelta = nBefore - nAfter;
	
	InvalidateRect(m_hWnd, NULL, FALSE);
	return 0;
}

void TraceCtl::UpdateScrollSizeVert(int flags)
{
	bool updatePosition = (flags & SUF_UpdatePosition) != 0;
	bool redraw = (flags & SUF_Redraw) != 0;

	//int nGetPos = GetScrollPos(SB_VERT);

	RECT rect = GetClientRect();
	const int nClientHeight = rect.bottom - rect.top;

	const int lineCount = m_lines.GetSize();

	SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;

	if( updatePosition )
	{
		si.fMask |= SIF_POS;
	}
	si.nMin = 0;
	si.nMax = lineCount * m_lineHeight;
	si.nPage = nClientHeight;
	si.nPos = si.nMax;
	si.nTrackPos = 0;

	SetScrollInfo(m_hWnd, SB_VERT, &si, redraw);
}

void TraceCtl::InitScrollSizeVert()
{
	RECT rect = GetClientRect();
	const int nClientHeight = rect.bottom - rect.top;

	const int lineCount = m_lines.GetSize();

	SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = lineCount * m_lineHeight;
    si.nPage = nClientHeight;
    si.nPos = 0;
    si.nTrackPos = 0;

	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
}


int TraceCtl::OnHScroll(WPARAM wParam, LPARAM lParam) 
{
	UINT nSBCode = LOWORD(wParam);
	HWND hScrollBar = (HWND) lParam;

	int nGetPos = GetScrollPos(SB_HORZ);
	//const int nBefore = nGetPos;

	int nMinPos = 0;
	int nMaxPos = 0;
	GetScrollRange(m_hWnd, SB_VERT, &nMinPos, &nMaxPos);

	switch(nSBCode)
	{
	case SB_TOP:
		nGetPos = nMinPos;
		break;
	case SB_BOTTOM:
		nGetPos = nMaxPos;
		break;
	case SB_ENDSCROLL:
		break;
	case SB_LINEDOWN:
		nGetPos += m_exWidth;
		break;
	case SB_LINEUP:
		nGetPos -= m_exWidth;
		break;
	case SB_PAGEDOWN:
		nGetPos += 4 * m_exWidth;
		break;
	case SB_PAGEUP:
		nGetPos -= 4 * m_exWidth;
		break;
	case SB_THUMBPOSITION:
		nGetPos = GetNewScrollPos32(SB_HORZ, hScrollBar);
		break;
	case SB_THUMBTRACK:
		nGetPos = GetNewScrollPos32(SB_HORZ, hScrollBar);
		break;
	}

	SetScrollPos(SB_HORZ, nGetPos, TRUE);
	
	//const int nAfter = GetScrollPos(SB_HORZ);
	//const int nDelta = nBefore - nAfter;
	
	InvalidateRect(m_hWnd, NULL, FALSE);
	return 0;
}


void TraceCtl::UpdateScrollSizeHorz(int flags)
{
	bool redraw = (flags & SUF_Redraw) != 0;

	//int nGetPos = GetScrollPos(SB_HORZ);

	RECT rect = GetClientRect();
	const int nClientWidth = rect.right - rect.left;
	
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL; // On ne met pas à jour la position
	si.nMin = 0;
	si.nMax = m_nLargestLineWidth;
	si.nPage = nClientWidth;
	si.nPos = si.nMax;
	si.nTrackPos = 0;

	SetScrollInfo(m_hWnd, SB_HORZ, &si, redraw);
}

void TraceCtl::InitScrollSizeHorz()
{
	RECT rect = GetClientRect();
	const int nClientWidth = rect.right - rect.left;

	SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = m_nLargestLineWidth;
    si.nPage = nClientWidth;
    si.nPos = 0;
    si.nTrackPos = 0;

	SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
}

int TraceCtl::ComputeLineWidth(const Line& line)
{
	HDC hResDC = GetDC();
	SelectObject(hResDC, m_hFont);

	int x = m_nMarginLeft;

	const int pieceCount = line.m_pieces.GetSize();
	for(int iPiece = 0; iPiece < pieceCount; ++iPiece)
	{
		const TextPiece& tp = line.m_pieces[iPiece];
		x += GetTextPieceWidth(hResDC, tp);
	}

	ReleaseDC(hResDC);
	return x;
}

int TraceCtl::OnSize(WPARAM, LPARAM)
{
	//int nType = int(wParam);
	//int cx = LOWORD(lParam);
	//int cy = HIWORD(lParam);
	
	chustd::TmpLock tmplock(m_cs);

	UpdateScrollSizeVert(SUF_Redraw); // Pas de mise à jour de position sinon on se retrouve tout en bas au resize
	UpdateScrollSizeHorz(SUF_Redraw);
	
	HDC hDC = GetDC();
	Draw(hDC);
	ReleaseDC(hDC);

	return 0;
}

void TraceCtl::Resize(int cx, int cy)
{
	MoveWindow(m_hWnd, 0, 0, cx, cy, TRUE);
}

void TraceCtl::SetFont(const HFONT& hFont)
{
	m_cs.Enter();

	m_hFont = hFont;

	HDC hDC = GetDC();

	SelectObject(hDC, m_hFont);
	
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);

	m_lineHeight = tm.tmHeight;
	m_nMarginLeft = tm.tmAveCharWidth / 2;
	m_exWidth = tm.tmAveCharWidth;

	// Recreate the dib for off line text rendering
	int nMaxWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);

	m_dib.Destroy();
	m_dib.Create(hDC, nMaxWidth, m_lineHeight);

	ReleaseDC(hDC);

	m_cs.Leave();
}

const HFONT& TraceCtl::GetFont() const 
{
	return m_hFont;
}

void TraceCtl::ResetHrzPos()
{
	m_cs.Enter();

	if( m_lines.GetSize() > 0 )
	{
		m_lines.GetLast().m_pieces.SetSize(0);
	}

	m_cs.Leave();
}

