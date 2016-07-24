/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_BMPCLIPBOARDDUMPER_H
#define PO_BMPCLIPBOARDDUMPER_H

class POEngine;

// BmpClipboardDumper class settings
struct BmpcdSettings
{
	bool   useDefaultDir;
	String customDir;
	bool   askForFileName; // Note: this only fires m_eventDumpStateChanged, no dialog involved
	bool   maximizeCompression;

	BmpcdSettings()
	{
		useDefaultDir = true;
		askForFileName = false;
		// Users feedback reveals that they expect this parameter to be true
		// (maximize compression when creating a screenshot)
		maximizeCompression = true;
	}
};

// This class allows the creation of a PNG from a bitmap stored in the Windows clipboard
class BmpClipboardDumper
{
public:
	enum DumpState
	{
		DS_AskForFileName,
		DS_Creating
	};
	Event1<DumpState> DumpStateChanged;

public:
	BmpcdSettings m_settings;

public:
	// Returns true upon success, false otherwise
	bool Dump(POEngine* pEngine);
	String GetLastError() const;

	String GetPreviousFileName() const;
	String GetFileName() const;
	String GetFilePath() const;

	// Call those methods after a dsAskForFileName event state 
	void SetFileName(const String& strFileName);
	void Abort();

	bool IsBmpAvailable() const;

	String GetDir() const;

	BmpClipboardDumper();
	virtual ~BmpClipboardDumper();

private:
	String m_strErr;
	String m_strPreviousFileName; // Filled after a successfull dump
	String m_strFileName; // File name (without the path) of the created PNG
	String m_strFilePath; // File path of the created PNG
	bool m_bAbort;

private:
	bool PrepareDib(chuwin32::DibBitmap& dib);
	String GetTempDir() const;
};

#endif // ndef PO_BMPCLIPBOARDDUMPER_H
