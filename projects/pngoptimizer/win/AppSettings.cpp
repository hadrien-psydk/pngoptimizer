/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AppSettings.h"

static const char k_appName[] = "PngOptimizer";

static const char k_szFileComment1[] = "PngOptimizer settings";
static const char k_szFileComment2[] = "This file is encoded in UTF-8";

static const char k_szSectionEngine[]         = "Engine";

static const char k_szSectionScreenshots[]      = "Screenshots";
static const char k_szShotUseDefaultDir[]       = "UseDefaultDir";
static const char k_szShotCustomDir[]           = "CustomDir";
static const char k_szShotMaximizeCompression[] = "MaximizeCompression";
static const char k_szShotAskForFileName[]      = "AskForFileName";

static const char k_szSectionWindow[]  = "Window";
static const char k_szWndX[]           = "X";
static const char k_szWndY[]           = "Y";
static const char k_szWndWidth[]       = "Width";
static const char k_szWndHeight[]      = "Height";
static const char k_szWndAlwaysOnTop[] = "AlwaysOnTop";

/////////////////////////////////////////////////////////////
bool AppSettings::Read(POEngineSettings& pos, BmpcdSettings* pBs, MainWndSettings* pMws)
{
	const int defaultWidth = 520;
	const int defaultHeight = 300;

	pMws->left = 0;
	pMws->top = 0;
	pMws->right = pMws->left + defaultWidth;
	pMws->bottom = pMws->top + defaultHeight;
	pMws->topLeftValid = false; // Will center the window
	pMws->alwaysOnTop = true; // Default

	String filePath = Process::GetConfigPathForRead(k_appName);
	if( filePath.IsEmpty() )
	{
		// First run
		return true;
	}

	MemIniFile ini;
	if( !ini.Load(filePath) )
	{
		// Cannot read config file :(
		return false;
	}

	ini.SetSection(k_szSectionEngine);
	pos.LoadFromIni(ini);

	////////////////////////////////////////////////
	if( pBs )
	{
		ini.SetSection(k_szSectionScreenshots);
		ini.GetBool(k_szShotUseDefaultDir, pBs->useDefaultDir);
		ini.GetString(k_szShotCustomDir, pBs->customDir);
		ini.GetBool(k_szShotMaximizeCompression, pBs->maximizeCompression);
		ini.GetBool(k_szShotAskForFileName, pBs->askForFileName);
	}

	////////////////////////////////////////////////
	if( pMws )
	{
		ini.SetSection(k_szSectionWindow);

		int wndX = 0;
		int wndY = 0;
		int wndWidth = 0;
		int wndHeight = 0;

		if( ini.GetInt(k_szWndX, wndX) && ini.GetInt(k_szWndY, wndY) )
		{
			// No need to center, the values are ok
			pMws->topLeftValid = true;
		}

		ini.GetInt(k_szWndWidth, wndWidth);
		ini.GetInt(k_szWndHeight, wndHeight);

		// Reajust rect if needed
		if( wndX < 0 )
		{
			wndX = 0;
		}

		if( wndY < 0 )
		{
			wndY = 0;
		}

		if( wndWidth <= 50 )
		{
			wndWidth = defaultWidth;
		}

		if( wndHeight <= 50 )
		{
			wndHeight = defaultHeight;
		}
		////////////////////////////////////////////////

		pMws->left = wndX;
		pMws->top = wndY;
		pMws->right = pMws->left + wndWidth;
		pMws->bottom = pMws->top + wndHeight;

		ini.GetBool(k_szWndAlwaysOnTop, pMws->alwaysOnTop);
	}
	return true;
}

bool AppSettings::Write(const POEngineSettings& pos, const BmpcdSettings* pBs, const MainWndSettings* pMws)
{
	MemIniFile ini;

	////////////////////////////////////////////////
	ini.SetSection(k_szSectionEngine);
	pos.SaveToIni(ini);

	////////////////////////////////////////////////
	if( pBs )
	{
		ini.SetSection(k_szSectionScreenshots);
		ini.SetBool(k_szShotUseDefaultDir, pBs->useDefaultDir);
		ini.SetString(k_szShotCustomDir, pBs->customDir);
		ini.SetBool(k_szShotMaximizeCompression, pBs->maximizeCompression);
		ini.SetBool(k_szShotAskForFileName, pBs->askForFileName);
	}

	////////////////////////////////////////////////
	if( pMws )
	{
		int wndX = pMws->left;
		int wndY = pMws->top;
		int wndWidth = pMws->right - pMws->left;
		int wndHeight = pMws->bottom - pMws->top;

		ini.SetSection(k_szSectionWindow);
		ini.SetInt(k_szWndX, wndX);
		ini.SetInt(k_szWndY, wndY);
		ini.SetInt(k_szWndWidth, wndWidth);
		ini.SetInt(k_szWndHeight, wndHeight);
		ini.SetBool(k_szWndAlwaysOnTop, pMws->alwaysOnTop);
	}

	////////////////////////////////////////////////
	ini.SetCommentLine1(k_szFileComment1);
	ini.SetCommentLine2(k_szFileComment2);

	String filePath = Process::GetConfigPathForWrite(k_appName);
	if( filePath.IsEmpty() )
	{
		return false;
	}
	return ini.Dump(filePath, true); // true is for CR-LF, we are kind for notepad users
}
