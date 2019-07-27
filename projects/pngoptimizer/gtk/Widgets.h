/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_WIDGETS_H
#define PO_WIDGETS_H

typedef GtkWidget* WIDGET_HANDLE;

///////////////////////////////////////////////////////////////////////////////
class Widget
{
public:
	Widget() : m_handle(nullptr) {}
	WIDGET_HANDLE GetHandle() const { return m_handle; }
	void SetFocus();
	void Enable(bool);
protected:
	WIDGET_HANDLE m_handle;
};

///////////////////////////////////////////////////////////////////////////////
class Window : public Widget
{
public:
	chustd::String GetTitle() const;
};

///////////////////////////////////////////////////////////////////////////////
class CheckButton : public Widget
{
public:
	chustd::Event0 Toggled;
public:
	void operator=(WIDGET_HANDLE handle);
	void Check(bool check = true);
	bool IsChecked() const;
private:
	static void OnToggled(WIDGET_HANDLE, gpointer);
};

///////////////////////////////////////////////////////////////////////////////
typedef CheckButton RadioButton;

///////////////////////////////////////////////////////////////////////////////
class Label : public Widget
{
public:
	void operator=(WIDGET_HANDLE handle);
	void SetText(const chustd::String& str);
};

///////////////////////////////////////////////////////////////////////////////
class EditBox : public Widget
{
public:
	chustd::Event0 Changed; // Fires after the content was changed
public:
	void operator=(WIDGET_HANDLE handle);
	void SetSel(int start, int stop);
	void SetSelAll();
	void SetText(const String& text);
	void SetTextInt(int);
	chustd::String GetText() const;
	int GetTextInt() const;
private:
	static void OnChanged(WIDGET_HANDLE, gpointer);
};

///////////////////////////////////////////////////////////////////////////////
class Button : public Widget
{
public:
	chustd::Event0 Clicked;
public:
	void operator=(WIDGET_HANDLE handle);
private:
	static void OnClicked(WIDGET_HANDLE, gpointer);
};

///////////////////////////////////////////////////////////////////////////////
class ComboBox : public Widget
{
public:
	void operator=(WIDGET_HANDLE handle);
	int AddString(const chustd::String& str);
	void LimitText(int limit);
	chustd::String GetText() const;
	void SetText(const chustd::String& text);
};

///////////////////////////////////////////////////////////////////////////////
class ColorButton : public Widget
{
public:
	chustd::Event0 ColorSet; // Fires when user chose a color
public:
	void operator=(WIDGET_HANDLE handle);
	void SetColor(chustd::Color cr);
	chustd::Color GetColor() const;

protected:
	chustd::Color m_color;
protected:
	static void OnColorSet(WIDGET_HANDLE, gpointer);
};

#endif
