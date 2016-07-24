/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////
#ifndef PO_DLGPNGOPTIONS_H
#define PO_DLGPNGOPTIONS_H

class DlgPngOptions : public Dialog
{
public:
	// Public settings [in,out]
	POEngineSettings m_settings;

public:
	int DoModal(HWND hParent);

	DlgPngOptions();

protected:
	// Controls
	CheckButton m_chkBackupOldPngFiles;
	CheckButton m_chkKeepInterlacing;
	CheckButton m_chkAvoidGreyWithSimpleTransparency;
	CheckButton m_chkIgnoreAnimatedGifs;
	CheckButton m_chkKeepFileDate;

	RadioButton m_radBkColorRemove;
	RadioButton m_radBkColorKeep;
	RadioButton m_radBkColorForce;
	ColorButton m_stBkColor;
	Window      m_stBkColorTxtDec;
	Window      m_stBkColorTxtHex;

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
	virtual LRESULT DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hCtl, LPARAM lParam);

	void OnCheckBkColor();
	void OnSelectBkColor();
	void SetColorControlStates();
	void SetColorControls(COLORREF cr);

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
