/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ContextMenu.h"

static gint my_popup_handler(GtkWidget* widget, GdkEvent* event)
{
	g_return_val_if_fail(widget != nullptr, FALSE);
	g_return_val_if_fail(GTK_IS_MENU (widget), FALSE);
	g_return_val_if_fail(event != nullptr, FALSE);

	// The "widget" is the menu that was supplied when 
 	// g_signal_connect_swapped() was called.
	GtkMenu* menu = GTK_MENU(widget);

	if( event->type == GDK_BUTTON_PRESS )
	{
		GdkEventButton* event_button = (GdkEventButton*) event;
		if( event_button->button == GDK_BUTTON_SECONDARY )
		{
#if !GTK_CHECK_VERSION(3,22,0)
			// gtk_menu_popup() is deprecated since 3.22
			gtk_menu_popup(menu, nullptr, nullptr, nullptr, nullptr, 
				event_button->button, event_button->time);
#else
			gtk_menu_popup_at_pointer(menu, event);
#endif
			return TRUE;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
void MenuItem::SetHandle(WIDGET_HANDLE handle)
{
	m_handle = handle;
	g_signal_connect(G_OBJECT(handle), "activate", G_CALLBACK(OnActivate), this);
}

void MenuItem::OnActivate(WIDGET_HANDLE, gpointer p)
{
	MenuItem* that = reinterpret_cast<MenuItem*>(p);
	that->Activate.Fire();
}

///////////////////////////////////////////////////////////////////////////////
void ContextMenu::Create()
{
	m_handle = gtk_menu_new();
	m_items.EnsureCapacity(20);
}

MenuItem* ContextMenu::AddItem(const char* text)
{
	GtkWidget* item = gtk_menu_item_new_with_label(text);
	gtk_menu_shell_append(GTK_MENU_SHELL(m_handle), item);
	int index = m_items.Add();
	MenuItem* mi = &(m_items[index]);
	mi->SetHandle(item);
	return mi;
}

void ContextMenu::AddSeparator()
{
	//GtkWidget* separator = gtk_separator_menu_item_new();
}

void ContextMenu::Install(Window* owner)
{
	g_signal_connect_swapped(owner->GetHandle(), "button_press_event", G_CALLBACK(my_popup_handler), m_handle);
	gtk_widget_show_all(m_handle);
}
