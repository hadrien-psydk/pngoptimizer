///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_CONTEXTMENU_H
#define CHUI_CONTEXTMENU_H

#include "gui.h"

namespace chuwin32 {

class ContextMenu
{
public:
	bool Create();
	bool AddItem(const chustd::String& text, int id);
	void Show(int x, int y, const Widget* owner);
	void SetPosition(int position);

	ContextMenu();
	~ContextMenu();

private:
	HMENU m_hMenu;
	int m_position;
};

} // namespace chuwin32

#endif
