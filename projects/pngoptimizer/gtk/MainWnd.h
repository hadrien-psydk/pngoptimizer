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

class MainWnd : public Window
{
public:
	Event1<const StringArray&> FilesDropped; // [filePaths]
	
public:
	bool Create(const char* welcomeMsg);
	void AddText(const chustd::String& text, Color cr = Color::Black);
	
private:
	ContextMenu m_ctm;
	TraceCtl m_traceCtl;

private:
	void OnOptions();
	void OnClear();
	void OnAbout();
};

#endif
