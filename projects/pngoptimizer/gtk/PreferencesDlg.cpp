/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreferencesDlg.h"

#include "MsgDialog.h"

// The Makefile will create logo48.o with those references from logo48.png
extern "C" const char _binary_gtk_preferences_glade_start[];
extern "C" const char _binary_gtk_preferences_glade_end[];

///////////////////////////////////////////////////////////////////////////////
PreferencesDlg::PreferencesDlg()
{
	m_ppSyncInProgress = false;
}

///////////////////////////////////////////////////////////////////////////////
bool PreferencesDlg::SetupUI()
{
	if( !LoadTemplate(_binary_gtk_preferences_glade_start,
        _binary_gtk_preferences_glade_end) )
	{
		return false;
	}
	/*
	// Usefull when working on the glade file without rebuilding
	auto builder = gtk_builder_new();
	GError* err = nullptr;
	if( gtk_builder_add_from_file(builder, "preferences.glade", &err) == 0 )
	{
		g_error("gtk_builder_add_from_file failed: %s", err->message);
		g_error_free(err);
		return false;
	}*/

	m_chkBackupOldPngFiles =  GetItem("check_backupoldpngfiles");
	m_chkKeepInterlacing =    GetItem("check_keepinterlacing");
	m_chkAvoidGreyWith =      GetItem("check_avoidgreywith");
	m_chkIgnoreAnimatedGifs = GetItem("check_ignoreanimatedgifs");
	m_chkKeepFileDate =       GetItem("check_keepfiledate");

	m_radBkColorRemove = GetItem("radio_bkcolor_remove");
	m_radBkColorKeep =   GetItem("radio_bkcolor_keep");
	m_radBkColorForce =  GetItem("radio_bkcolor_force");
	m_cbtBkColor =       GetItem("colbutton_bkcol");
	m_lblBkColorTxtDec = GetItem("label_bkcoldec");
	m_lblBkColorTxtHex = GetItem("label_bkcolhex");

	m_radTextRemove =    GetItem("radio_text_remove");
	m_radTextKeep =      GetItem("radio_text_keep");
	m_radTextForce =     GetItem("radio_text_force");
	m_comboTextKeyword = GetItem("combo_text_keyword");
	m_editTextData =     GetItem("edit_text_data");

	// pHYs chunk
	m_radPhysRemove = GetItem("radio_phys_remove");
	m_radPhysKeep =   GetItem("radio_phys_keep");
	m_radPhysForce =  GetItem("radio_phys_force");
	m_editPpmX = GetItem("edit_ppmx");
	m_editPpmY = GetItem("edit_ppmy");
	m_editPpiX = GetItem("edit_ppix");
	m_editPpiY = GetItem("edit_ppiy");

	return true;
}


///////////////////////////////////////////////////////////////////////////////
void PreferencesDlg::SetupConnections()
{
	m_radBkColorRemove.Toggled.Connect(this, &PreferencesDlg::OnCheckBkColor);
	m_radBkColorKeep.Toggled.Connect(this, &PreferencesDlg::OnCheckBkColor);
	m_radBkColorForce.Toggled.Connect(this, &PreferencesDlg::OnCheckBkColor);
	m_cbtBkColor.ColorSet.Connect(this, &PreferencesDlg::OnSelectBkColor);

	m_radTextRemove.Toggled.Connect(this, &PreferencesDlg::OnCheckText);
	m_radTextKeep.Toggled.Connect(this, &PreferencesDlg::OnCheckText);
	m_radTextForce.Toggled.Connect(this, &PreferencesDlg::OnCheckText);

	m_radPhysRemove.Toggled.Connect(this, &PreferencesDlg::OnCheckPhys);
	m_radPhysKeep.Toggled.Connect(this, &PreferencesDlg::OnCheckPhys);
	m_radPhysForce.Toggled.Connect(this, &PreferencesDlg::OnCheckPhys);

	m_editPpmX.Changed.Connect(this, &PreferencesDlg::SyncPpiFromPpm);
	m_editPpmY.Changed.Connect(this, &PreferencesDlg::SyncPpiFromPpm);
	m_editPpiX.Changed.Connect(this, &PreferencesDlg::SyncPpmFromPpi);
	m_editPpiY.Changed.Connect(this, &PreferencesDlg::SyncPpmFromPpi);
}

///////////////////////////////////////////////////////////////////////////////
// Sets the control values from attributes
void PreferencesDlg::LoadValues()
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
bool PreferencesDlg::StoreValues()
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
		MsgDialog md("Please set the forced text keyword.", GetTitle(), CMT_Warning, CBT_Ok);
		md.DoModal(this);
		m_comboTextKeyword.SetFocus();
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void PreferencesDlg::OnCheckBkColor()
{
	SetColorControlStates();
}

// Enable/Disable color controls according to BackgroundColor chunk option
void PreferencesDlg::SetColorControlStates()
{
	m_cbtBkColor.Enable( m_radBkColorForce.IsChecked() );
	m_lblBkColorTxtDec.Enable( m_radBkColorForce.IsChecked() );
	m_lblBkColorTxtHex.Enable( m_radBkColorForce.IsChecked() );
}

void PreferencesDlg::OnSelectBkColor()
{
	SetColorControls(m_cbtBkColor.GetColor());
}

///////////////////////////////////////////////////////
void PreferencesDlg::OnCheckText()
{
	SetTextControlStates();
}

void PreferencesDlg::SetTextControlStates()
{
	m_comboTextKeyword.Enable( m_radTextForce.IsChecked() );
	m_editTextData.Enable( m_radTextForce.IsChecked() );
}

///////////////////////////////////////////////////////
void PreferencesDlg::OnCheckPhys()
{
	SetPhysControlStates();
}

void PreferencesDlg::SetPhysControlStates()
{
	m_editPpmX.Enable( m_radPhysForce.IsChecked() );
	m_editPpmY.Enable( m_radPhysForce.IsChecked() );
	m_editPpiX.Enable( m_radPhysForce.IsChecked() );
	m_editPpiY.Enable( m_radPhysForce.IsChecked() );
}

void PreferencesDlg::SetColorControls(Color col)
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
void PreferencesDlg::SyncPpiFromPpm()
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
void PreferencesDlg::SyncPpmFromPpi()
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
