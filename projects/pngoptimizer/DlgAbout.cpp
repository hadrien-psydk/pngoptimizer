/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgAbout.h"

#include "resource.h"

static const char k_appVersion[] = "2.4.3";

/////////////////////////////////////////////////////////////////////////////////////
DlgAbout::DlgAbout()
{
	m_hFont = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
int DlgAbout::DoModal(HWND hParent)
{
	return DoModalHelper(GetModuleHandle(NULL), IDD_ABOUT, hParent);
}

static HFONT CreateBoldWindowFont(Window wnd)
{
	HFONT hFont = wnd.GetFont();
	LOGFONTW font = { 0 };
	::GetObject(hFont, sizeof(font), &font);
	font.lfWeight = FW_BOLD;
	return CreateFontIndirectW(&font); 
}

/////////////////////////////////////////////////////////////////////////////////////
LRESULT DlgAbout::DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if( nMsg == WM_INITDIALOG )
	{
		Window wndVer = GetItem(IDC_STATIC_VERSION);
		String version = wndVer.GetText();
		version = version + String(k_appVersion);
#if defined(_M_X64)
		version = version + String(" (x64)");
#elif defined(_M_IX86)  
		version = version + String(" (x86)");
#endif
		wndVer.SetText(version);

		// Make the main text bold
		m_hFont = CreateBoldWindowFont(wndVer); 
		wndVer.SetFont(m_hFont, false);

		Window wnd = GetItem(IDC_STATIC_URL);
		if( m_syslinkUrl.Create(wnd.GetRelativeRect(), m_hWnd, IDC_STATIC_URL) )
		{
			// Replace the static text with a SysLink
			String strOri = wnd.GetText();
			String str = "<a href=\"http://pngoptimizer.org\">" + strOri + "</a>";
			m_syslinkUrl.SetText(str);
			m_syslinkUrl.SetSameFontThanParent();
			wnd.Hide();
		}
		CenterWindow(true);
		return TRUE;
	}
	else if( nMsg == WM_CLOSE )
	{
		if( m_hFont != NULL )
		{
			DeleteObject(m_hFont);
			m_hFont = NULL;
		}
	}
	else if( nMsg == WM_NOTIFY )
	{
		// Open the link in default browser
		PNMLINK pNMLink = (PNMLINK)lParam;
		if( pNMLink->hdr.code == NM_CLICK || pNMLink->hdr.code == NM_RETURN )
		{
			LITEM item = pNMLink->item;
			if( item.iLink == 0 )
			{
				ShellExecute(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
			}
			return TRUE;
		}
	}
	return Dialog::DlgProc(nMsg, wParam, lParam);
}
