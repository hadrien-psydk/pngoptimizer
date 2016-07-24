/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DlgScreenshotsOptions.h"

///////////////////////////////////////////////////////////////////////////////

DlgScreenshotsOptions::DlgScreenshotsOptions()
{
	m_useDefaultDir = true;
}

LRESULT DlgScreenshotsOptions::DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch(nMsg)
	{
	case WM_INITDIALOG:
		return OnInitDialog(HWND(wParam), lParam);

	case WM_COMMAND: 
		switch( LOWORD(wParam)) 
		{ 
		case IDCANCEL:
			EndDialog(IDCANCEL);
			return TRUE;

		case IDOK:
			if( StoreControlValues() )
			{
				EndDialog(IDOK);
			}
			return TRUE;

		case IDC_RADIO_DEFAULT:
			OnCheckDefault();
			return TRUE;

		case IDC_RADIO_CUSTOM:
			OnCheckCustom();
			return TRUE;

		case IDC_BUTTON_BROWSE:
			OnButtonBrowse();
			return TRUE;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

BOOL DlgScreenshotsOptions::OnInitDialog(HWND, LPARAM)
{
	m_radDefault = GetItem(IDC_RADIO_DEFAULT);
	m_radCustom = GetItem(IDC_RADIO_CUSTOM);
	m_btBrowse = GetItem(IDC_BUTTON_BROWSE);
	
	m_chkMaximizeCompression = GetItem(IDC_CHECK_MAXIMIZECOMPRESSION);
	m_chkAskForFilename = GetItem(IDC_CHECK_ASKFORFILENAME);

	m_editDirectory = GetItem(IDC_EDIT_DIRECTORY);

	m_editDirectory.SetText(m_customDir);
	m_editDirectory.SetSameFontThanParent();

	SyncControlStates();
	CenterWindow(true);

	return TRUE;
}

void DlgScreenshotsOptions::OnCheckDefault()
{
	m_useDefaultDir = true;
	SyncRadioStates();
}

void DlgScreenshotsOptions::OnCheckCustom()
{
	m_useDefaultDir = false;
	SyncRadioStates();

	// Put the focus to the custom directory editbox and select the whole content
	m_editDirectory.SetFocus();
	m_editDirectory.SetSelAll();
}

void DlgScreenshotsOptions::SyncControlStates()
{
	SyncRadioStates();
	m_chkMaximizeCompression.Check(m_maximizeCompression);
	m_chkAskForFilename.Check(m_askForFileName);
}

void DlgScreenshotsOptions::SyncRadioStates()
{
	m_radDefault.Check(m_useDefaultDir);
	m_radCustom.Check( !m_useDefaultDir);
	m_editDirectory.Enable( !m_useDefaultDir);
	m_btBrowse.Enable( !m_useDefaultDir);
}

bool DlgScreenshotsOptions::StoreControlValues()
{
	m_useDefaultDir = m_radDefault.IsChecked();
	m_customDir = m_editDirectory.GetText();

	if( !m_useDefaultDir && m_customDir.IsEmpty() )
	{
		MessageBoxW(m_hWnd, L"Please set a directory", GetText().GetBuffer(), MB_OK);
		m_editDirectory.SetFocus();
		return false;
	}

	m_maximizeCompression = m_chkMaximizeCompression.IsChecked();
	m_askForFileName = m_chkAskForFilename.IsChecked();

	return true;
}

void DlgScreenshotsOptions::OnButtonBrowse()
{
	DirDlg dlg(m_hWnd);
	dlg.SetTitle(L"Please choose the screenshots directory");
	
	// Show the directory in the dialog
	String strDir = m_editDirectory.GetText();
	FilePath::AddSeparator(strDir, L'\\');
	
	// The / is a valid path, but Windows does not like it, so we convert it
	// into something it likes, "D:\" "E:\" etc.
	strDir = File::GetAbsolutePath(strDir);
	dlg.SetStartDir(strDir);

	if( dlg.DoModal() == IDCANCEL )
		return;

	m_customDir = dlg.GetDir();
	m_editDirectory.SetText(m_customDir);
}

int DlgScreenshotsOptions::DoModal(HWND hParent)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	return DoModalHelper(hInstance, IDD_SCREENSHOTSOPTIONS, hParent);
}
