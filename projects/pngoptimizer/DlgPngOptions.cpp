/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DlgPngOptions.h"

///////////////////////////////////////////////////////////////////////////////

DlgPngOptions::DlgPngOptions()
{
	m_ppSyncInProgress = false;
}

LRESULT DlgPngOptions::DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
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

		case IDC_RADIO_BKCOLOR_REMOVE:
		case IDC_RADIO_BKCOLOR_KEEP:
		case IDC_RADIO_BKCOLOR_FORCE:
			OnCheckBkColor();
			return TRUE;
		case IDC_STATIC_BKCOL:
			OnSelectBkColor();
			return TRUE;

		case IDC_RADIO_TEXT_REMOVE:
		case IDC_RADIO_TEXT_KEEP:
		case IDC_RADIO_TEXT_FORCE:
			OnCheckText();
			return TRUE;

		case IDC_RADIO_PHYS_REMOVE:
		case IDC_RADIO_PHYS_KEEP:
		case IDC_RADIO_PHYS_FORCE:
			OnCheckPhys();
			return TRUE;
		
		case IDC_EDIT_PPMX:
		case IDC_EDIT_PPMY:
			if( HIWORD(wParam) == EN_CHANGE )
			{
				SyncPpiFromPpm();
				return TRUE;
			}
			return FALSE;
		case IDC_EDIT_PPIX:
		case IDC_EDIT_PPIY:
			if( HIWORD(wParam) == EN_CHANGE )
			{
				SyncPpmFromPpi();
				return TRUE;
			}
			return FALSE;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

BOOL DlgPngOptions::OnInitDialog(HWND, LPARAM)
{
	m_chkBackupOldPngFiles = GetItem(IDC_CHECK_BACKUPOLDPNGFILES);
	m_chkKeepInterlacing = GetItem(IDC_CHECK_KEEPINTERLACING);
	m_chkAvoidGreyWithSimpleTransparency = GetItem(IDC_CHECK_AVOIDGREYWITHSIMPLETRANSPARENCY);
	m_chkIgnoreAnimatedGifs = GetItem(IDC_CHECK_IGNOREANIMATEDGIFS);
	m_chkKeepFileDate = GetItem(IDC_CHECK_KEEPFILEDATE);

	m_radBkColorRemove = GetItem(IDC_RADIO_BKCOLOR_REMOVE);
	m_radBkColorKeep = GetItem(IDC_RADIO_BKCOLOR_KEEP);
	m_radBkColorForce = GetItem(IDC_RADIO_BKCOLOR_FORCE);
	Window wnd = GetItem(IDC_STATIC_BKCOL);
	m_stBkColor.Create(wnd.GetRelativeRect(), m_hWnd, IDC_STATIC_BKCOL);
	wnd.Hide();
	m_stBkColorTxtDec = GetItem(IDC_STATIC_BKCOLDEC);
	m_stBkColorTxtHex = GetItem(IDC_STATIC_BKCOLHEX);

	m_radTextRemove = GetItem(IDC_RADIO_TEXT_REMOVE);
	m_radTextKeep = GetItem(IDC_RADIO_TEXT_KEEP);
	m_radTextForce = GetItem(IDC_RADIO_TEXT_FORCE);
	m_comboTextKeyword = GetItem(IDC_COMBO_TEXT_KEYWORD);
	m_editTextData = GetItem(IDC_EDIT_TEXT_DATA);

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
	const int count = sizeof(keywords) / sizeof(keywords[0]);
	for(int i = 0; i < count; ++i)
	{
		m_comboTextKeyword.AddString(keywords[i]);
	}
	m_comboTextKeyword.LimitText(79);

	// pHYs chunk
	m_radPhysRemove = GetItem(IDC_RADIO_PHYS_REMOVE);
	m_radPhysKeep = GetItem(IDC_RADIO_PHYS_KEEP);
	m_radPhysForce = GetItem(IDC_RADIO_PHYS_FORCE);
	m_editPpmX = GetItem(IDC_EDIT_PPMX);
	m_editPpmY = GetItem(IDC_EDIT_PPMY);
	m_editPpiX = GetItem(IDC_EDIT_PPIX);
	m_editPpiY = GetItem(IDC_EDIT_PPIY);

	LoadControlValues();
	CenterWindow(true);

	return TRUE;
}

///////////////////////////////////////////////////////
void DlgPngOptions::OnCheckBkColor()
{
	SetColorControlStates();
}

