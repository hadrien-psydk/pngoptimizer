/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AboutDlg.h"
#include "../msgtable.h"

// The Makefile will create logo48.o with those references from logo48.png
extern "C" const char _binary_gtk_logo48_png_start[];
extern "C" const char _binary_gtk_logo48_png_end[];

///////////////////////////////////////////////////////////////////////////////
AboutDlg::AboutDlg()
{

}

///////////////////////////////////////////////////////////////////////////////
GdkPixbuf* gdk_pixbuf_new_from_memory(const void* data, gsize len, GError** error)
{
	GInputStream* is = g_memory_input_stream_new_from_data(data, len, nullptr);
	return gdk_pixbuf_new_from_stream(is, nullptr, error);
}

///////////////////////////////////////////////////////////////////////////////
bool AboutDlg::SetupUI()
{
	GtkAboutDialog* dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

	GError* error = nullptr;
	auto logo = gdk_pixbuf_new_from_memory(_binary_gtk_logo48_png_start,
		_binary_gtk_logo48_png_end - _binary_gtk_logo48_png_start, &error);
	gtk_about_dialog_set_logo(dialog, logo);

	gtk_about_dialog_set_program_name(dialog, PNGO_APPNAME);
	gtk_about_dialog_set_version(dialog, PNGO_VERSION);
	gtk_about_dialog_set_copyright(dialog, PNGO_COPYRIGHT);
	gtk_about_dialog_set_license_type(dialog, GTK_LICENSE_GPL_2_0);
	gtk_about_dialog_set_website(dialog, PNGO_WEBSITE);
	gtk_about_dialog_set_website_label(dialog, PNGO_WEBSITE);
	gtk_about_dialog_set_wrap_license(dialog, TRUE);

	static const gchar* authors[] = { "Hadrien Nilsson", nullptr };
	gtk_about_dialog_set_authors(dialog, authors);

	m_handle = GTK_WIDGET(dialog);

	return true;
}
