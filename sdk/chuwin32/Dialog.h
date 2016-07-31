///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_DIALOG_H
#define CHUI_DIALOG_H

#include "Window.h"

namespace chuwin32 {

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
	Dialog(DIALOG_ID did) : Window(), m_id(did) {}
	Dialog(WIDGET_HANDLE handle) : Window(handle), m_id(0) {}
	void operator = (WIDGET_HANDLE handle) { m_handle = handle; }

	DialogResp DoModal(const Window* parent);
	WIDGET_HANDLE GetItem(int id);
	Rect GetItemRect(int id);

private:
	DIALOG_ID m_id;

public:
	bool DoCommand(WIDGET_HANDLE wh, uintptr_t param);
	virtual uintptr_t DlgProc(uint32 msg, uintptr_t param1, uintptr_t param2);

	virtual bool SetupUI() = 0; // true for automatic focus on child widget
	virtual void SetupConnections() = 0;
	virtual void LoadValues() = 0;
	virtual bool StoreValues() = 0; // true to actually exit the dialog
};

} // namespace chuwin32

#endif // ndef CHUI_DIALOG_H
