/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_SCREENSHOTSOPTIONSDLG_H
#define PO_SCREENSHOTSOPTIONSDLG_H

#include "BmpcdSettings.h"

class ScreenshotsOptionsDlg : public Dialog
{
public:
	// Public settings [in,out]
	BmpcdSettings m_settings;

public:
	ScreenshotsOptionsDlg();

protected:
	RadioButton	m_radDefault;
	RadioButton	m_radCustom;
	EditBox	    m_editDirectory;
	Button      m_btBrowse;

	CheckButton	m_chkMaximizeCompression;
	CheckButton	m_chkAskForFilename;

protected:
	virtual bool SetupUI();
	virtual void SetupConnections();
	virtual void LoadValues();
	virtual bool StoreValues();

	void OnCheckDefault();
	void OnCheckCustom();
	void OnButtonBrowse();

	void SyncRadioStates();
};

#endif // ndef PO_DLGSCREENSHOTSOPTIONS_H