// Enable/Disable color controls according to BackgroundColor chunk option
void DlgPngOptions::SetColorControlStates()
{
	m_stBkColor.Enable( m_radBkColorForce.IsChecked() );
	m_stBkColorTxtDec.Enable( m_radBkColorForce.IsChecked() );
	m_stBkColorTxtHex.Enable( m_radBkColorForce.IsChecked() );
}

void DlgPngOptions::OnSelectBkColor()
{
	COLORREF acr[16] = { 0xff };

	CHOOSECOLOR cc;
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = m_hWnd;
	cc.hInstance = 0;
	cc.rgbResult = m_stBkColor.GetColor();
	cc.lpCustColors = acr;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;

	if( !ChooseColor(&cc) )
	{
		return;
	}
	SetColorControls(cc.rgbResult);
}

///////////////////////////////////////////////////////
void DlgPngOptions::OnCheckText()
{
	SetTextControlStates();
}

void DlgPngOptions::SetTextControlStates()
{
	m_comboTextKeyword.Enable( m_radTextForce.IsChecked() );
	m_editTextData.Enable( m_radTextForce.IsChecked() );
}

///////////////////////////////////////////////////////
void DlgPngOptions::OnCheckPhys()
{
	SetPhysControlStates();
}

void DlgPngOptions::SetPhysControlStates()
{
	m_editPpmX.Enable( m_radPhysForce.IsChecked() );
	m_editPpmY.Enable( m_radPhysForce.IsChecked() );
	m_editPpiX.Enable( m_radPhysForce.IsChecked() );
	m_editPpiY.Enable( m_radPhysForce.IsChecked() );
}

// Sets the control values from attributes
void DlgPngOptions::LoadControlValues()
{
	m_chkBackupOldPngFiles.Check(m_settings.backupOldPngFiles);
	m_chkKeepInterlacing.Check(m_settings.keepInterlacing);
	m_chkAvoidGreyWithSimpleTransparency.Check(m_settings.avoidGreyWithSimpleTransparency);
	m_chkIgnoreAnimatedGifs.Check(m_settings.ignoreAnimatedGifs);
	m_chkKeepFileDate.Check(m_settings.keepFileDate);

	m_radBkColorRemove.Check(m_settings.bkgdOption == POChunk_Remove);
	m_radBkColorKeep.Check(m_settings.bkgdOption == POChunk_Keep);
	m_radBkColorForce.Check(m_settings.bkgdOption == POChunk_Force);
	COLORREF cr = RGB(m_settings.bkgdColor.r, m_settings.bkgdColor.g, m_settings.bkgdColor.b);
	SetColorControls(cr);
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

void DlgPngOptions::SetColorControls(COLORREF cr)
{
	m_stBkColor.SetColor(cr);

	int r = GetRValue(cr);
	int g = GetGValue(cr);
	int b = GetBValue(cr);

	m_stBkColorTxtDec.SetText("rgb(" 
		+ String::FromInt(r)
		+ ","
		+ String::FromInt(g)
		+ ","
		+ String::FromInt(b)
		+ ")");
	
	uint32 nVal = (r << 16) | (g << 8) | b;
	m_stBkColorTxtHex.SetText( "#" + String::FromInt(nVal, 'x', 6, '0').Right(6) );
}

// Returns true upon success
bool DlgPngOptions::StoreControlValues()
{
	m_settings.backupOldPngFiles = m_chkBackupOldPngFiles.IsChecked();
	m_settings.keepInterlacing = m_chkKeepInterlacing.IsChecked();
	m_settings.avoidGreyWithSimpleTransparency = m_chkAvoidGreyWithSimpleTransparency.IsChecked();
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
	COLORREF cr = m_stBkColor.GetColor();
	uint8 r = GetRValue(cr);
	uint8 g = GetGValue(cr);
	uint8 b = GetBValue(cr);
	m_settings.bkgdColor.SetRgb(r, g, b);

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
		::MessageBoxW(m_hWnd, L"Please set the forced text keyword.", GetText().GetBuffer(), MB_ICONEXCLAMATION | MB_OK);
		m_comboTextKeyword.SetFocus();
		return false;
	}
	return true;
}

int DlgPngOptions::DoModal(HWND hParent)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	return DoModalHelper(hInstance, IDD_PNGOPTIONS, hParent);
}

// Synchronize the PPI editboxes from the PPM editboxes
void DlgPngOptions::SyncPpiFromPpm()
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
void DlgPngOptions::SyncPpmFromPpi()
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

