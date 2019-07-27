// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

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
