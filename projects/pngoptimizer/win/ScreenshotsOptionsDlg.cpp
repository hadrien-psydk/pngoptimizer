/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ScreenshotsOptionsDlg.h"

///////////////////////////////////////////////////////////////////////////////
ScreenshotsOptionsDlg::ScreenshotsOptionsDlg() : Dialog(IDD_SCREENSHOTSOPTIONS)
{
}

///////////////////////////////////////////////////////////////////////////////
bool ScreenshotsOptionsDlg::SetupUI()
{
	m_radDefault = GetItem(IDC_RADIO_DEFAULT);
	m_radCustom = GetItem(IDC_RADIO_CUSTOM);
	m_btBrowse = GetItem(IDC_BUTTON_BROWSE);
	m_chkMaximizeCompression = GetItem(IDC_CHECK_MAXIMIZECOMPRESSION);
	m_chkAskForFilename = GetItem(IDC_CHECK_ASKFORFILENAME);
	m_editDirectory = GetItem(IDC_EDIT_DIRECTORY);
	m_editDirectory.SetSameFontThanParent();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void ScreenshotsOptionsDlg::SetupConnections()
{
	m_radDefault.Toggled.Connect(this, &ScreenshotsOptionsDlg::OnCheckDefault);
	m_radCustom.Toggled.Connect(this, &ScreenshotsOptionsDlg::OnCheckCustom);
	m_btBrowse.Clicked.Connect(this, &ScreenshotsOptionsDlg::OnButtonBrowse);
}

///////////////////////////////////////////////////////////////////////////////
void ScreenshotsOptionsDlg::LoadValues()
{
	m_editDirectory.SetText(m_settings.customDir);

	SyncRadioStates();
	m_chkMaximizeCompression.Check(m_settings.maximizeCompression);
	m_chkAskForFilename.Check(m_settings.askForFileName);
}

///////////////////////////////////////////////////////////////////////////////
bool ScreenshotsOptionsDlg::StoreValues()
{
	m_settings.useDefaultDir = m_radDefault.IsChecked();
	m_settings.customDir = m_editDirectory.GetText();

	if( !m_settings.useDefaultDir && m_settings.customDir.IsEmpty() )
	{
		MsgDialog md("Please set a directory", GetText(), CMT_Warning, CBT_Ok);
		md.DoModal(this);
		m_editDirectory.SetFocus();
		return false;
	}

	m_settings.maximizeCompression = m_chkMaximizeCompression.IsChecked();
	m_settings.askForFileName = m_chkAskForFilename.IsChecked();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
void ScreenshotsOptionsDlg::OnCheckDefault()
{
	m_settings.useDefaultDir = true;
	SyncRadioStates();
}

void ScreenshotsOptionsDlg::OnCheckCustom()
{
	m_settings.useDefaultDir = false;
	SyncRadioStates();

	// Put the focus to the custom directory editbox and select the whole content
	m_editDirectory.SetFocus();
	m_editDirectory.SetSelAll();
}

void ScreenshotsOptionsDlg::SyncRadioStates()
{
	m_radDefault.Check(m_settings.useDefaultDir);
	m_radCustom.Check( !m_settings.useDefaultDir);
	m_editDirectory.Enable( !m_settings.useDefaultDir);
	m_btBrowse.Enable( !m_settings.useDefaultDir);
}

void ScreenshotsOptionsDlg::OnButtonBrowse()
{
	DirDlg dlg;
	dlg.SetTitle(L"Please choose the screenshots directory");

	// Show the directory in the dialog
	String strDir = m_editDirectory.GetText();
	FilePath::AddSeparator(strDir, L'\\');

	// The / is a valid path, but Windows does not like it, so we convert it
	// into something it likes, "D:\" "E:\" etc.
	strDir = File::GetAbsolutePath(strDir);
	dlg.SetStartDir(strDir);

	if( dlg.DoModal(this) == DialogResp::Cancel )
		return;

	m_settings.customDir = dlg.GetDir();
	m_editDirectory.SetText(m_settings.customDir);
}
