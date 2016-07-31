/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_ASKFORFILENAMEDLG_H
#define PO_ASKFORFILENAMEDLG_H

// Dialog box used to ask for a file name when creating a screenshot

class AskForFileNameDlg : public Dialog
{
public:
	String m_fileName;        // [in/out] The result file name (may be a file path)
	String m_screenshotDir;   // [in] (mainly used to check previous file existence)
	bool   m_incrementFileName; // [in] Set to true to change the file name upon dialog creation, 
	                            // example : Button.png -> Button 2.png

public:
	AskForFileNameDlg();

private:
	EditBox	m_editFileName;
	Window  m_staticWarning;

private:
	virtual bool SetupUI();
	virtual void SetupConnections();
	virtual void LoadValues();
	virtual bool StoreValues();

	void IncrementFileName();
	bool GetDigitGroup(const String& str, int& groupStart, int& groupStop);
};

#endif // ndef PO_DLGASKFORFILENAME_H
