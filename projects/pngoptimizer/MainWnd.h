/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_MAINWND_H
#define PO_MAINWND_H

#include "POTraceCtl.h"

// PngOptimizer Main window class

class MainWnd : public chuwin32::Window
{
public:
	// Events
	Event0 Destroying;  // Fired when the window is closing
	Event0 ListCleared; // Fired after the list was cleared
	Event0 Activated;   // Fired the window was moved to foreground
	Event1<const StringArray&> FilesDropped; // [filePaths]

public:
	bool Create(const String& title, RECT rcWnd, bool alwaysOnTop, const String& welcomeMsg);
	
	HWND   GetHandle();
	String GetLastError() const;
	void   AllowFileDropping(bool bAllow);

	void SetAlwaysOnTop(bool alwaysOnTop);
	bool GetAlwaysOnTop() const;

	///////////////////////////////////////////////////////////////////////////////
	// Proxy functions to write inside the tracectl
	void Write(const String& strText, COLORREF cr);
	void Write(const String& strText, COLORREF cr, int minWidthEx, int justify);
	void WriteLine(const String& strText, COLORREF cr);
	void SetTraceCtlRedraw(bool bRedraw);
	void ClearLine();
	void AddLink(const String& strFileNameFinal, const String& strTotalFinalPath);
	///////////////////////////////////////////////////////////////////////////////

	MainWnd();

private:
	POTraceCtl m_tracectl; // The main control of the window

	String m_strErr;
	StringArray m_droppedFilePaths; // Used as a parameter when firing m_eventFilesDropped

	bool m_alwaysOnTop;

private:
	void DoPngOptions();
	void DoScreenshotsOptions();
	void DoAbout();

	void ResizeTraceCtl(int cx, int cy);
	void HandleMenuItemStates(HMENU hSubMenu);

	bool IsTraceCtlHwnd(HWND hwndCompare);
	void Clear();

	String GetDraggedFileName(HDROP hDrop, int nFile);

	///////////////////////////////////////////////////////
	virtual LRESULT WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

	void OnWmMouseWheel(WPARAM wParam, LPARAM lParam);
	void OnWmDropFiles(WPARAM wParam, LPARAM lParam);
	///////////////////////////////////////////////////////
};

#endif // ndef PO_MAINWND_H
