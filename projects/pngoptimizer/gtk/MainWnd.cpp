/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWnd.h"

#include "AppSettings.h"

static void OnDragDataReceived(GtkWidget*, GdkDragContext* context,
	int /* x */, int /* y */,
    GtkSelectionData* seldata, guint /*info*/, guint time,
    gpointer userData)
{
	//Console::WriteLine("drop");

	StringArray arg;

	gchar** uris = gtk_selection_data_get_uris(seldata);
	if( uris )
	{
		int i = 0;
		for(;;)
		{
			const gchar* uri = uris[i];
			if( !uri )
				break;

			gchar* path = g_filename_from_uri(uri, NULL, NULL);
			if( path )
			{
				arg.Add( String::FromUtf8Z(path) );
				g_free(path);
			}
			i++;
		}	
	 	g_strfreev(uris);
	}
	bool success = !arg.IsEmpty();

	gtk_drag_finish(context, success, FALSE, time);

	if( arg.IsEmpty() )
		return;
 
	MainWnd* that = reinterpret_cast<MainWnd*>(userData);
	that->FilesDropped.Fire(arg);
}

static void OnWindowStateEvent(GtkWidget*, GdkEvent* event, gpointer /*userData*/)
{
	if( event->window_state.changed_mask == GDK_WINDOW_STATE_FOCUSED )
	{
		return;
	}

	// This is an attempt to get the "Always on top" setting from the Window Manager
	// It seems buggy at this time
/*
	printf("wse %08x %08x\n",
		event->window_state.changed_mask,
		event->window_state.new_window_state);
		*/
/*
	if( event->window_state.changed_mask & GDK_WINDOW_STATE_ABOVE )
	{
		MainWndSettings mwsettings;
		mwsettings.alwaysOnTop = (event->window_state.new_window_state & GDK_WINDOW_STATE_ABOVE);
		AppSettings::GetInstance().SetMWSettings(mwsettings);
		// Window settings are save upon exit
	}*/
}

static void OnDeleteEvent(GtkWidget* /*widget*/, GdkEvent*, gpointer)
{
	//GdkWindow* gw = gtk_widget_get_window(widget);
	//GdkWindowState state = gdk_window_get_state(gw);
	//printf("delete-event %08x\n", (unsigned int)(state));

	gtk_main_quit();
}

void MainWnd::OnOptions()
{
	PngOptionsDlg dlg;
	dlg.m_settings = AppSettings::GetInstance().GetPOSettings();
	if( dlg.DoModal(this) != DialogResp::Ok )
		return;

	AppSettings::GetInstance().SetPOSettings(dlg.m_settings);
	AppSettings::GetInstance().Write();
}

void MainWnd::OnClear()
{
	m_traceCtl.Clear();
}

void MainWnd::OnAbout()
{
	AboutDlg aboutDlg;
	aboutDlg.DoModal(this);
}

///////////////////////////////////////////////////////////////////////////////
bool MainWnd::Create(const char* welcomeMsg)
{
	auto window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	m_handle = window;

	//const MainWndSettings& mwsettings = AppSettings::GetInstance().GetMWSettings();
	//gtk_window_set_keep_above(GTK_WINDOW(window), mwsettings.alwaysOnTop);

	gtk_window_set_title(GTK_WINDOW(window), "PngOptimizer");
	gtk_window_set_default_size(GTK_WINDOW(window), 580, 300);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	GtkTargetEntry targets[] = {
		{ const_cast<char*>("text/uri-list"), GTK_TARGET_OTHER_APP, 0xb00b00 }
	};

	gtk_drag_dest_set(window, GTK_DEST_DEFAULT_ALL,
		targets, ARRAY_SIZE(targets), GDK_ACTION_COPY);

    g_signal_connect(window, "drag-data-received",
        G_CALLBACK(OnDragDataReceived), this);

	g_signal_connect(window, "window-state-event", 
		G_CALLBACK(OnWindowStateEvent), this);

	////////////////////////////
	// Context menu
	m_ctm.Create();
	auto mi0 = m_ctm.AddItem("Options...");
	m_ctm.AddSeparator();
	auto mi1 = m_ctm.AddItem("Clear");
	m_ctm.AddSeparator();
	auto mi2 = m_ctm.AddItem("About");
	m_ctm.Install(this);

	mi0->Activate.Connect(this, &MainWnd::OnOptions);
	mi1->Activate.Connect(this, &MainWnd::OnClear);
	mi2->Activate.Connect(this, &MainWnd::OnAbout);
	
	//////////////////////////
    m_traceCtl.Create(this, welcomeMsg);

	g_signal_connect(window, "delete-event", G_CALLBACK(OnDeleteEvent), this);

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), this);
	gtk_widget_show_all(window);

	return true;
}

void MainWnd::AddText(const chustd::String& text, Color cr)
{
	m_traceCtl.AddText(text, cr);
}
