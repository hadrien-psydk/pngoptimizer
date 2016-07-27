///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_DIALOG_H
#define CHUWIN32_DIALOG_H

#include "gui.h"

namespace chuwin32 {

class Dialog : public Window
{
public:
	Dialog() : Window(nullptr) {}
	Dialog(HWND hWnd) : Window(hWnd) {}
	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	HWND GetItem(int nID) { HWND hChild = GetDlgItem(m_hWnd, nID); ASSERT(hChild); return hChild; }

	RECT GetItemRect(int nID)
	{
		HWND hChild = GetDlgItem(m_hWnd, nID);
		
		Window wndItem = hChild;
		RECT rect = wndItem.GetWindowRect();
		ScreenToClient(rect);

		return rect;
	}

	void EndDialog(int nReturn)
	{
		::EndDialog(m_hWnd, nReturn);
	}

	int DoModalHelper(HINSTANCE hInstance, int nDialogId, HWND hParent);

protected:
	virtual LRESULT DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	
private:
	static LRESULT CALLBACK DlgProcStatic(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace chuwin32

#endif // ndef CHUWIN32_DIALOG_H
