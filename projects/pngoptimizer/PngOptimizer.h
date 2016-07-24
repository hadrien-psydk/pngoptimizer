///////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) 2002/2014 Hadrien Nilsson - psydk.org
// 
// PngOptimizer is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// PngOptimizer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with PngOptimizer; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef PO_PNGOPTIMIZER_H
#define PO_PNGOPTIMIZER_H

#include "BmpClipboardDumper.h"
#include "MainWnd.h"
#include "AppSettings.h"

// Main application class
class POApplication
{
public:
	POEngine m_engine;          // The optimization engine object.
	BmpClipboardDumper m_bmpcd; // The object used to capture bitmaps from the clipboard

	HINSTANCE m_hInstance;      // Various Windows function need that

public:
	bool Initialize(HINSTANCE hInstance);
	int32 Run();

	bool IsJobRunning();         // Used by the user interface to know what menus have to be disabled
	void OnJobDone();            // Received by the main window when the working thread finished its work
	bool IsBmpAvailable() const; // Used by the user interface to determine the state of the "paste screenshot" menu

	void DumpScreenshot();       // Called by the user interface to create a screenshot

	POApplication();
	virtual ~POApplication();

	// Allows an access to the single application instance
	static POApplication& GetInstance();

	// Color conversion from one world (the engine) to another (the Windows UI)
	static COLORREF CrFromTc(POEngine::TextType textType);

private:
	// Related to the working thread
	static int ThreadProcStatic(void* pParameter);
	void ThreadProc();

	void StartJob();

	// Events callbacks
	void OnEngineProgressing(const POEngine::ProgressingArg& arg);
	void OnBmpcdStateChanged(BmpClipboardDumper::DumpState eDumpState);
	void OnMainWndFilesDropped(const chustd::StringArray& filePaths);
	void OnMainWndDestroying();
	void OnMainWndListCleared();
	void OnMainWndActivated();

	void ChangeRectBecauseOfOverlap(RECT& rcWnd);
	void ProcessCmdLineArgs();
	
	// Perform event connections to interesting objects (m_engine, m_bmpcd et m_mainwnd)
	void DoConnections();

private:
	MainWnd        m_mainwnd;         // PngOptimizer main window
	StringArray    m_filePaths;   // Files to process by the working thread
	Thread         m_workingThread;   // The working thread
	HACCEL         m_hAccel;          // Keyboard shortcut array handle
	HANDLE         m_hMutexPngOptimizer; // Mutex used to know if a previous instance is running
	ITaskbarList3* m_pTaskbarList;       // To control the icon display on Vista/Seven
};

///////////////////////////////////////////////////////////////////////////////
// Message sent by the working thread and received by the main window
// to know when a job is done
const int WM_APP_THREADJOBDONE = WM_APP+4;
///////////////////////////////////////////////////////////////////////////////

#endif // ndef PO_PNGOPTIMIZER_H
