/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AppSettings.h"
#include "PngOptimizer.h"

static const char k_szAppSettingsFileName[] = "PngOptimizer.ini";
static const char k_szAppSettingsSubDir[] = "PngOptimizer";

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
String AppSettings::GetFilePath() const
{
	String strConfigFileName = k_szAppSettingsFileName;

	// A settings file in the executable directory ?
	String exeDir = Process::GetExecutableDirectory();
	String filePath = FilePath::Combine(exeDir, strConfigFileName);
	if( !File::Exists(filePath) )
	{
		// Nop, use a user profile location
		String appDataDir = System::GetApplicationDataDirectory();
		String configDir = FilePath::Combine(appDataDir, k_szAppSettingsSubDir);

		if( !Directory::Exists(configDir) )
		{
			// First time PngOptimizer is run, create profile directory
			Directory::Create(configDir);
		}
		filePath = FilePath::Combine(configDir, strConfigFileName);
	}
	return filePath;
}

/////////////////////////////////////////////////////////////
bool AppSettings::Read(POApplication& app, RECT& rectWnd, bool& centerWindow, bool& alwaysOnTop)
{
	const int32 defaultWidth = 482;
	const int32 defaultHeight = 300;

	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = rc.left + defaultWidth;
	rc.bottom = rc.top + defaultHeight;

	rectWnd = rc;
	centerWindow = true;
	alwaysOnTop = true; // Default

	String filePath = GetFilePath();
	if( !File::Exists(filePath) )
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
	app.m_engine.m_settings.LoadFromIni(ini);

	////////////////////////////////////////////////
	ini.SetSection(k_szSectionScreenshots);
	ini.GetBool(k_szShotUseDefaultDir, app.m_bmpcd.m_settings.useDefaultDir);
	ini.GetString(k_szShotCustomDir, app.m_bmpcd.m_settings.customDir);
	ini.GetBool(k_szShotMaximizeCompression, app.m_bmpcd.m_settings.maximizeCompression);
	ini.GetBool(k_szShotAskForFileName, app.m_bmpcd.m_settings.askForFileName);

	////////////////////////////////////////////////
	ini.SetSection(k_szSectionWindow);

	int wndX = 0;
	int wndY = 0;
	int wndWidth = 0;
	int wndHeight = 0;

	if( ini.GetInt(k_szWndX, wndX) && ini.GetInt(k_szWndY, wndY) )
	{
		// No need to center, the values are ok
		centerWindow = false;
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

	rc.left = wndX;
	rc.top = wndY;
	rc.right = rc.left + wndWidth;
	rc.bottom = rc.top + wndHeight;
	rectWnd = rc;

	ini.GetBool(k_szWndAlwaysOnTop, alwaysOnTop);
	return true;
}

bool AppSettings::Write(const POApplication& app, const RECT& rcWnd, bool alwaysOnTop)
{
	int wndX = rcWnd.left;
	int wndY = rcWnd.top;
	int wndWidth = rcWnd.right - rcWnd.left;
	int wndHeight = rcWnd.bottom - rcWnd.top;

	MemIniFile ini;

	////////////////////////////////////////////////
	ini.SetSection(k_szSectionEngine);
	app.m_engine.m_settings.SaveToIni(ini);

	////////////////////////////////////////////////
	ini.SetSection(k_szSectionScreenshots);
	ini.SetBool(k_szShotUseDefaultDir, app.m_bmpcd.m_settings.useDefaultDir);
	ini.SetString(k_szShotCustomDir, app.m_bmpcd.m_settings.customDir);
	ini.SetBool(k_szShotMaximizeCompression, app.m_bmpcd.m_settings.maximizeCompression);
	ini.SetBool(k_szShotAskForFileName, app.m_bmpcd.m_settings.askForFileName);

	////////////////////////////////////////////////
	ini.SetSection(k_szSectionWindow);
	ini.SetInt(k_szWndX, wndX);
	ini.SetInt(k_szWndY, wndY);
	ini.SetInt(k_szWndWidth, wndWidth);
	ini.SetInt(k_szWndHeight, wndHeight);
	ini.SetBool(k_szWndAlwaysOnTop, alwaysOnTop);

	ini.SetCommentLine1(k_szFileComment1);
	ini.SetCommentLine2(k_szFileComment2);

	String filePath = GetFilePath();
	return ini.Dump(filePath, true); // true is for CR-LF, we are kind for notepad users
}
