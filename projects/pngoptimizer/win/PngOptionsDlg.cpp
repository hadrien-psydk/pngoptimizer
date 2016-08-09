/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PngOptionsDlg.h"
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////
PngOptionsDlg::PngOptionsDlg() : Dialog(IDD_PNGOPTIONS)
{
	m_ppSyncInProgress = false;
}


///////////////////////////////////////////////////////////////////////////////
bool PngOptionsDlg::SetupUI()
{
	m_chkBackupOldPngFiles = GetItem(IDC_CHECK_BACKUPOLDPNGFILES);
	m_chkKeepInterlacing = GetItem(IDC_CHECK_KEEPINTERLACING);
	m_chkAvoidGreyWith = GetItem(IDC_CHECK_AVOIDGREYWITHSIMPLETRANSPARENCY);
	m_chkIgnoreAnimatedGifs = GetItem(IDC_CHECK_IGNOREANIMATEDGIFS);
	m_chkKeepFileDate = GetItem(IDC_CHECK_KEEPFILEDATE);

	m_radBkColorRemove = GetItem(IDC_RADIO_BKCOLOR_REMOVE);
	m_radBkColorKeep = GetItem(IDC_RADIO_BKCOLOR_KEEP);
	m_radBkColorForce = GetItem(IDC_RADIO_BKCOLOR_FORCE);
	Window wnd = GetItem(IDC_STATIC_BKCOL);
	m_cbtBkColor.Create(wnd.GetRelativeRect(), this, IDC_COLBUTTON_BKCOL);
	wnd.Hide();
	m_lblBkColorTxtDec = GetItem(IDC_STATIC_BKCOLDEC);
	m_lblBkColorTxtHex = GetItem(IDC_STATIC_BKCOLHEX);

	m_radTextRemove = GetItem(IDC_RADIO_TEXT_REMOVE);
	m_radTextKeep = GetItem(IDC_RADIO_TEXT_KEEP);
	m_radTextForce = GetItem(IDC_RADIO_TEXT_FORCE);
	m_comboTextKeyword = GetItem(IDC_COMBO_TEXT_KEYWORD);
	m_editTextData = GetItem(IDC_EDIT_TEXT_DATA);

	// pHYs chunk
	m_radPhysRemove = GetItem(IDC_RADIO_PHYS_REMOVE);
	m_radPhysKeep = GetItem(IDC_RADIO_PHYS_KEEP);
	m_radPhysForce = GetItem(IDC_RADIO_PHYS_FORCE);
	m_editPpmX = GetItem(IDC_EDIT_PPMX);
	m_editPpmY = GetItem(IDC_EDIT_PPMY);
	m_editPpiX = GetItem(IDC_EDIT_PPIX);
	m_editPpiY = GetItem(IDC_EDIT_PPIY);

	// Fill the combo
	static const char* keywords[] = {
		"Author",
		"Comment",
		"Copyright",
		"Creation Time",
		"Description",
		"Disclaimer",
		"Software",
		"Source",
		"Title",
		"Warning"
	};
	const int count = ARRAY_SIZE(keywords);
	for(int i = 0; i < count; ++i)
	{
		m_comboTextKeyword.AddString(keywords[i]);
	}
	m_comboTextKeyword.LimitText(79);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void PngOptionsDlg::SetupConnections()
{
	m_radBkColorRemove.Toggled.Connect(this, &PngOptionsDlg::OnCheckBkColor);
	m_radBkColorKeep.Toggled.Connect(this, &PngOptionsDlg::OnCheckBkColor);
	m_radBkColorForce.Toggled.Connect(this, &PngOptionsDlg::OnCheckBkColor);
	m_cbtBkColor.ColorSet.Connect(this, &PngOptionsDlg::OnSelectBkColor);

	m_radTextRemove.Toggled.Connect(this, &PngOptionsDlg::OnCheckText);
	m_radTextKeep.Toggled.Connect(this, &PngOptionsDlg::OnCheckText);
	m_radTextForce.Toggled.Connect(this, &PngOptionsDlg::OnCheckText);

	m_radPhysRemove.Toggled.Connect(this, &PngOptionsDlg::OnCheckPhys);
	m_radPhysKeep.Toggled.Connect(this, &PngOptionsDlg::OnCheckPhys);
	m_radPhysForce.Toggled.Connect(this, &PngOptionsDlg::OnCheckPhys);

	m_editPpmX.Changed.Connect(this, &PngOptionsDlg::SyncPpiFromPpm);
	m_editPpmY.Changed.Connect(this, &PngOptionsDlg::SyncPpiFromPpm);
	m_editPpiX.Changed.Connect(this, &PngOptionsDlg::SyncPpmFromPpi);
	m_editPpiY.Changed.Connect(this, &PngOptionsDlg::SyncPpmFromPpi);
}

///////////////////////////////////////////////////////////////////////////////
// Sets the control values from attributes
void PngOptionsDlg::LoadValues()
{
	m_chkBackupOldPngFiles.Check(m_settings.backupOldPngFiles);
	m_chkKeepInterlacing.Check(m_settings.keepInterlacing);
	m_chkAvoidGreyWith.Check(m_settings.avoidGreyWithSimpleTransparency);
	m_chkIgnoreAnimatedGifs.Check(m_settings.ignoreAnimatedGifs);
	m_chkKeepFileDate.Check(m_settings.keepFileDate);

	m_radBkColorRemove.Check(m_settings.bkgdOption == POChunk_Remove);
	m_radBkColorKeep.Check(m_settings.bkgdOption == POChunk_Keep);
	m_radBkColorForce.Check(m_settings.bkgdOption == POChunk_Force);
	SetColorControls(m_settings.bkgdColor);
	SetColorControlStates();

	m_radTextRemove.Check(m_settings.textOption == POChunk_Remove);
	m_radTextKeep.Check(m_settings.textOption == POChunk_Keep);
	m_radTextForce.Check(m_settings.textOption == POChunk_Force);
	m_comboTextKeyword.SetText(m_settings.textKeyword);
	m_editTextData.SetText(m_settings.textData);
	SetTextControlStates();

	m_radPhysRemove.Check(m_settings.physOption == POChunk_Remove);
	m_radPhysKeep.Check(m_settings.physOption == POChunk_Keep);
	m_radPhysForce.Check(m_settings.physOption == POChunk_Force);
	m_editPpmX.SetTextInt(m_settings.physPpmX);
	m_editPpmY.SetTextInt(m_settings.physPpmY);
	m_editPpiX.SetTextInt( POEngineSettings::PpiFromPpm(m_settings.physPpmX) );
	m_editPpiY.SetTextInt( POEngineSettings::PpiFromPpm(m_settings.physPpmY) );
	SetPhysControlStates();
}

///////////////////////////////////////////////////////////////////////////////
// Returns true upon success
bool PngOptionsDlg::StoreValues()
{
	m_settings.backupOldPngFiles = m_chkBackupOldPngFiles.IsChecked();
	m_settings.keepInterlacing = m_chkKeepInterlacing.IsChecked();
	m_settings.avoidGreyWithSimpleTransparency = m_chkAvoidGreyWith.IsChecked();
	m_settings.ignoreAnimatedGifs = m_chkIgnoreAnimatedGifs.IsChecked();
	m_settings.keepFileDate = m_chkKeepFileDate.IsChecked();

	if( m_radBkColorRemove.IsChecked() )
	{
		m_settings.bkgdOption = POChunk_Remove;
	}
	if( m_radBkColorKeep.IsChecked() )
	{
		m_settings.bkgdOption = POChunk_Keep;
	}
	if( m_radBkColorForce.IsChecked() )
	{
		m_settings.bkgdOption = POChunk_Force;
	}
	m_settings.bkgdColor = m_cbtBkColor.GetColor();

	if( m_radTextRemove.IsChecked() )
	{
		m_settings.textOption = POChunk_Remove;
	}
	if( m_radTextKeep.IsChecked() )
	{
		m_settings.textOption = POChunk_Keep;
	}
	if( m_radTextForce.IsChecked() )
	{
		m_settings.textOption = POChunk_Force;
	}
	m_settings.textKeyword = m_comboTextKeyword.GetText();
	m_settings.textData = m_editTextData.GetText();

	if( m_radPhysRemove.IsChecked() )
	{
		m_settings.physOption = POChunk_Remove;
	}
	if( m_radPhysKeep.IsChecked() )
	{
		m_settings.physOption = POChunk_Keep;
	}
	if( m_radPhysForce.IsChecked() )
	{
		m_settings.physOption = POChunk_Force;
	}
	m_settings.physPpmX = m_editPpmX.GetTextInt();
	m_settings.physPpmY = m_editPpmY.GetTextInt();

	// Validity check
	if( m_settings.textOption == POChunk_Force && m_settings.textKeyword.IsEmpty() )
	{
		MsgDialog md("Please set the forced text keyword.", GetText(), CMT_Warning, CBT_Ok);
		md.DoModal(this);
		m_comboTextKeyword.SetFocus();
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void PngOptionsDlg::OnCheckBkColor()
{
	SetColorControlStates();
}

// Enable/Disable color controls according to BackgroundColor chunk option
void PngOptionsDlg::SetColorControlStates()
{
	m_cbtBkColor.Enable( m_radBkColorForce.IsChecked() );
	m_lblBkColorTxtDec.Enable( m_radBkColorForce.IsChecked() );
	m_lblBkColorTxtHex.Enable( m_radBkColorForce.IsChecked() );
}

void PngOptionsDlg::OnSelectBkColor()
{
	SetColorControls(m_cbtBkColor.GetColor());
}

///////////////////////////////////////////////////////
void PngOptionsDlg::OnCheckText()
{
	SetTextControlStates();
}

void PngOptionsDlg::SetTextControlStates()
{
	m_comboTextKeyword.Enable( m_radTextForce.IsChecked() );
	m_editTextData.Enable( m_radTextForce.IsChecked() );
}

///////////////////////////////////////////////////////
void PngOptionsDlg::OnCheckPhys()
{
	SetPhysControlStates();
}

void PngOptionsDlg::SetPhysControlStates()
{
	m_editPpmX.Enable( m_radPhysForce.IsChecked() );
	m_editPpmY.Enable( m_radPhysForce.IsChecked() );
	m_editPpiX.Enable( m_radPhysForce.IsChecked() );
	m_editPpiY.Enable( m_radPhysForce.IsChecked() );
}

void PngOptionsDlg::SetColorControls(Color col)
{
	m_cbtBkColor.SetColor(col);

	m_lblBkColorTxtDec.SetText("rgb(" 
		+ String::FromInt(col.r)
		+ ","
		+ String::FromInt(col.g)
		+ ","
		+ String::FromInt(col.b)
		+ ")");
	
	uint32 val = (col.r << 16) | (col.g << 8) | col.b;
	m_lblBkColorTxtHex.SetText( "#" + String::FromInt(val, 'x', 6, '0').Right(6) );
}

// Synchronize the PPI editboxes from the PPM editboxes
void PngOptionsDlg::SyncPpiFromPpm()
{
	if( m_ppSyncInProgress )
		return;
	m_ppSyncInProgress = true;
	int ppmX = m_editPpmX.GetTextInt();
	int ppmY = m_editPpmY.GetTextInt();
	m_editPpiX.SetTextInt( POEngineSettings::PpiFromPpm(ppmX) );
	m_editPpiY.SetTextInt( POEngineSettings::PpiFromPpm(ppmY) );
	m_ppSyncInProgress = false;
}

// Synchronize the PPM editboxes from the PPI editboxes
void PngOptionsDlg::SyncPpmFromPpi()
{
	if( m_ppSyncInProgress )
		return;
	m_ppSyncInProgress = true;
	int ppiX = m_editPpiX.GetTextInt();
	int ppiY = m_editPpiY.GetTextInt();
	m_editPpmX.SetTextInt( POEngineSettings::PpmFromPpi(ppiX) );
	m_editPpmY.SetTextInt( POEngineSettings::PpmFromPpi(ppiY) );
	m_ppSyncInProgress = false;
}

