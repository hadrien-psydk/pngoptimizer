///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dialog.h"

///////////////////////////////////////////////////////////////////////////////
namespace chuwin32 {\

#if defined(_WIN32)
///////////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK DlgProcStatic(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Dialog* pDlg = nullptr;

	if( msg == WM_INITDIALOG )
	{
		// lParam contains the object pointer given when DialogBoxParamW was called
		pDlg = (Dialog*) lParam;
		ASSERT(pDlg);
		
		// Store the pointer for next messages
		::SetWindowLongPtr(hDlg, DWLP_USER, LONG_PTR(pDlg));
		
		// Store the handle in the object so we know the dialog is successfully created
		*pDlg = hDlg;
	}
	else
	{
		pDlg = (Dialog*)::GetWindowLongPtr(hDlg, DWLP_USER);
	}

	if( pDlg )
	{
		if( msg == WM_INITDIALOG )
		{
			bool autoFocus = pDlg->SetupUI();
			pDlg->SetupConnections();
			pDlg->LoadValues();
			pDlg->CenterWindow(true);
			return autoFocus;
		}
		else if( msg == WM_COMMAND )
		{
			int buttonId = LOWORD(wParam);
			switch( buttonId ) 
			{
			case IDOK:
				if( !pDlg->StoreValues() )
				{
					return TRUE;
				}
			case IDCANCEL:
				::EndDialog(hDlg, buttonId);
				return TRUE;
			}

			if( pDlg->DoCommand((HWND)lParam, wParam) )
			{
				return TRUE;
			}
		}
		else if( msg == WM_NOTIFY )
		{
			NMHDR* pNM = (NMHDR*)lParam;
			if( pNM->code == NM_CLICK || pNM->code == NM_RETURN )
			{
				if( pDlg->DoCommand(pNM->hwndFrom, (uintptr_t)pNM) )
				{
					return TRUE;
				}
			}
		}
		return pDlg->DlgProc(msg, wParam, lParam);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
uintptr_t Dialog::DlgProc(uint32, uintptr_t, uintptr_t)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Checks if we should delegate WM_COMMAND management to the widget that sent it
bool Dialog::DoCommand(WIDGET_HANDLE wh, uintptr_t param)
{
	Widget* pWidget = (Widget*)::GetWindowLongPtr(wh, GWLP_USERDATA);
	if( pWidget )
	{
		pWidget->OnCommand(param);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
DialogResp Dialog::DoModal(const Window* parent)
{
	INT_PTR ret = DialogBoxParamW(GetModuleHandle(nullptr),
		MAKEINTRESOURCE(m_id),
		parent->GetHandle(),
		(DLGPROC) DlgProcStatic,
		LPARAM(this)
		);
	return DialogResp(ret);
}

WIDGET_HANDLE Dialog::GetItem(int id)
{
	HWND hChild = GetDlgItem(m_handle, id);
	ASSERT(hChild);
	return hChild;
}

Rect Dialog::GetItemRect(int id)
{
	HWND hChild = GetDlgItem(m_handle, id);
		
	Widget wndItem = hChild;
	Rect rect = wndItem.GetWindowRect();
	ScreenToClient(rect);
	return rect;
}

#endif

///////////////////////////////////////////////////////////////////////////////
}
