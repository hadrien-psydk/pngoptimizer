/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Widgets.h"

///////////////////////////////////////////////////////////////////////////////
void Widget::SetFocus()
{
	gtk_widget_grab_focus(GTK_WIDGET(m_handle));
}

void Widget::Enable(bool enabled)
{
	gtk_widget_set_sensitive(GTK_WIDGET(m_handle), enabled);
}

///////////////////////////////////////////////////////////////////////////////
chustd::String Window::GetTitle() const
{
	const char* p = gtk_window_get_title(GTK_WINDOW(m_handle));
	return String::FromUtf8Z(p);
}

///////////////////////////////////////////////////////////////////////////////
void CheckButton::operator=(WIDGET_HANDLE handle)
{
	m_handle = handle;
	g_signal_connect(m_handle, "toggled", G_CALLBACK(OnToggled), this);
}

void CheckButton::Check(bool check)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_handle), check);
}

bool CheckButton::IsChecked() const
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_handle));
}

void CheckButton::OnToggled(WIDGET_HANDLE, gpointer p)
{
	CheckButton* that = reinterpret_cast<CheckButton*>(p);
	that->Toggled.Fire();
}

///////////////////////////////////////////////////////////////////////////////
void Label::operator=(WIDGET_HANDLE handle)
{
	m_handle = handle;
}

void Label::SetText(const chustd::String& str)
{
	char tmp[100];
	str.ToUtf8Z(tmp);
	gtk_label_set_text(GTK_LABEL(m_handle), tmp);
}

///////////////////////////////////////////////////////////////////////////////
void EditBox::operator=(WIDGET_HANDLE handle)
{
	m_handle = handle;
	g_signal_connect(m_handle, "changed", G_CALLBACK(OnChanged), this);
}

void EditBox::OnChanged(WIDGET_HANDLE, gpointer p)
{
	EditBox* that = reinterpret_cast<EditBox*>(p);
	that->Changed.Fire();
}

void EditBox::SetText(const String& text)
{
	char tmp[200];
	text.ToUtf8Z(tmp);
	gtk_entry_set_text(GTK_ENTRY(m_handle), tmp);
}

void EditBox::SetTextInt(int value)
{
	char tmp[40];
	sprintf(tmp, "%d", value);
	gtk_entry_set_text(GTK_ENTRY(m_handle), tmp);
}

chustd::String EditBox::GetText() const
{
	const char* p = gtk_entry_get_text(GTK_ENTRY(m_handle));
	return String::FromUtf8Z(p);
}

int EditBox::GetTextInt() const
{
	const char* p = gtk_entry_get_text(GTK_ENTRY(m_handle));
	return atoi(p);
}

///////////////////////////////////////////////////////////////////////////////
void Button::operator=(WIDGET_HANDLE handle)
{
	m_handle = handle;
	g_signal_connect(m_handle, "clicked", G_CALLBACK(OnClicked), this);
}

void Button::OnClicked(WIDGET_HANDLE, gpointer p)
{
	Button* that = reinterpret_cast<Button*>(p);
	that->Clicked.Fire();
}

///////////////////////////////////////////////////////////////////////////////
void ComboBox::operator=(WIDGET_HANDLE handle)
{
	m_handle = handle;
}

int ComboBox::AddString(const chustd::String& str)
{
	char tmp[100];
	str.ToUtf8Z(tmp);
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(m_handle), nullptr, tmp);
	return 0;
}

void ComboBox::LimitText(int /*limit*/)
{

}

chustd::String ComboBox::GetText() const
{
	auto entry = gtk_bin_get_child(GTK_BIN(m_handle));
	const gchar* tmp = gtk_entry_get_text(GTK_ENTRY(entry));
	return String::FromUtf8Z(tmp);
}

void ComboBox::SetText(const chustd::String& text)
{
	char tmp[200];
	text.ToUtf8Z(tmp);
	auto entry = gtk_bin_get_child(GTK_BIN(m_handle));
	gtk_entry_set_text(GTK_ENTRY(entry), tmp);
}

///////////////////////////////////////////////////////////////////////////////
void ColorButton::operator=(WIDGET_HANDLE handle)
{
	m_handle = handle;
	g_signal_connect(m_handle, "color-set", G_CALLBACK(OnColorSet), this);
}

void ColorButton::SetColor(chustd::Color cr)
{
	GdkRGBA gc;
	gc.red = double(cr.r) / 255.0;
	gc.green = double(cr.g) / 255.0;
	gc.blue = double(cr.b) / 255.0;
	gc.alpha = 1.0;
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(m_handle), &gc);
}

chustd::Color ColorButton::GetColor() const
{
	GdkRGBA gc;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(m_handle), &gc);
	chustd::Color ret;
	ret.r = int(gc.red * 255);
	ret.g = int(gc.green * 255);
	ret.b = int(gc.blue * 255);
	return ret;
}

void ColorButton::OnColorSet(WIDGET_HANDLE, gpointer p)
{
	ColorButton* that = reinterpret_cast<ColorButton*>(p);
	that->ColorSet.Fire();
}
