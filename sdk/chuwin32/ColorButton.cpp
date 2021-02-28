///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ColorButton.h"

#include "gdi.h"

namespace chuwin32 {\

///////////////////////////////////////////////////////////////////////////////
ATOM ColorButton::RegisterClass(const wchar* pszClassName)
{
	WNDCLASSEXW wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProcStatic;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(nullptr);;
	wcex.hIcon			= nullptr;
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= nullptr;
	wcex.lpszClassName	= pszClassName;
	wcex.hIconSm		= nullptr;

	return RegisterClassExW(&wcex);
}

///////////////////////////////////////////////////////////////////////////////
bool ColorButton::Create(const Rect& rc, const Widget* parent, int id)
{
	m_color = Color::Black;

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

	m_handle = CreateWindowExW(0, szClassName, L"", WS_BORDER|WS_TABSTOP|WS_CHILD|WS_VISIBLE,
		rc.x1, rc.y1, rc.x2-rc.x1, rc.y2-rc.y1, parent->GetHandle(), HMENU( LongToHandle(id)), nullptr, this);
	return m_handle != 0;
}

///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK ColorButton::WndProcStatic(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	ColorButton* pWin = nullptr;

	if( nMsg == WM_NCCREATE )
	{
		LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT) lParam;
		pWin = (ColorButton*) pCreateStruct->lpCreateParams;

		// The pointer must given with the CreateWindow
		ASSERT(pWin != nullptr);

		::SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(pWin));
		pWin->m_handle = hWnd;
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

///////////////////////////////////////////////////////////////////////////////
// Returns true if the user validated the new color
///////////////////////////////////////////////////////////////////////////////
static bool DoColorDialog(HWND hWnd, Color& col)
{
	COLORREF acr[16] = { 0xff };

	CHOOSECOLOR cc;
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hWnd;
	cc.hInstance = 0;
	cc.rgbResult = RGB(col.r, col.g, col.b);
	cc.lpCustColors = acr;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;
	cc.lCustData = 0;
	cc.lpfnHook = nullptr;
	cc.lpTemplateName = nullptr;

	if( !::ChooseColorW(&cc) )
	{
		return false;
	}
	Color newCol( GetRValue(cc.rgbResult),
	              GetGValue(cc.rgbResult),
	              GetBValue(cc.rgbResult) );
	col = newCol;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
LRESULT ColorButton::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if( msg == WM_ERASEBKGND )
	{
		return TRUE;
	}
	else if( msg == WM_PAINT )
	{
		PAINTSTRUCT ps;
		DeviceContext dc = BeginPaint(m_handle, &ps);
		Rect rect = GetClientRect();
		dc.FillSolidRect(rect, RGB(m_color.r, m_color.g, m_color.b));
		if( !IsEnabled() )
		{
			// Shows that the control is disabled
			
			// Use the darkened control color for the hatch
			int r = m_color.r;
			int g = m_color.g;
			int b = m_color.b;
			r = max(0, r - 40);
			g = max(0, g - 40);
			b = max(0, b - 40);

			HBRUSH hbr = CreateHatchBrush(HS_BDIAGONAL, RGB(r, g, b));

			HGDIOBJ hOld = dc.SelectObject(hbr);
			dc.FillRect(rect, hbr);
			dc.SelectObject(hOld);
			DeleteObject(hbr);
		}
		EndPaint(m_handle, &ps);
		return TRUE;
	}
	else if( msg == WM_LBUTTONUP )
	{
		if( DoColorDialog(m_handle, m_color) )
		{
			ColorSet.Fire();
		}
		return TRUE;
	}
	else if( msg == WM_DESTROY )
	{
	}
	else if( msg == WM_ENABLE )
	{
		InvalidateRect(m_handle, nullptr, TRUE);
	}
	return DefWindowProc(m_handle, msg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
void ColorButton::SetColor(chustd::Color cr)
{
	m_color = cr;
	InvalidateRect(m_handle, nullptr, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
chustd::Color ColorButton::GetColor() const
{
	return m_color;
}

///////////////////////////////////////////////////////////////////////////////
}
