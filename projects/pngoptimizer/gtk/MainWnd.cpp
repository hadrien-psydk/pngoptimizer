/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWnd.h"

#include "AppSettings.h"

static void WinAction_OnActivate(GSimpleAction*, GVariant*, gpointer userData)
{
	WinAction* that = reinterpret_cast<WinAction*>(userData);
	that->Activate.Fire();
}

void WinAction::Create(Window* win, const char* id)
{
	auto hwin = win->GetHandle();

	auto action = g_simple_action_new(id, nullptr);
	g_signal_connect(action, "activate", G_CALLBACK(WinAction_OnActivate), this);
	g_action_map_add_action(G_ACTION_MAP(hwin), G_ACTION(action));
	g_object_unref(action);
}

///////////////////////////////////////////////////////////////////////////////
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

			gchar* path = g_filename_from_uri(uri, nullptr, nullptr);
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

MainWnd::MainWnd()
{
	m_pGtkApp = nullptr;
}

void MainWnd::OnPreferences()
{
	PreferencesDlg dlg;
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
bool MainWnd::Create(GtkApplication* pGtkApp, const char* welcomeMsg)
{
	auto window = gtk_application_window_new(pGtkApp);
	//auto window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	m_handle = window;

	//const MainWndSettings& mwsettings = AppSettings::GetInstance().GetMWSettings();
	//gtk_window_set_keep_above(GTK_WINDOW(window), mwsettings.alwaysOnTop);

	gtk_window_set_title(GTK_WINDOW(window), "PngOptimizer");
	gtk_window_set_default_size(GTK_WINDOW(window), 580, 300);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	////////////////////////////
	// Header bar
	auto headerBar = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(headerBar), "PngOptimizer");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerBar), true);

	auto hamburgerButton = gtk_menu_button_new();
	auto hamburgerImage = gtk_image_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(hamburgerButton), hamburgerImage);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(headerBar), hamburgerButton);

	auto clearButton = gtk_button_new_from_icon_name("edit-clear-all-symbolic",
		GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(headerBar), clearButton);

	m_btHamburger = hamburgerButton;
	m_btClear = clearButton;

	gtk_window_set_titlebar(GTK_WINDOW(window), headerBar);

	m_btClear.Clicked.Connect(this, &MainWnd::OnClear);

	////////////////////////////
	// Drag and drop
	const GtkTargetEntry targets[] = {
		{ const_cast<char*>("text/uri-list"), GTK_TARGET_OTHER_APP, 0xb00b00 }
	};

	gtk_drag_dest_set(window, GTK_DEST_DEFAULT_ALL,
		targets, ARRAY_SIZE(targets), GDK_ACTION_COPY);

	g_signal_connect(window, "drag-data-received",
        G_CALLBACK(OnDragDataReceived), this);

	g_signal_connect(window, "window-state-event",
		G_CALLBACK(OnWindowStateEvent), this);

	////////////////////////////
	// Hamburger menu
	auto menu = g_menu_new();
  	g_menu_append(menu, "Preferences", "win.preferences");
  	g_menu_append(menu, "About", "win.about");
	auto popover = gtk_popover_new_from_model(hamburgerButton, G_MENU_MODEL(menu));

	gtk_menu_button_set_popover(GTK_MENU_BUTTON(hamburgerButton), popover);

	m_actPreferences.Create(this, "preferences");
	m_actPreferences.Activate.Connect(this, &MainWnd::OnPreferences);

	m_actAbout.Create(this, "about");
	m_actAbout.Activate.Connect(this, &MainWnd::OnAbout);

	//////////////////////////
	m_traceCtl.Create(this, welcomeMsg);

	gtk_widget_show(window);
	return true;
}

void MainWnd::AddText(const chustd::String& text, Color cr)
{
	m_traceCtl.AddText(text, cr);
}

ThemeInfo MainWnd::GetThemeInfo() const
{
	ThemeInfo ti;

	GdkRGBA col;
	// Look up the default text color in the theme
	GtkStyleContext* styleContext = gtk_widget_get_style_context(GetHandle());
	gtk_style_context_get_color(styleContext, GTK_STATE_FLAG_NORMAL, &col);
	double lum = 0.2 * col.red + 0.7 * col.green + 0.1 * col.blue;
	ti.darkThemeUsed = bool(lum > 0.3);
	uint8 r = static_cast<uint8>(col.red * 255);
	uint8 g = static_cast<uint8>(col.green * 255);
	uint8 b = static_cast<uint8>(col.blue * 255);
	ti.normalColor = Color(r, g, b);
	return ti;
}
