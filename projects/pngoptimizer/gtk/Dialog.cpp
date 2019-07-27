/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dialog.h"

Dialog::Dialog()
{
	m_handle = nullptr;
	m_builder = nullptr;
	m_parent = nullptr;
}

static void OnOkClicked(WIDGET_HANDLE, gpointer p)
{
	Dialog* that = reinterpret_cast<Dialog*>(p);
	gtk_dialog_response(GTK_DIALOG(that->GetHandle()), GTK_RESPONSE_OK);
}

static void OnCancelClicked(WIDGET_HANDLE, gpointer p)
{
	Dialog* that = reinterpret_cast<Dialog*>(p);
	gtk_dialog_response(GTK_DIALOG(that->GetHandle()), GTK_RESPONSE_CANCEL);
}

DialogResp Dialog::DoModal(Window* parent)
{
	m_parent = parent;

	if( !SetupUI() )
	{
		return DialogResp::None;
	}

	auto button_ok = GetItem("button_ok");
	auto button_cancel = GetItem("button_cancel");

	if( button_ok )
	{
		g_signal_connect(button_ok, "clicked", G_CALLBACK(OnOkClicked), this);
	}
	if( button_cancel )
	{
		g_signal_connect(button_cancel, "clicked", G_CALLBACK(OnCancelClicked), this);
	}

	SetupConnections();
	LoadValues();

	if( parent )
	{
		gtk_window_set_transient_for(GTK_WINDOW(m_handle), GTK_WINDOW(parent->GetHandle()));
	}
	gtk_widget_show_all(GTK_WIDGET(m_handle));

	int gtk_ret = 0;
	for(;;)
	{
		gtk_ret = gtk_dialog_run(GTK_DIALOG(m_handle));
		if( gtk_ret != GTK_RESPONSE_OK || StoreValues() )
		{
			break;
		}
	}
	gtk_widget_destroy(GTK_WIDGET(m_handle));

	DialogResp ret = DialogResp::None;
	if( gtk_ret == GTK_RESPONSE_OK )
	{
		ret = DialogResp::Ok;
	}
	else if( gtk_ret == GTK_RESPONSE_CANCEL )
	{
		ret = DialogResp::Cancel;
	}
	return ret;
}

bool Dialog::LoadTemplate(const char* begin, const char* end)
{
	auto builder = gtk_builder_new_from_string(begin, end-begin);
	if( !builder )
	{
		g_error("gtk_builder_new_from_string failed");
		return false;
	}
	auto object = gtk_builder_get_object(builder, "dialog");
	if( !object )
	{
		g_error("gtk_builder_get_object failed");
		return false;
	}
	auto dialog = GTK_DIALOG(object);
	if( !dialog )
	{
		g_error("root object is not a dialog");
		return false;
	}

	m_handle = GTK_WIDGET(dialog);
	m_builder = builder;
	return true;
}

WIDGET_HANDLE Dialog::GetItem(const char* name)
{
	if( !m_builder )
	{
		return nullptr;
	}
	auto obj = gtk_builder_get_object(m_builder, name);
	if( !obj )
	{
		return nullptr;
	}
	return GTK_WIDGET(obj);
}
