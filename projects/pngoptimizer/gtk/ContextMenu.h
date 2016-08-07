/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_CONTEXTMENU_H
#define PO_CONTEXTMENU_H

#include "Widgets.h"

class MenuItem
{
public:
	Event0 Activate;

public:
	void SetHandle(WIDGET_HANDLE handle);
private:
	WIDGET_HANDLE m_handle;
private:
	static void OnActivate(WIDGET_HANDLE, gpointer);
};

class ContextMenu
{
public:
	void Create();
	MenuItem* AddItem(const char* text);
	void AddSeparator();
	void Install(Window* owner);

private:
	WIDGET_HANDLE m_handle;
	chustd::Array<MenuItem> m_items;
};

#endif
