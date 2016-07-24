///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dialog.h"

///////////////////////////////////////////////////////////////////////////////
using namespace chuwin32;
///////////////////////////////////////////////////////////////////////////////

LRESULT Dialog::DlgProc(UINT nMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	// Default behaviour
	if( nMsg == WM_COMMAND )
	{
		int nId = LOWORD(wParam);
		switch( nId ) 
		{
		case IDCANCEL:
		case IDOK:
			EndDialog(nId);
			return TRUE;
		}
	}
	return FALSE;
}

LRESULT CALLBACK Dialog::DlgProcStatic(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	Dialog* pDlg = NULL;

	if( nMsg == WM_INITDIALOG )
	{
		// lParam contains the object pointer given when DialogBoxParamW was called
		pDlg = (Dialog*) lParam;
		ASSERT(pDlg != NULL);
		
		// Store the pointer for next messages
		::SetWindowLongPtr(hDlg, DWLP_USER, LONG_PTR(pDlg));
		
		// Store the handle in the object so we know the dialog is successfully created
		pDlg->m_hWnd = hDlg;
	}
	else
	{
		LONG_PTR nLong = ::GetWindowLongPtr(hDlg, DWLP_USER);
		pDlg = (Dialog*) (nLong);
	}

	if( pDlg != NULL )
	{
		return pDlg->DlgProc(nMsg, wParam, lParam);
	}
	return FALSE;
}

int Dialog::DoModalHelper(HINSTANCE hInstance, int nDialogId, HWND hParent)
{
	INT_PTR ret = DialogBoxParamW(hInstance, 
		MAKEINTRESOURCE(nDialogId),
		hParent,
		(DLGPROC) DlgProcStatic,
		LPARAM(this)
		);
	return int(ret);
}
