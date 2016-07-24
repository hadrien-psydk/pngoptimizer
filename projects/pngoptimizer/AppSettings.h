/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_APPSETTINGS_H
#define PO_APPSETTINGS_H

class POApplication;

// Reads and writes settings for the POApplication class
class AppSettings
{
public:
	bool Read(POApplication& app, RECT& rectWnd, bool& centerWindow, bool& alwaysOnTop);
	bool Write(const POApplication& app, const RECT& rcWnd, bool alwaysOnTop);

private:
	// The full path to PngOptimizer.ini
	String GetFilePath() const;
};

#endif // ndef PO_APPSETTINGS_H
