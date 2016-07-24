///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_WINDOW_H
#define CHUWIN32_WINDOW_H

namespace chuwin32 {

/////////////////////////////
void SetRect(RECT& rect, int x1, int y1, int x2, int y2);
bool IsRectNull(const RECT& rect);
bool IsRectEmpty(const RECT& rect);
bool IsPointInRect(const RECT& rect, POINT pt);
int  RectWidth(const RECT& rect);
int  RectHeight(const RECT& rect);

POINT operator - (POINT pt1, POINT pt2);

/////////////////////////////
class Window
{
public:
	HWND m_hWnd;

public:
	Window() : m_hWnd(0) {}
	Window(HWND hWnd) : m_hWnd(hWnd) {}
	virtual ~Window() {};

	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	void Enable(bool bEnable = true) {	EnableWindow(m_hWnd, bEnable ? TRUE : FALSE); }
	void Disable() {	EnableWindow(m_hWnd, FALSE); }
	void SetIcon(HICON hIcon, bool bBigIcon)
	{
		SendMessage(m_hWnd, WM_SETICON, bBigIcon ? ICON_BIG : ICON_SMALL, (LPARAM) hIcon);
	}

	void SetFocus()
	{
		::SetFocus(m_hWnd);
	}

	void Hide()
	{
		::ShowWindow(m_hWnd, SW_HIDE);
	}

	void Show(int nCmdShow = SW_SHOW)
	{
		::ShowWindow(m_hWnd, nCmdShow);
	}

	void CenterWindow(bool bAvoidHide = false);

	void SetText(const chustd::String& str)
	{
		::SetWindowTextW(m_hWnd, str.GetBuffer());
	}
	void SetTextInt(int value)
	{
		SetText(chustd::String::FromInt(value));
	}
	chustd::String GetText() const;
	int GetTextInt() const;
	RECT GetWindowRect() const;
	RECT GetWindowRectAero() const;
	RECT GetClientRect() const;
	RECT GetRelativeRect();

	HWND GetParent()
	{
		ASSERT(m_hWnd);

		return ::GetParent(m_hWnd);
	}

	void ScreenToClient(RECT& rect)
	{
		POINT pt0 = { rect.left, rect.top };
		POINT pt1 = { rect.right, rect.bottom };

		::ScreenToClient(m_hWnd, &pt0);
		::ScreenToClient(m_hWnd, &pt1);

		rect.left = pt0.x;
		rect.top = pt0.y;

		rect.right = pt1.x;
		rect.bottom= pt1.y;
	}

	void ClientToScreen(RECT& rect)
	{
		POINT pt0 = { rect.left, rect.top };
		POINT pt1 = { rect.right, rect.bottom };

		::ClientToScreen(m_hWnd, &pt0);
		::ClientToScreen(m_hWnd, &pt1);

		rect.left = pt0.x;
		rect.top = pt0.y;

		rect.right = pt1.x;
		rect.bottom= pt1.y;
	}

	HFONT GetFont() const
	{
		HFONT hFont = (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
		return hFont;
	}
	
	void SetFont(HFONT hFont, bool bRedraw = TRUE)
	{
		BOOL bWinRedraw = bRedraw ? TRUE : FALSE;
		::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(bWinRedraw, 0));
	}

	bool SetSameFontThanParent()
	{
		HWND hParent = GetParent();
		if( !hParent )
			return false;

		Window wnd = hParent;
		HFONT hFontParent = wnd.GetFont();
		if( !hFontParent )
			return false;

		SetFont(hFontParent);
		return true;
	}

	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  int x, int y, int w, int h, HWND hParent, HMENU hMenu = 0);
	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  int x, int y, int w, int h, HWND hParent, int nId);
	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  const RECT& rect, HWND hParent, HMENU hMenu = 0);
	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  const RECT& rect, HWND hParent, int nId);

	int GetScrollPos(int nBar)
	{
		return ::GetScrollPos(m_hWnd, nBar);
	}

	int SetScrollPos(int nBar, int nPos, bool bRedraw = false)
	{
		return ::SetScrollPos(m_hWnd, nBar, nPos, BOOL(bRedraw));
	}

	HDC GetDC()
	{
		return ::GetDC(m_hWnd);
	}

	void ReleaseDC(HDC hDC)
	{
		::ReleaseDC(m_hWnd, hDC);
	}

	bool IsVisible() const
	{
		return IsWindowVisible(m_hWnd) != FALSE;	
	}

	bool IsEnabled() const
	{
		return IsWindowEnabled(m_hWnd) != FALSE;
	}

	bool Update()
	{
		return ::UpdateWindow(m_hWnd) != FALSE;
	}

	int GetId() const 
	{
		return PtrToLong(GetMenu(m_hWnd));
	}

protected:
	virtual LRESULT WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

protected:
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace chuwin32

#endif