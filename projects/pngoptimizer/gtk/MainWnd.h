/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_MAINWND_H
#define PO_MAINWND_H

#include "ContextMenu.h"
#include "TraceCtl.h"
#include "PngOptionsDlg.h"
#include "AboutDlg.h"

struct ThemeInfo
{
	bool darkThemeUsed;
	Color normalColor;

	ThemeInfo() : darkThemeUsed(false) {}
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

	void OnOptions();
	void OnAbout();

private:
	Button m_btPrefs;
	Button m_btClear;
	ContextMenu m_ctm;
	TraceCtl m_traceCtl;
	GtkApplication* m_pGtkApp;

private:
	void OnClear();
};

#endif
