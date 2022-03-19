///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gui.h"

typedef HRESULT (WINAPI *PFN_DwmIsCompositionEnabled)(BOOL* pEnabled);
typedef HRESULT (WINAPI *PFN_DwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
enum DWMWINDOWATTRIBUTE {
	DWMWA_NCRENDERING_ENABLED = 1,
	DWMWA_NCRENDERING_POLICY,
	DWMWA_TRANSITIONS_FORCEDISABLED,
	DWMWA_ALLOW_NCPAINT,
	DWMWA_CAPTION_BUTTON_BOUNDS,
	DWMWA_NONCLIENT_RTL_LAYOUT,
	DWMWA_FORCE_ICONIC_REPRESENTATION,
	DWMWA_FLIP3D_POLICY,
	DWMWA_EXTENDED_FRAME_BOUNDS,
	DWMWA_HAS_ICONIC_BITMAP,
	DWMWA_DISALLOW_PEEK,
	DWMWA_EXCLUDED_FROM_PEEK,
	DWMWA_CLOAK,
	DWMWA_CLOAKED,
	DWMWA_FREEZE_REPRESENTATION,
	DWMWA_LAST 
};

namespace chuwin32 {\

static inline Rect convert(const RECT& rc)
{
	return Rect(rc.left, rc.top, rc.right, rc.bottom);
}

static inline RECT convert(const Rect& rc)
{
	RECT ret;
	ret.left = rc.x1;
	ret.top = rc.y1;
	ret.right = rc.x2;
	ret.bottom = rc.y2;
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////
void Widget::Enable(bool enable)
{
	EnableWindow(m_handle, enable ? TRUE : FALSE);
}

void Widget::SetIcon(const Icon& icon, bool bigIcon)
{
	HICON hIcon = (HICON) icon.m_handle;
	SendMessage(m_handle, WM_SETICON, bigIcon ? ICON_BIG : ICON_SMALL, (LPARAM) hIcon);
}

void Widget::SetFocus()
{
	::SetFocus(m_handle);
}

void Widget::Show(CmdShow cs)
{
	int winCS = 0;
	if( cs == CS_Show )
	{
		winCS = SW_SHOW;
	}
	else if( cs == CS_Hide )
	{
		winCS = SW_HIDE;
	}
	::ShowWindow(m_handle, winCS);
}


////////////////////////////////////////////////////////////////////////////////////////////
chustd::String Widget::GetText() const
{
	int length = GetWindowTextLength(m_handle);
	chustd::String strText;
	GetWindowTextW(m_handle, strText.GetUnsafeBuffer(length), length + 1);
	return strText;
}

////////////////////////////////////////////////////////////////////////////////////////////
int Widget::GetTextInt() const
{
	chustd::String str = GetText();
	int value = 0;
	str.ToInt(value);
	return value;
}

////////////////////////////////////////////////////////////////////////////////////////////
void Widget::CenterWindow(bool avoidHide)
{
	RECT rectParent;
	HWND hParent = GetParent();
	if( hParent == nullptr )
	{
		::GetWindowRect(GetDesktopWindow(), &rectParent);
	}
	else
	{
		Widget parentWindow(hParent);
		Rect rc = parentWindow.GetWindowRect();
		rectParent = convert(rc);
	}
	Rect rect = GetWindowRect();

	const int nParentWidth = rectParent.right - rectParent.left;
	const int nParentHeight = rectParent.bottom - rectParent.top;

	int nWidth = rect.Width();
	int nHeight = rect.Height();

	int nX = rectParent.left + (nParentWidth - nWidth) / 2;
	int nY = rectParent.top + (nParentHeight - nHeight) / 2;

	if( avoidHide )
	{
		// Get the best monitor to display the window
		POINT pt = { nX + nWidth/2, nY };
		HMONITOR hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { 0 };
		mi.cbSize = sizeof(mi);
		if( GetMonitorInfo(hm, &mi) )
		{
			Rect rectAero = GetWindowRectAero();
			int borderLeft = rect.y1 - rectAero.y1;
			int borderTop = rect.x1 - rectAero.x1;
			int borderRight =  (rectAero.x2 - rect.x2);
			int borderBottom =  (rectAero.y2 - rect.y2);

			int minX = mi.rcWork.left + borderLeft;
			int maxX = mi.rcWork.right - nWidth - borderRight;
			int minY = mi.rcWork.top + borderTop;
			int maxY = mi.rcWork.bottom - nHeight - borderBottom;

			if( nX > maxX )
			{
				nX = maxX;
			}
			if( nY > maxY )
			{
				nY = maxY;
			}
			if( nX < minX )
			{
				nX = minX;
			}
			if( nY < minY )
			{
				nY = minY;
			}
		}
	}
	MoveWindow(m_handle, nX, nY, nWidth, nHeight, TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Make the Window fully visible.
// Take into account that annoying behavior of windows 10 where the shadow is part
// of the window rect.
////////////////////////////////////////////////////////////////////////////////////////////
void Widget::EnsureVisible()
{
	Rect rect = GetWindowRect();

	// Get the best monitor to display the window
	RECT winRect = { rect.x1, rect.y1, rect.x2, rect.y2 };
	HMONITOR hm = MonitorFromRect(&winRect, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { 0 };
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfo(hm, &mi))
	{
		return;
	}
	Rect rectAero = GetWindowRectAero();
	int borderLeft = rect.x1 - rectAero.x1;
	int borderTop = rect.y1 - rectAero.y1;
	int borderRight = (rectAero.x2 - rect.x2);
	int borderBottom = (rectAero.y2 - rect.y2);

	const int monWidth = mi.rcWork.right - mi.rcWork.left;
	const int monHeight = mi.rcWork.bottom - mi.rcWork.top;

	int minX = mi.rcWork.left;
	int maxX = mi.rcWork.right;
	int minY = mi.rcWork.top;
	int maxY = mi.rcWork.bottom;

	int width = rectAero.Width();
	int height = rectAero.Height();

	if (width > monWidth)
	{
		width = monWidth;
	}
	if (height > monHeight)
	{
		height = monHeight;
	}

	int x = rectAero.x1;
	int y = rectAero.y1;

	if (rectAero.x1 < minX)
	{
		x = minX;
	}
	if (rectAero.x2 > maxX)
	{
		x = maxX - width;
	}
	if (rectAero.y1 < minY)
	{
		y = minY;
	}
	if (rectAero.y2 > maxY)
	{
		y = maxY - height;
	}
	x = x + borderLeft;
	y = y + borderTop;
	width = width - borderLeft - borderRight;
	height = height - borderTop - borderBottom;
	const int w = rect.Width();
	const int h = rect.Height();
	MoveWindow(m_handle, x, y, width, height, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
void Widget::SetText(const chustd::String& str)
{
	::SetWindowTextW(m_handle, str.GetBuffer());
}

///////////////////////////////////////////////////////////////////////////////
void Widget::SetTextInt(int value)
{
	SetText(chustd::String::FromInt(value));
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	int x, int y, int w, int h, HWND hParent, HMENU hMenu)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	m_handle = CreateWindowExW(
		nExStyle, // extended window style
		pszClassName,   // registered class name
		pszTitle,     // window name
		nStyle,      // window style
		x,    // horizontal position of window
		y,     // vertical position of window
		w, // window width
		h, // window height
		hParent,          // handle to parent or owner window
		hMenu,          // menu handle or child identifier
		hInstance,   // handle to application instance (Windows NT/2000/XP: This value is ignored)
		this           // window-creation data
		);

	return m_handle != 0;
}

bool Widget::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	int x, int y, int w, int h, HWND hParent, int nId)
{
	return CreateEx(nExStyle, pszClassName, pszTitle, nStyle, x, y, w, h, hParent, HMENU( LongToHandle(nId)));
}

bool Widget::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	const RECT& rect, HWND hParent, HMENU hMenu)
{
	return CreateEx(nExStyle, pszClassName, pszTitle, nStyle, 
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hParent, hMenu);
}

bool Widget::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	const RECT& rect, HWND hParent, int nId)
{
	return CreateEx(nExStyle, pszClassName, pszTitle, nStyle, rect, hParent, HMENU( LongToHandle(nId)));
}

//////////////////////
LRESULT CALLBACK Widget::WndProcStatic(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	Widget* pWin = nullptr;

	if( nMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT) lParam;
		pWin = (Widget*) pCreateStruct->lpCreateParams;

		// The pointer must given with the CreateWindow
		ASSERT(pWin);
		pWin->SetHandle(hWnd);
	}
	else
	{
		LONG_PTR nLong = ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pWin = (Widget*) (nLong);
	}

	if( pWin == nullptr )
	{
		// Can occur in very rare cases
		return ::DefWindowProc(hWnd, nMsg, wParam, lParam);
	}

	return pWin->WndProc(nMsg, wParam, lParam);
}

LRESULT Widget::WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(m_handle, nMsg, wParam, lParam);
}

Rect Widget::GetRelativeRect()
{
	// Rect relative to parent
	ASSERT(m_handle);

	Rect rect = GetWindowRect();
	HWND hParent = GetParent();
	if( hParent )
	{
		Widget wndParent = hParent;
		wndParent.ScreenToClient(rect);
	}
	return rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WIDGET_HANDLE Widget::GetParent()
{
	ASSERT(m_handle);
	return ::GetParent(m_handle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Widget::ScreenToClient(Rect& rect)
{
	POINT pt0 = { rect.x1, rect.y1 };
	POINT pt1 = { rect.x2, rect.y2 };

	::ScreenToClient(m_handle, &pt0);
	::ScreenToClient(m_handle, &pt1);

	rect.x1 = pt0.x;
	rect.y1 = pt0.y;
	rect.x2 = pt1.x;
	rect.y2 = pt1.y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Widget::ClientToScreen(RECT& rect)
{
	POINT pt0 = { rect.left, rect.top };
	POINT pt1 = { rect.right, rect.bottom };

	::ClientToScreen(m_handle, &pt0);
	::ClientToScreen(m_handle, &pt1);

	rect.left = pt0.x;
	rect.top = pt0.y;

	rect.right = pt1.x;
	rect.bottom= pt1.y;
}

FONT_HANDLE Widget::GetFont() const
{
	HFONT hFont = (HFONT)::SendMessage(m_handle, WM_GETFONT, 0, 0);
	return hFont;
}
	
void Widget::SetFont(HFONT hFont, bool redraw)
{
	BOOL winRedraw = redraw ? TRUE : FALSE;
	::SendMessage(m_handle, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(winRedraw, 0));
}

bool Widget::SetSameFontThanParent()
{
	WIDGET_HANDLE hParent = GetParent();
	if( !hParent )
		return false;

	Widget wnd = hParent;
	HFONT hFontParent = wnd.GetFont();
	if( !hFontParent )
		return false;

	SetFont(hFontParent);
	return true;
}

int Widget::GetScrollPos(int bar)
{
	return ::GetScrollPos(m_handle, bar);
}

int Widget::SetScrollPos(int bar, int pos, bool redraw)
{
	return ::SetScrollPos(m_handle, bar, pos, BOOL(redraw));
}

bool Widget::IsVisible() const
{
	return IsWindowVisible(m_handle) != FALSE;	
}

bool Widget::IsEnabled() const
{
	return IsWindowEnabled(m_handle) != FALSE;
}

bool Widget::Update()
{
	return ::UpdateWindow(m_handle) != FALSE;
}

int Widget::GetId() const 
{
	return PtrToLong(GetMenu(m_handle));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Rect Widget::GetWindowRect() const
{
	ASSERT(m_handle);
	RECT rect;
	::GetWindowRect(m_handle, &rect);
	return convert(rect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GetWindowRect and MoveWindow do not behave correctly if we provide the executable with a version < 6.0
// Works only when the window is visible, otherwise returns the same rect than GetWindowRect.
///////////////////////////////////////////////////////////////////////////////////////////////////
Rect Widget::GetWindowRectAero() const
{
	ASSERT(m_handle);
	RECT rect;
	::GetWindowRect(m_handle, &rect);

	// Fix the rect that is not correct when the the excutable has a system version < 6.0

	HINSTANCE hDwmapiDll = ::LoadLibraryW(L"dwmapi.dll");
	if( hDwmapiDll )
	{
		BOOL enabled;
		PFN_DwmIsCompositionEnabled DwmIsCompositionEnabled;
		DwmIsCompositionEnabled = (PFN_DwmIsCompositionEnabled) ::GetProcAddress(hDwmapiDll, "DwmIsCompositionEnabled");
		if( DwmIsCompositionEnabled )
		{
			HRESULT hres = DwmIsCompositionEnabled(&enabled);
			if( SUCCEEDED(hres) && enabled )
			{
				PFN_DwmGetWindowAttribute DwmGetWindowAttribute;
				DwmGetWindowAttribute = (PFN_DwmGetWindowAttribute) ::GetProcAddress(hDwmapiDll, "DwmGetWindowAttribute");
				if( DwmGetWindowAttribute ) 
				{
					RECT extendedBounds;
					hres = DwmGetWindowAttribute(m_handle, DWMWA_EXTENDED_FRAME_BOUNDS, &extendedBounds, sizeof(RECT));
					if( SUCCEEDED( hres ) )
					{
						rect = extendedBounds;
					}
				}
			}
		}
		::FreeLibrary(hDwmapiDll);
	} 
	return convert(rect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Rect Widget::GetClientRect() const
{
	ASSERT(m_handle);

	RECT rect;
	::GetClientRect(m_handle, &rect);
	return convert(rect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Makes a link between this object and the OS object
void Widget::SetHandle(WIDGET_HANDLE handle)
{
#ifdef _WIN32
	// We store the object address to get access from the WndProc
	::SetWindowLongPtr(handle, GWLP_USERDATA, LONG_PTR(this));
	m_handle = handle;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Widget::OnCommand(uintptr_t)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace chuwin32
