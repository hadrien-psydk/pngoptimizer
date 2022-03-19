/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_PREFERENCESDLG_H
#define PO_PREFERENCESDLG_H

#include "Dialog.h"
#include "Widgets.h"

class PreferencesDlg : public Dialog
{
public:
	// Public settings [in,out]
	POEngineSettings m_settings;

public:
	PreferencesDlg();

protected:
	CheckButton m_chkBackupOldPngFiles;
	CheckButton m_chkKeepInterlacing;
	CheckButton m_chkAvoidGreyWith;
	CheckButton m_chkIgnoreAnimatedGifs;
	CheckButton m_chkKeepFileDate;

	RadioButton m_radBkColorRemove;
	RadioButton m_radBkColorKeep;
	RadioButton m_radBkColorForce;
	ColorButton m_cbtBkColor;
	Label       m_lblBkColorTxtDec;
	Label       m_lblBkColorTxtHex;

	RadioButton m_radTextRemove;
	RadioButton m_radTextKeep;
	RadioButton m_radTextForce;
	ComboBox    m_comboTextKeyword;
	EditBox     m_editTextData;

	RadioButton m_radPhysRemove;
	RadioButton m_radPhysKeep;
	RadioButton m_radPhysForce;
	EditBox     m_editPpmX;
	EditBox     m_editPpmY;
	EditBox     m_editPpiX;
	EditBox     m_editPpiY;

protected:
	virtual bool SetupUI();
	virtual void SetupConnections();
	virtual void LoadValues();
	virtual bool StoreValues();

private:
	bool m_ppSyncInProgress;

private:
	void OnCheckBkColor();
	void SetColorControlStates();
	void OnSelectBkColor();
	void OnCheckText();
	void SetTextControlStates();
	void OnCheckPhys();
	void SetPhysControlStates();
	void SetColorControls(Color col);
	void SyncPpiFromPpm();
	void SyncPpmFromPpi();
};

#endif
