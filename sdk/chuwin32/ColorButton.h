///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_COLORBUTTON_H
#define CHUI_COLORBUTTON_H

#include "Widget.h"

namespace chuwin32 {

class ColorButton : public Widget
{
public:
	chustd::Event0 ColorSet; // Fires when user chose a color

public:
	void operator = (WIDGET_HANDLE handle) { m_handle = handle; }

	bool Create(const Rect& rc, const Widget* parent, int id);
	void SetColor(chustd::Color cr);
	chustd::Color GetColor() const;

protected:
	chustd::Color m_color;
protected:
	static LRESULT CALLBACK WndProcStatic(WIDGET_HANDLE handle, UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	ATOM RegisterClass(const wchar* pszClassName);
};

} // namespace chuwin32

#endif // ndef CHUI_GUI_H
