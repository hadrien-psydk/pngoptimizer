/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AboutDlg.h"

#include "version.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////
AboutDlg::AboutDlg() : Dialog(IDD_ABOUT)
{
	m_hFont = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
static HFONT CreateBoldWindowFont(Window wnd)
{
	HFONT hFont = wnd.GetFont();
	LOGFONTW font = { 0 };
	::GetObject(hFont, sizeof(font), &font);
	font.lfWeight = FW_BOLD;
	return CreateFontIndirectW(&font); 
}

/////////////////////////////////////////////////////////////////////////////////////
bool AboutDlg::SetupUI()
{
	Window wndVer = GetItem(IDC_STATIC_VERSION);
	String version = PNGO_APPNAME;
	version = version + " ";
	version = version + String(PNGO_VERSION);
#if defined(_M_X64)
	version = version + String(" (x64)");
#elif defined(_M_IX86)  
	version = version + String(" (x86)");
#endif
	wndVer.SetText(version);

	Label labelCopyright = GetItem(IDC_STATIC_COPYRIGHT);
	labelCopyright.SetText(PNGO_COPYRIGHT);

	// Make the main text bold
	m_hFont = CreateBoldWindowFont(wndVer); 
	wndVer.SetFont(m_hFont, false);

	Window wnd = GetItem(IDC_STATIC_URL);
	if( m_syslinkUrl.Create(wnd.GetRelativeRect(), this, IDC_STATIC_URL) )
	{
		// Replace the static text with a SysLink
		String strOri = PNGO_WEBSITE;
		String str = "<a href=\"" PNGO_WEBSITE "\">" + strOri + "</a>";
		m_syslinkUrl.SetText(str);
		m_syslinkUrl.SetSameFontThanParent();
		wnd.Hide();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
void AboutDlg::SetupConnections()
{
}

/////////////////////////////////////////////////////////////////////////////////////
void AboutDlg::LoadValues()
{
}

/////////////////////////////////////////////////////////////////////////////////////
bool AboutDlg::StoreValues()
{
	if( m_hFont != NULL )
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
	return true;
}
