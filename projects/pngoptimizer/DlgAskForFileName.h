/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_DLGASKFORFILENAME_H
#define PO_DLGASKFORFILENAME_H

// Dialog box used to ask for a file name when creating a screenshot

class DlgAskForFileName : public chuwin32::Dialog
{
public:
	String m_strFileName;        // [in/out] The result file name (may be a file path)
	String m_strScreenshotDir;   // [in] (mainly used to check previous file existence)
	bool   m_bIncrementFileName; // [in] Set to true to change the file name upon dialog creation, 
	                             // example : Button.png -> Button 2.png

public:
	int DoModal(HWND hParent);

	DlgAskForFileName();
	virtual ~DlgAskForFileName();

protected:
	virtual LRESULT DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

private:
	EditBox	m_editFileName;
	Window  m_staticWarning;

private:
	BOOL OnInitDialog(HWND hCtl, LPARAM lParam);
	bool StoreControlValues();
	void IncrementFileName();
	bool GetDigitGroup(const String& str, int& nGroupStart, int& nGroupStop);
};

#endif // ndef PO_DLGASKFORFILENAME_H
