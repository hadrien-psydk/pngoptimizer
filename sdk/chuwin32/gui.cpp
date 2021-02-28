///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gui.h"

namespace chuwin32 {\

#ifndef _WIN32
static void OnCheckButtonStatic(GtkWidget* widget, gpointer userData)
{
	CheckButton* that = (CheckButton*)userData;
	that->Toggled.Fire();
}
#endif

///////////////////////////////////////////////////////////////////////////////
void CheckButton::operator=(WIDGET_HANDLE handle)
{
	SetHandle(handle);
#ifndef _WIN32
	g_signal_connect(handle, "toggled", G_CALLBACK(OnCheckButtonStatic), this);
#endif
}

void CheckButton::Check(bool check)
{
	SendMessage(m_handle, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool CheckButton::IsChecked() const
{
	return (SendMessage(m_handle, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void CheckButton::OnCommand(uintptr_t)
{
	Toggled.Fire();
}

///////////////////////////////////////////////////////////////////////////////
bool EditBox::Create(const Rect& rc, const Widget* parent, int id)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	int nExStyle = WS_EX_CLIENTEDGE;
	int nStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	
	m_handle = ::CreateWindowExW(
		nExStyle,			// extended window style
		WC_EDITW,	// registered class name
		L"",	// window name
		nStyle,	// window style
		rc.x1,			// horizontal position of window
		rc.y1,			// vertical position of window
		rc.x2 - rc.x1,		// window width
		rc.y2 - rc.y1,		// window height
		parent->GetHandle(),		// handle to parent or owner window
		HMENU( LongToHandle(id)),		// menu handle or child identifier
		hInstance,		// handle to application instance (Windows NT/2000/XP: This value is ignored)
		nullptr		// window-creation data
		);

	return m_handle != nullptr;
}

void EditBox::SetSel(int start, int stop)
{
	::SendMessage(m_handle, EM_SETSEL, start, stop);
}

void EditBox::SetSelAll()
{
	SetSel(0, -1);
}

void EditBox::OnCommand(uintptr_t param)
{
	if( HIWORD(param) == EN_CHANGE )
	{
		Changed.Fire();
	}
}

///////////////////////////////////////////////////////////////////////////////
void Button::OnCommand(uintptr_t)
{
	Clicked.Fire();
}

///////////////////////////////////////////////////////////////////////////////
int ComboBox::AddString(const chustd::String& str)
{
	return (int)::SendMessage(m_handle, CB_ADDSTRING, 0, (LPARAM) str.GetBuffer());
}

void ComboBox::SetItemData(int nIndex, uint32 nData)
{
	::SendMessage(m_handle, CB_SETITEMDATA, nIndex, nData);
}

int ComboBox::GetCurSel()
{
	return (int)::SendMessage(m_handle, CB_GETCURSEL, 0, 0);
}

uint32 ComboBox::GetItemData(int nIndex)
{
	return (uint32)::SendMessage(m_handle, CB_GETITEMDATA, nIndex, 0);
}

void ComboBox::LimitText(int limit)
{
	::SendMessage(m_handle, CB_LIMITTEXT, limit, 0);
}

///////////////////////////////////////////////////////////////////////////////
void Slider::SetRangeMin(int min, bool redraw)
{
	::SendMessage(m_handle, TBM_SETRANGEMIN, BOOL(redraw), min);
}

void Slider::SetRangeMax(int max, bool redraw)
{
	::SendMessage(m_handle, TBM_SETRANGEMAX, BOOL(redraw), max);
}

void Slider::SetRange(int min, int max, bool redraw)
{
	SetRangeMin(min, redraw);
	SetRangeMax(max, redraw);
}

void Slider::SetPos(int pos)
{
	::SendMessage(m_handle, TBM_SETPOS, TRUE, pos);
}

int Slider::GetPos()
{
	return (int) ::SendMessage(m_handle, TBM_GETPOS, 0, 0L);
}

///////////////////////////////////////////////////////////////////////////////
bool SysLink::Create(const Rect& rc, const Widget* parent, int id)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	int nStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE;

	HWND handle = CreateWindowExW(0, WC_LINK, L"", nStyle,
		rc.x1, rc.y1, rc.x2 - rc.x1, rc.y2 - rc.y1, // x,y,w,h
		parent->GetHandle(),	
		HMENU( LongToHandle(id)),
		hInstance, // handle to application instance (Windows NT/2000/XP: This value is ignored)
		nullptr // window-creation data
		);
	SetHandle(handle);
	return handle != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
void SysLink::OnCommand(uintptr_t param)
{
	// Open the link in default browser
	PNMLINK pNMLink = (PNMLINK)param;
	LITEM item = pNMLink->item;
	if( item.iLink == 0 )
	{
		ShellExecute(nullptr, L"open", item.szUrl, nullptr, nullptr, SW_SHOW);
	}
}


///////////////////////////////////////////////////////////////////////////////
}
