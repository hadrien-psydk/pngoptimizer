/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_APPSETTINGS_H
#define PO_APPSETTINGS_H

#include "BmpcdSettings.h"

struct MainWndSettings
{
	int top, left, bottom, right;
	bool topLeftValid; // true if top,left are valid
	bool alwaysOnTop;

	MainWndSettings()
	{
		top = left = bottom = right = 0;
		topLeftValid = false;
		alwaysOnTop = false;
	}
};

// Reads and writes settings for the POApplication class
class AppSettings
{
public:
	bool Read(POEngineSettings& pos, BmpcdSettings*, MainWndSettings*);
	bool Write(const POEngineSettings& pos, const BmpcdSettings*, const MainWndSettings*);
};

#endif // ndef PO_APPSETTINGS_H
