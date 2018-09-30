/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_MAINWND_H
#define PO_MAINWND_H

#include "TraceCtl.h"
#include "PreferencesDlg.h"
#include "AboutDlg.h"

struct ThemeInfo
{
	bool darkThemeUsed;
	Color normalColor;

	ThemeInfo() : darkThemeUsed(false) {}
};

class WinAction
{
public:
	Event0 Activate;

public:
	void Create(Window*, const char* id);
};

class MainWnd : public Window
{
public:
	Event1<const StringArray&> FilesDropped; // [filePaths]

public:
	MainWnd();
	bool Create(GtkApplication* pGtkApp, const char* welcomeMsg);
	void AddText(const chustd::String& text, Color cr = Color::Black);
	ThemeInfo GetThemeInfo() const;

private:
	Button m_btHamburger;
	Button m_btClear;
	WinAction m_actPreferences;
	WinAction m_actAbout;
	TraceCtl m_traceCtl;
	GtkApplication* m_pGtkApp;

private:
	void OnClear();
	void OnPreferences();
	void OnAbout();
};

#endif
