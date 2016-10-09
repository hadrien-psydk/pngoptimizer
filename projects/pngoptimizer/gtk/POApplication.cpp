/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POApplication.h"

#include "MainWnd.h"
#include "MsgDialog.h"
#include "AppSettings.h"

#include "../msgtable.h"

///////////////////////////////////////////////////////////////////////////////
POApplication::POApplication()
{
	m_exitNow = false;
}

///////////////////////////////////////////////////////////////////////////////
void POApplication::OnFilesDropped(const StringArray& filePaths)
{
	/*
	Console::WriteLine("po drop");
	foreach(filePaths, i)
	{
		Console::WriteLine(filePaths[i]);
	}
	*/
	m_lock.Enter();
	m_filePaths = filePaths;
	m_lock.Leave();

	StartJob();
}

///////////////////////////////////////////////////////////////////////////////
int POApplication::DoEngineProgressingStatic(void* p)
{
	auto that = reinterpret_cast<POApplication*>(p);
	that->DoEngineProgressing();
	return FALSE;
}

void POApplication::DoEngineProgressing()
{
	POEngine::ProgressingArg progArg;

	m_lock.Enter();
	if( !m_progArgs.IsEmpty() )
	{
		progArg = m_progArgs[0];
		m_progArgs.RemoveAt(0);
	}
	m_lock.Leave();

	Color cr = POEngine::ColorFromTextType(progArg.textType);

	// This is a test to pretty format file sizes
	/*
	int minWidthEx = 0;
	int justify = 0;
	if( arg.textType == POEngine::TT_SizeInfoNum )
	{
		// Pretty formatting for numbers
		minWidthEx = 0;
		justify = 1;
	}
	m_mainwnd.AddText(arg.text, cr, minWidthEx, justify);
	*/
	m_mainwnd.AddText(progArg.text, cr);
}

///////////////////////////////////////////////////////////////////////////////
void POApplication::OnEngineProgressing(const POEngine::ProgressingArg& arg)
{
	m_lock.Enter();
	m_progArgs.Add(arg);
	m_lock.Leave();

	// Here we are in the engine thread, we want to update the TraceCtl
	// from the UI thread
	g_idle_add(&DoEngineProgressingStatic, this);
}

///////////////////////////////////////////////////////////////////////////////
void POApplication::OnSettingsChanged()
{
	m_engine.m_settings = AppSettings::GetInstance().GetPOSettings();
}

///////////////////////////////////////////////////////////////////////////////
int POApplication::ThreadProcStatic(void* p)
{
	auto that = reinterpret_cast<POApplication*>(p);
	return that->ThreadProc();
}

int POApplication::ThreadProc()
{
	for(;;)
	{
		if( m_semStart.Wait() != 0 )
		{
			Console::WriteLine("Thread sem wait failed");
			return 1;
		}
		if( m_exitNow )
		{
			return 0;
		}
		// Start consuming m_filePaths
		StringArray filePaths = m_filePaths;
		m_filePaths.Clear();
		m_engine.OptimizeFiles(filePaths);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
void POApplication::ProcessCmdLineArgs(int argc, char** argv)
{
	StringArray args = Process::CommandLineToArgv( Process::GetCommandLine());
	if( argc <= 1 )
		return;

	m_filePaths.SetSize(0);
	m_filePaths.EnsureCapacity(argc - 1);
	for(int i = 1; i < argc; ++i)
	{
		m_filePaths.Add(argv[i]);
	}
	StartJob();
}

///////////////////////////////////////////////////////////////////////////////
void POApplication::StartJob()
{
	m_semStart.Increment();
}

///////////////////////////////////////////////////////////////////////////////
bool POApplication::Init(int argc, char** argv)
{
	gtk_init(&argc, &argv);

	if( !m_mainwnd.Create(WELCOME_MESSAGE) )
	{
		return false;
	}

	AppSettings::GetInstance().Read();
	m_engine.m_settings = AppSettings::GetInstance().GetPOSettings();
	AppSettings::GetInstance().Changed.Connect(this, &POApplication::OnSettingsChanged);

	// On Linux we can
	m_engine.EnableUnicodeArrow();

	// Perform some early resources creation to speedup first optimization
	if( !m_engine.WarmUp() )
	{
		MsgDialog md("POEngine warm-up failed", PNGO_ERROR, CMT_Warning, CBT_Ok);
		md.DoModal(nullptr);
		return false;
	}

	m_semStart.Create();
	m_thread.Start(&POApplication::ThreadProcStatic, this);

	m_mainwnd.FilesDropped.Connect(this, &POApplication::OnFilesDropped);
	m_engine.Progressing.Connect(this, &POApplication::OnEngineProgressing);

	// Test TraceCtl
	/*
	for(int i = 0; i < 10; ++i)
	{
		m_mainwnd.AddText(String::FromInt(i) + " onk", Color::Black);
		m_mainwnd.AddText("plop", Color::Red);
		m_mainwnd.AddText("chabada", Color::Green);
		m_mainwnd.AddText("chabadaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", Color::Magenta);
		//wchar x = 0x2192;
		wchar x = '.';
		m_mainwnd.AddText( String(&x, 1), Color(50, 50, 50) );
		m_mainwnd.AddText("(OK) lorem ipsum doloret truc\n", Color(100, 100, 100));
	}
	*/

	ProcessCmdLineArgs(argc, argv);

	/////////////////////////////////////////////////////////////
	// Create application shortcuts
	return true;
}

/////////////////////////////////////////////////////////////
int POApplication::Run()
{	
	gtk_main();

	m_exitNow = true;
	m_semStart.Increment();
	m_thread.WaitForExit();

	m_semStart.Close();
	return 0;
}
