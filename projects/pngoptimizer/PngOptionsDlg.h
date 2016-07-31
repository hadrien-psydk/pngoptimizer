/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_PNGOPTIONSDLG_H
#define PO_PNGOPTIONSDLG_H

class PngOptionsDlg : public Dialog
{
public:
	// Public settings [in,out]
	POEngineSettings m_settings;

public:
	PngOptionsDlg();

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

	void OnCheckBkColor();
	void OnSelectBkColor();
	void SetColorControlStates();
	void SetColorControls(Color col);

	void OnCheckText();
	void SetTextControlStates();

	void OnCheckPhys();
	void SetPhysControlStates();

	void LoadControlValues();
	bool StoreControlValues();

	void SyncPpiFromPpm();
	void SyncPpmFromPpi();

private:
	bool m_ppSyncInProgress; // true if Synch function being called
};

#endif // ndef PO_DLGPNGOPTIONS_H
