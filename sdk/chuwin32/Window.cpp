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

	void SetRect(RECT& rect, int x1, int y1, int x2, int y2)
{
	rect.left = x1;
	rect.top = y1;
	rect.right = x2;
	rect.bottom = y2;
}
bool IsRectNull(const RECT& rect)
{
	if( rect.left == 0 && rect.top == 0 && rect.right == 0 && rect.bottom == 0 )
		return true;

	return false;
}

bool IsRectEmpty(const RECT& rect)
{
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;
	if( nWidth <= 0 || nHeight <= 0 )
		return true;

	return false;
}

int RectWidth(const RECT& rect)
{
	return rect.right - rect.left;
}

int RectHeight(const RECT& rect)
{
	return rect.bottom - rect.top;
}

bool IsPointInRect(const RECT& rect, POINT pt)
{
	if( rect.left <= pt.x && pt.x < rect.right )
	{
		if( rect.top <= pt.y && pt.y < rect.bottom )
		{
			return true;
		}
	}
	return false;
}

POINT operator - (POINT pt1, POINT pt2)
{
	POINT pt;
	pt.x = pt1.x - pt2.x;
	pt.y = pt1.y - pt2.y;
	return pt;
}

////////////////////////////////////////////////////////////////////////////////////////////
chustd::String Window::GetText() const
{
	int nLength = GetWindowTextLength(m_hWnd);
	chustd::String strText;

	GetWindowTextW(m_hWnd, strText.GetUnsafeBuffer(nLength), nLength + 1);

	return strText;
}

////////////////////////////////////////////////////////////////////////////////////////////
int Window::GetTextInt() const
{
	chustd::String str = GetText();
	int value = 0;
	str.ToInt(value);
	return value;
}

////////////////////////////////////////////////////////////////////////////////////////////
void Window::CenterWindow(bool bAvoidHide)
{
	RECT rectParent;
	HWND hParent = GetParent();
	if( hParent == NULL )
	{
		::GetWindowRect(GetDesktopWindow(), &rectParent);
	}
	else
	{
		Window parentWindow(hParent);
		rectParent = parentWindow.GetWindowRect();
	}
	RECT rect = GetWindowRect();

	const int nParentWidth = rectParent.right - rectParent.left;
	const int nParentHeight = rectParent.bottom - rectParent.top;

	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;

	int nX = rectParent.left + (nParentWidth - nWidth) / 2;
	int nY = rectParent.top + (nParentHeight - nHeight) / 2;

	if( bAvoidHide )
	{
		// Get the best monitor to display the window
		POINT pt = { nX + nWidth/2, nY };
		HMONITOR hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { 0 };
		mi.cbSize = sizeof(mi);
		if( GetMonitorInfo(hm, &mi) )
		{
			RECT rectAero = GetWindowRectAero();
			int borderLeft = rect.top - rectAero.top;
			int borderTop = rect.left - rectAero.left;
			int borderRight =  (rectAero.right - rect.right);
			int borderBottom =  (rectAero.bottom - rect.bottom);

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
	MoveWindow(m_hWnd, nX, nY, nWidth, nHeight, TRUE);
}

bool Window::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	int x, int y, int w, int h, HWND hParent, HMENU hMenu)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	m_hWnd = CreateWindowExW(
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

	return m_hWnd != 0;
}

bool Window::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	int x, int y, int w, int h, HWND hParent, int nId)
{
	return CreateEx(nExStyle, pszClassName, pszTitle, nStyle, x, y, w, h, hParent, HMENU( LongToHandle(nId)));
}

bool Window::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	const RECT& rect, HWND hParent, HMENU hMenu)
{
	return CreateEx(nExStyle, pszClassName, pszTitle, nStyle, 
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hParent, hMenu);
}

bool Window::CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
	const RECT& rect, HWND hParent, int nId)
{
	return CreateEx(nExStyle, pszClassName, pszTitle, nStyle, rect, hParent, HMENU( LongToHandle(nId)));
}

//////////////////////
LRESULT CALLBACK Window::WndProcStatic(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	Window* pWin = null;

	if( nMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT) lParam;
		pWin = (Window*) pCreateStruct->lpCreateParams;

		// The pointer must given with the CreateWindow
		ASSERT(pWin != NULL);

		::SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(pWin));
		pWin->m_hWnd = hWnd;
	}
	else
	{
		LONG_PTR nLong = ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pWin = (Window*) (nLong);
	}

	if( pWin == null )
	{
		// Can occur in very rare cases
		return ::DefWindowProc(hWnd, nMsg, wParam, lParam);
	}

	return pWin->WndProc(nMsg, wParam, lParam);
}

LRESULT Window::WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(m_hWnd, nMsg, wParam, lParam);
}

RECT Window::GetRelativeRect()
{
	// Rect relative to parent
	ASSERT(m_hWnd);

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	HWND hParent = GetParent();
	if( hParent )
	{
		Window wndParent = hParent;
		wndParent.ScreenToClient(rect);
	}
	return rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RECT Window::GetWindowRect() const
{
	ASSERT(m_hWnd);
	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	return rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GetWindowRect and MoveWindow do not behave correctly if we provide the executable with a version < 6.0
///////////////////////////////////////////////////////////////////////////////////////////////////
RECT Window::GetWindowRectAero() const
{
	ASSERT(m_hWnd);
	RECT rect;
	::GetWindowRect(m_hWnd, &rect);

	// Fix the rect that is not correct when the the excutable has a system version < 6.0

	HINSTANCE hDwmapiDll = ::LoadLibraryW(L"dwmapi.dll");
	if( hDwmapiDll != NULL )
	{
		BOOL enabled;
		PFN_DwmIsCompositionEnabled DwmIsCompositionEnabled;
		DwmIsCompositionEnabled = (PFN_DwmIsCompositionEnabled) ::GetProcAddress(hDwmapiDll, "DwmIsCompositionEnabled");
		if( DwmIsCompositionEnabled != NULL )
		{
			HRESULT hres = DwmIsCompositionEnabled(&enabled);
			if( SUCCEEDED(hres) && enabled )
			{
				PFN_DwmGetWindowAttribute DwmGetWindowAttribute;
				DwmGetWindowAttribute = (PFN_DwmGetWindowAttribute) ::GetProcAddress(hDwmapiDll, "DwmGetWindowAttribute");
				if( DwmGetWindowAttribute != NULL ) 
				{
					RECT extendedBounds;
					hres = DwmGetWindowAttribute(m_hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &extendedBounds, sizeof(RECT));
					if( SUCCEEDED( hres ) )
					{
						rect = extendedBounds;
					}
				}
			}
		}
		::FreeLibrary(hDwmapiDll);
	} 
	return rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RECT Window::GetClientRect() const
{
	ASSERT(m_hWnd);

	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	return rect;
}


} // namespace chuwin32