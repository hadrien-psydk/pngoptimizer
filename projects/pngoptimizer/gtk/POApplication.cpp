/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
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
	m_pGtkApp = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
void POApplication::OnFilesDropped(const StringArray& filePaths)
{
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

	ThemeInfo themeInfo = m_mainwnd.GetThemeInfo();
	Color cr = POEngine::ColorFromTextType(progArg.textType, themeInfo.darkThemeUsed);
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
		m_engine.OptimizeMultiFilesDisk(filePaths);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
void POApplication::ProcessCmdLineArgs(int argc, char** argv)
{
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

#if 0
#define WANTS_TRACECTL_TEST
///////////////////////////////////////////////////////////////////////////////
static void TestTraceCtl(MainWnd& mainwnd)
{
	gtk_window_resize(GTK_WINDOW(mainwnd.GetHandle()), 600, 600);
	for(int i = 0; i < POEngine::TT_Last + 1; ++i)
	{
		Color col = POEngine::ColorFromTextType(static_cast<POEngine::TextType>(i), false);
		mainwnd.AddText("This is a test line with light theme\n", col);
	}
	mainwnd.AddText("---------------------------------------------------\n", Color::Black);
	for(int i = 0; i < POEngine::TT_Last + 1; ++i)
	{
		Color col = POEngine::ColorFromTextType(static_cast<POEngine::TextType>(i), true);
		mainwnd.AddText("This is a test line with dark theme\n", col);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
bool POApplication::Init()
{
	m_pGtkApp = gtk_application_new("org.psydk.pngoptimizer",
		G_APPLICATION_HANDLES_COMMAND_LINE
		//G_APPLICATION_HANDLES_OPEN
		//G_APPLICATION_FLAGS_NONE
		);

	g_signal_connect(m_pGtkApp, "startup", G_CALLBACK(&POApplication::OnStartupStatic), this);
	g_signal_connect(m_pGtkApp, "activate", G_CALLBACK(&POApplication::OnActivateStatic), this);
	g_signal_connect(m_pGtkApp, "command-line", G_CALLBACK(&POApplication::OnCommandLineStatic), this);
	g_signal_connect(m_pGtkApp, "open", G_CALLBACK(&POApplication::OnOpenStatic), this);

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

	m_engine.Progressing.Connect(this, &POApplication::OnEngineProgressing);

	return true;
}

void POApplication::OnStartupStatic(GtkApplication*, gpointer userData)
{
	POApplication* that = (POApplication*)userData;
	that->OnStartup();
}

void POApplication::OnActivateStatic(GtkApplication*, gpointer userData)
{
	POApplication* that = (POApplication*)userData;
	that->OnActivate();
}

int POApplication::OnCommandLineStatic(GtkApplication*,
	GApplicationCommandLine* commandLine, gpointer userData)
{
	POApplication* that = (POApplication*)userData;
	return that->OnCommandLine(commandLine);
}

void POApplication::OnOpenStatic(GApplication* app, gpointer, gint /*fileCount*/, gchar*, gpointer)
{
	//printf("on open static\n");
	g_application_activate(app);
}

void POApplication::OnStartup()
{
	if( !m_mainwnd.Create(m_pGtkApp, WELCOME_MESSAGE) )
	{
		//return false;
	}
	m_mainwnd.FilesDropped.Connect(this, &POApplication::OnFilesDropped);

#ifdef WANTS_TRACECTL_TEST
	TestTraceCtl(m_mainwnd);
#endif
}

void POApplication::OnActivate()
{
	//printf("on activate\n");
	gtk_widget_show_all(GTK_WIDGET(m_mainwnd.GetHandle()));
}

int POApplication::OnCommandLine(GApplicationCommandLine* commandLine)
{
	//printf("on command line\n");
	int argc = 0;
	gchar** argv = g_application_command_line_get_arguments(commandLine, &argc);
	ProcessCmdLineArgs(argc, argv);
	g_application_activate((GApplication*)m_pGtkApp);
	return 0;
}

/////////////////////////////////////////////////////////////
int POApplication::Run(int argc, char** argv)
{
	if( argc > 1 && String(argv[1]) == "--version" )
	{
		Console::WriteLine(PNGO_APPNAME " " PNGO_VERSION);
		return 0;
	}
	int status = g_application_run(G_APPLICATION(m_pGtkApp), argc, argv);

	g_object_unref(m_pGtkApp);
	m_pGtkApp = nullptr;

	m_exitNow = true;
	m_semStart.Increment();
	m_thread.WaitForExit();

	m_semStart.Close();
	return status;
}
