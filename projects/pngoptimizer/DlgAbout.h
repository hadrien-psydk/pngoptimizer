/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////
#ifndef PO_DLGABOUT_H
#define PO_DLGABOUT_H

/////////////////////////////////////////////////////////////////////////////////////
class DlgAbout : public Dialog
{
public:
	int DoModal(HWND hParent);
	virtual LRESULT DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

	DlgAbout();
protected:
	SysLink m_syslinkUrl;
	HFONT m_hFont;
};

#endif
