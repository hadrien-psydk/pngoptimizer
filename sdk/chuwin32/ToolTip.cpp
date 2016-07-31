///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTip.h"

using namespace chuwin32;

///////////////////////////////////////////////////////////////////////////////
ToolTip::ToolTip()
{
	m_bVisible = false;
}

ToolTip::~ToolTip()
{

}

bool ToolTip::Create(HWND hwndParent)
{
	INITCOMMONCONTROLSEX icex;

    // Load the ToolTip class from the DLL.
    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_BAR_CLASSES;

    if( !InitCommonControlsEx(&icex) )
    {
		return false;
	}

	HINSTANCE hInstance = GetModuleHandle(NULL);

	m_handle = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            hwndParent, NULL, hInstance,
                            NULL);

	if( m_handle == NULL )
	{
		return false;
	}

	SetWindowPos(m_handle, HWND_TOPMOST,0, 0, 0, 0,
				 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Prepare TOOLINFO structure for use as tracking ToolTip.
	chustd::Memory::Zero(&m_ti, sizeof(m_ti));
	m_ti.cbSize = sizeof(m_ti);
	m_ti.uFlags = TTF_SUBCLASS;
	//m_ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	//m_ti.uFlags = TTF_TRACK;
	m_ti.hwnd   = hwndParent;
	m_ti.uId    = 0;
	m_ti.hinst  = hInstance;
	m_ti.lpszText  = L"onk----------------";
	m_ti.rect.left = m_ti.rect.top = m_ti.rect.bottom = m_ti.rect.right = 0; 
	m_ti.rect.bottom = m_ti.rect.right = 20; 

	//Window wndParent(hwndParent);
	//m_ti.rect = wndParent.GetClientRect();

	// Add the tool to the control, displaying an error if needed.
	if( !SendMessage(m_handle, TTM_ADDTOOL, 0, (LPARAM)&m_ti) )
	{
		return false;
	}

	// Activate (display) the tracking ToolTip. Then, set a global
	// flag value to indicate that the ToolTip is active, so other
	// functions can check to see if it's visible.
	//SendMessage(m_handle, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&m_ti);
	//g_bIsVisible = TRUE;

	return true;
}

void ToolTip::SetText(const chustd::String& strText)
{
	m_strText = strText;
	m_ti.lpszText = const_cast<LPTSTR>(strText.GetBuffer());

	SendMessage(m_handle, TTM_UPDATETIPTEXT, 0, LPARAM(&m_ti));
}

void ToolTip::TrackActivate(bool bShow)
{
	if( m_bVisible == bShow )
	{
		return;
	}
	m_bVisible = bShow;
	
	SendMessage(m_handle, TTM_TRACKACTIVATE, bShow, LPARAM(&m_ti));
}

void ToolTip::TrackPosition(int x, int y)
{
	SendMessage(m_handle, TTM_TRACKPOSITION, 0, LPARAM(DWORD( MAKELONG(x, y) )) );
}

void ToolTip::NewRect(const RECT& rect)
{
	m_ti.rect = rect;
	SendMessage(m_handle, TTM_NEWTOOLRECT, 0, LPARAM(&m_ti));
}
