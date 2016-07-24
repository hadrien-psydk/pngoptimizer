/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_DLGSCREENSHOTSOPTIONS_H
#define PO_DLGSCREENSHOTSOPTIONS_H

class DlgScreenshotsOptions : public Dialog
{
public:
	// Public settings [in,out]
	bool           m_useDefaultDir;
	chustd::String m_customDir;
	bool           m_maximizeCompression;
	bool           m_askForFileName;

public:
	int DoModal(HWND hParent);

	DlgScreenshotsOptions();

protected:
	// Controls
	RadioButton	m_radDefault;
	RadioButton	m_radCustom;
	EditBox	    m_editDirectory;
	Button      m_btBrowse;

	CheckButton	m_chkMaximizeCompression;
	CheckButton	m_chkAskForFilename;

protected:
	virtual LRESULT DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hCtl, LPARAM lParam);

	void OnCheckDefault();
	void OnCheckCustom();
	void OnButtonBrowse();

	// Returns true if no problem
	bool StoreControlValues();
	void SyncControlStates();
	void SyncRadioStates();
};

#endif // ndef PO_DLGSCREENSHOTSOPTIONS_H
