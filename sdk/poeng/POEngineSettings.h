/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
/////////////////////////////////////////////////////////////////////////////////////
#ifndef POENG_POENGINESETTINGS_H
#define POENG_POENGINESETTINGS_H

///////////////////////////////////////////////////////////////////////////////////////////////////
enum POChunkOption
{
	POChunk_Remove,
	POChunk_Keep,
	POChunk_Force
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Settings for POEngine
struct POEngineSettings
{
	// Settings, directly read/written by the application
	bool backupOldPngFiles;
	bool keepInterlacing;
	bool avoidGreyWithSimpleTransparency;
	bool ignoreAnimatedGifs;
	bool keepFileDate;
	bool dontOptimize;

	POChunkOption  bkgdOption;
	chustd::Color  bkgdColor; // Forced color

	POChunkOption  textOption;
	chustd::String textKeyword; // Forced text keyword
	chustd::String textData;    // Forced text data

	POChunkOption  physOption;
	int            physPpmX;
	int            physPpmY;

	POChunkOption  fctlOption; // fcTL for APNG
	int            fctlDelayNum;
	int            fctlDelayDen; // Seconds. If 0 then equals to 100

	POEngineSettings();
	void LoadFromIni(const chustd::MemIniFile& ini);
	void LoadFromArgv(const chustd::ArgvParser& ap);
	void SaveToIni(chustd::MemIniFile& ini) const;

	static void WriteArgvUsage(const chustd::String& indent);

	// Sometimes it easier for the user to enter values in PPI, so we offer
	// some conversion functions
	static int PpiFromPpm(int ppm);
	static int PpmFromPpi(int ppi);
};

#endif
