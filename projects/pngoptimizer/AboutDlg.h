/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_ABOUTDLG_H
#define PO_ABOUTDLG_H

class AboutDlg : public Dialog
{
public:
	AboutDlg();

protected:
	SysLink m_syslinkUrl;
	FONT_HANDLE m_hFont;

protected:
	virtual bool SetupUI();
	virtual void SetupConnections();
	virtual void LoadValues();
	virtual bool StoreValues();
};

#endif
