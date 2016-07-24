///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_CONTEXTMENU_H
#define CHUWIN32_CONTEXTMENU_H

#include "gui.h"

namespace chuwin32 {

class ContextMenu
{
public:
	bool Create();
	bool AddItem(const chustd::String& strText, int nId);
	void Show(int x, int y, HWND hOwner);
	void SetPosition(UINT nPosition);

	ContextMenu();
	~ContextMenu();

private:
	HMENU m_hMenu;
	UINT m_nPosition;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_CONTEXTMENU_H
