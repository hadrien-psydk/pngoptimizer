/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_APPSETTINGS_H
#define PO_APPSETTINGS_H

struct MainWndSettings
{
	bool alwaysOnTop;

	MainWndSettings()
	{
		alwaysOnTop = true;
	}
};

// Reads and writes settings for the POApplication class
class AppSettings
{
public:
	Event0 Changed;

public:
	static AppSettings& GetInstance();

	void SetPOSettings(const POEngineSettings&);
	const POEngineSettings& GetPOSettings() const;

	void SetMWSettings(const MainWndSettings&);
	const MainWndSettings& GetMWSettings() const;

	bool Read();
	bool Write();

private:
	POEngineSettings m_posettings;
	MainWndSettings  m_mwsettings;
};

#endif
