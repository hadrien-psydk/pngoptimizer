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

#ifndef PO_POAPPLICATION_H
#define PO_POAPPLICATION_H

#include "MainWnd.h"

class POApplication
{
public:
	POEngine m_engine;          // The optimization engine object.

public:
	POApplication();

	bool Init();
	int Run(int argc, char** argv);

private:
	MainWnd        m_mainwnd;       // PngOptimizer main window

	CriticalSection m_lock;

	// In thread arguments
	StringArray    m_filePaths;     // Files to process by the working thread

	Thread         m_thread;
	bool           m_exitNow;
	Semaphore      m_semStart;

	// Out thread arguments
	Array<POEngine::ProgressingArg> m_progArgs;

	GtkApplication* m_pGtkApp;

private:
	void OnFilesDropped(const StringArray& filePaths);
	void OnEngineProgressing(const POEngine::ProgressingArg& arg);
	void OnSettingsChanged();

	static int ThreadProcStatic(void*);
	int ThreadProc();

	static int DoEngineProgressingStatic(void*);
	void DoEngineProgressing();

	void ProcessCmdLineArgs(int argc, char** argv);
	void StartJob();

	static void OnStartupStatic(GtkApplication*, gpointer);
	static void OnActivateStatic(GtkApplication*, gpointer);
	static int OnCommandLineStatic(GtkApplication*, GApplicationCommandLine*, gpointer);
	static void OnOpenStatic(GApplication*, gpointer, gint, gchar*, gpointer);

	void OnStartup();
	void OnActivate();
	int OnCommandLine(GApplicationCommandLine*);
};

#endif
