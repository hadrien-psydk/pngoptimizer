/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_DIALOG_H
#define PO_DIALOG_H

#include "Widgets.h"

enum class DialogResp
{
	None,
	Ok,
	Cancel,
	Abort,
	Retry,
	Ignore,
	Yes,
	No
};

class Dialog : public Window
{
public:
	Dialog();
	DialogResp DoModal(Window* parent);

protected:
	virtual bool SetupUI() = 0; // true for automatic focus on child widget
	virtual void SetupConnections() {};
	virtual void LoadValues() {};
	virtual bool StoreValues() { return true; }; // true to actually exit the dialog

protected:
	GtkBuilder* m_builder;
	Window* m_parent;

protected:
	bool LoadTemplate(const char* begin, const char* end);
	WIDGET_HANDLE GetItem(const char* name);
};

#endif
