/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AppSettings.h"

static const char k_appName[] = "pngoptimizer"; // All lowercase on Linux

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

///////////////////////////////////////////////////////////////////////////////
AppSettings& AppSettings::GetInstance()
{
	static AppSettings instance;
	return instance;
}

///////////////////////////////////////////////////////////////////////////////
void AppSettings::SetPOSettings(const POEngineSettings& posettings)
{
	m_posettings = posettings;
	Changed.Fire();
}

///////////////////////////////////////////////////////////////////////////////
const POEngineSettings& AppSettings::GetPOSettings() const
{
	return m_posettings;
}

///////////////////////////////////////////////////////////////////////////////
void AppSettings::SetMWSettings(const MainWndSettings& mwsettings)
{
	m_mwsettings = mwsettings;
	Changed.Fire();
}

///////////////////////////////////////////////////////////////////////////////
const MainWndSettings& AppSettings::GetMWSettings() const
{
	return m_mwsettings;
}

///////////////////////////////////////////////////////////////////////////////
bool AppSettings::Read()
{
	/*
	const int defaultWidth = 482;
	const int defaultHeight = 300;

	pMWS->left = 0;
	pMWS->top = 0;
	pMWS->right = pMWS->left + defaultWidth;
	pMWS->bottom = pMWS->top + defaultHeight;
	pMWS->topLeftValid = false; // Will center the window
	pMWS->alwaysOnTop = true; // Default
	*/

	String filePath = Process::GetConfigPathForRead(k_appName);
	if( filePath.IsEmpty() )
	{
		// No config file
		return true;
	}

	MemIniFile ini;
	if( !ini.Load(filePath) )
	{
		// Cannot read config file :(
		return false;
	}

	ini.SetSection(k_szSectionEngine);
	m_posettings.LoadFromIni(ini);

	/*
	////////////////////////////////////////////////
	if( pBS )
	{
		ini.SetSection(k_szSectionScreenshots);
		ini.GetBool(k_szShotUseDefaultDir, pBS->useDefaultDir);
		ini.GetString(k_szShotCustomDir, pBS->customDir);
		ini.GetBool(k_szShotMaximizeCompression, pBS->maximizeCompression);
		ini.GetBool(k_szShotAskForFileName, pBS->askForFileName);
	}

	////////////////////////////////////////////////
	if( pMWS )
	{
		ini.SetSection(k_szSectionWindow);

		int wndX = 0;
		int wndY = 0;
		int wndWidth = 0;
		int wndHeight = 0;

		if( ini.GetInt(k_szWndX, wndX) && ini.GetInt(k_szWndY, wndY) )
		{
			// No need to center, the values are ok
			pMWS->topLeftValid = true;
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

		pMWS->left = wndX;
		pMWS->top = wndY;
		pMWS->right = pMWS->left + wndWidth;
		pMWS->bottom = pMWS->top + wndHeight;

		ini.GetBool(k_szWndAlwaysOnTop, pMWS->alwaysOnTop);
	}
	*/
	return true;
}

bool AppSettings::Write()
{
	String filePath = Process::GetConfigPathForWrite(k_appName);
	if( filePath.IsEmpty() )
	{
		// Error creating directory structure
		return false;
	}

	MemIniFile ini;

	////////////////////////////////////////////////
	ini.SetSection(k_szSectionEngine);
	m_posettings.SaveToIni(ini);

/*
	////////////////////////////////////////////////
	if( pBS )
	{
		ini.SetSection(k_szSectionScreenshots);
		ini.SetBool(k_szShotUseDefaultDir, pBS->useDefaultDir);
		ini.SetString(k_szShotCustomDir, pBS->customDir);
		ini.SetBool(k_szShotMaximizeCompression, pBS->maximizeCompression);
		ini.SetBool(k_szShotAskForFileName, pBS->askForFileName);
	}

	////////////////////////////////////////////////
	if( pMWS )
	{
		int wndX = pMWS->left;
		int wndY = pMWS->top;
		int wndWidth = pMWS->right - pMWS->left;
		int wndHeight = pMWS->bottom - pMWS->top;

		ini.SetSection(k_szSectionWindow);
		ini.SetInt(k_szWndX, wndX);
		ini.SetInt(k_szWndY, wndY);
		ini.SetInt(k_szWndWidth, wndWidth);
		ini.SetInt(k_szWndHeight, wndHeight);
		ini.SetBool(k_szWndAlwaysOnTop, pMWS->alwaysOnTop);
	}
	*/

	////////////////////////////////////////////////
	ini.SetCommentLine1(k_szFileComment1);
	ini.SetCommentLine2(k_szFileComment2);

	return ini.Dump(filePath, true); // true is for CR-LF, we are kind for notepad users
}
