///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gui.h"

using namespace chuwin32;

bool EditBox::Create(const RECT& rc, HWND hParent, int nId)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	int nExStyle = WS_EX_CLIENTEDGE;
	int nStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	
	m_hWnd = ::CreateWindowExW(
		nExStyle,			// extended window style
		WC_EDITW,	// registered class name
		L"",	// window name
		nStyle,	// window style
		rc.left,			// horizontal position of window
		rc.top,			// vertical position of window
		rc.right - rc.left,		// window width
		rc.bottom - rc.top,		// window height
		hParent,		// handle to parent or owner window
		HMENU( LongToHandle(nId)),		// menu handle or child identifier
		hInstance,		// handle to application instance (Windows NT/2000/XP: This value is ignored)
		NULL		// window-creation data
		);

	return m_hWnd != NULL;
}

bool SysLink::Create(const RECT& rc, HWND hParent, int nId)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	int nExStyle = 0;
	int nStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE;

	m_hWnd = CreateWindowExW(
		nExStyle, 
		WC_LINK, 
		L"", 
		nStyle, 
		rc.left,			// horizontal position of window
		rc.top,			// vertical position of window
		rc.right - rc.left,		// window width
		rc.bottom - rc.top,		// window height
		hParent,		// handle to parent or owner window
		HMENU( LongToHandle(nId)),		// menu handle or child identifier
		hInstance,		// handle to application instance (Windows NT/2000/XP: This value is ignored)
		NULL		// window-creation data
		);

	return m_hWnd != NULL;
}

ATOM ColorButton::RegisterClass(const wchar* pszClassName)
{
	WNDCLASSEXW wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProcStatic;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(NULL);;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= pszClassName;
	wcex.hIconSm		= NULL;

	return RegisterClassExW(&wcex);
}

bool ColorButton::Create(const RECT& rc, HWND hParent, int nId)
{
	m_cr = 0;

	static const wchar szClassName[] = L"chuwin32::ColorButton";
	static ATOM atom = 0;
	if( atom == 0 )
	{
		atom = RegisterClass(szClassName);
		if( atom == 0 )
		{
			return false;
		}
	}

	return CreateEx(0, szClassName, L"", WS_BORDER | WS_TABSTOP | WS_CHILD | WS_VISIBLE, rc, hParent, nId);
}

//////////////////////
LRESULT CALLBACK ColorButton::WndProcStatic(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	ColorButton* pWin = nullptr;

	if( nMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT) lParam;
		pWin = (ColorButton*) pCreateStruct->lpCreateParams;

		// The pointer must given with the CreateWindow
		ASSERT(pWin != NULL);

		::SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(pWin));
		pWin->m_hWnd = hWnd;
	}
	else
	{
		LONG_PTR nLong = ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pWin = (ColorButton*) (nLong);
	}
	
	if( pWin == nullptr )
	{
		// Can occur in very rare cases
		return ::DefWindowProc(hWnd, nMsg, wParam, lParam);
	}

	return pWin->WndProc(nMsg, wParam, lParam);
}

LRESULT ColorButton::WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if( nMsg == WM_ERASEBKGND )
	{
		return TRUE;
	}
	else if( nMsg == WM_PAINT )
	{
		PAINTSTRUCT ps;
		DeviceContext dc = BeginPaint(m_hWnd, &ps);
		RECT rect = GetClientRect();
		dc.FillSolidRect(rect, m_cr);
		if( !IsEnabled() )
		{
			// Shows that the control is disabled
			
			// Use the darkened control color for the hatch
			int r = GetRValue(m_cr);
			int g = GetGValue(m_cr);
			int b = GetBValue(m_cr);
			r = max(0, r - 40);
			g = max(0, g - 40);
			b = max(0, b - 40);

			HBRUSH hbr = CreateHatchBrush(HS_BDIAGONAL, RGB(r, g, b));

			HGDIOBJ hOld = dc.SelectObject(hbr);
			dc.FillRect(rect, hbr);
			dc.SelectObject(hOld);
			DeleteObject(hbr);
		}
		EndPaint(m_hWnd, &ps);
		return TRUE;
	}
	else if( nMsg == WM_LBUTTONUP )
	{
		PostMessage( GetParent(), WM_COMMAND, LOWORD( GetId() ), 0);
		return TRUE;
	}
	else if( nMsg == WM_DESTROY )
	{
	}
	else if( nMsg == WM_ENABLE )
	{
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
	return DefWindowProc(m_hWnd, nMsg, wParam, lParam);
}

void ColorButton::SetColor(COLORREF cr)
{
	m_cr = cr;
	InvalidateRect(m_hWnd, NULL, TRUE);
}

COLORREF ColorButton::GetColor() const
{
	return m_cr;
}
