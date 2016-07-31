///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_WINDOW_H
#define CHUI_WINDOW_H

#include "gui.h"

namespace chuwin32 {\

class Window : public Widget
{
public:
	Window() : Widget(nullptr) {}
	Window(WIDGET_HANDLE handle) : Widget(handle) {}
	void operator = (WIDGET_HANDLE handle) { m_handle = handle; }
};

} // namespace chuwin32

#endif
