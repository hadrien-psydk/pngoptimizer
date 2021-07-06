///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_GUI_H
#define CHUI_GUI_H

#include "Widget.h"

namespace chuwin32 {

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
	virtual void OnCommand(uintptr_t);
};

typedef CheckButton RadioButton;
typedef Widget Label;

///////////////////////////////////////////////////////////////////////////////
class EditBox : public Widget
{
public:
	chustd::Event0 Changed; // Fires after the content was changed
public:
	void operator=(WIDGET_HANDLE handle) { SetHandle(handle); }
	bool Create(const Rect& rc, const Widget* parent, int id);
	void SetSel(int start, int stop);
	void SetSelAll();

private:
	virtual void OnCommand(uintptr_t);
};

///////////////////////////////////////////////////////////////////////////////
class Button : public Widget
{
public:
	chustd::Event0 Clicked;
public:
	void operator=(WIDGET_HANDLE handle) { SetHandle(handle); }

private:
	virtual void OnCommand(uintptr_t);
};

///////////////////////////////////////////////////////////////////////////////
class ComboBox : public Widget
{
public:
	void operator=(WIDGET_HANDLE handle) { SetHandle(handle); }
	int AddString(const chustd::String& str);
	void SetItemData(int nIndex, uint32 nData);
	int GetCurSel();
	uint32 GetItemData(int nIndex);
	void LimitText(int limit);
};

///////////////////////////////////////////////////////////////////////////////
class Slider : public Widget
{
public:
	void operator=(WIDGET_HANDLE handle) { SetHandle(handle); }
	void SetRangeMin(int min, bool redraw = false);
	void SetRangeMax(int max, bool redraw = false);
	void SetRange(int min, int max, bool redraw = false);
	void SetPos(int pos);
	int  GetPos();
};

///////////////////////////////////////////////////////////////////////////////
class SysLink : public Widget
{
public:
	void operator=(WIDGET_HANDLE handle) { SetHandle(handle); }
	bool Create(const Rect& rc, const Widget* parent, int id);

private:
	virtual void OnCommand(uintptr_t);
};

///////////////////////////////////////////////////////////////////////////////
} // namespace chuwin32

#endif // ndef CHUI_GUI_H
