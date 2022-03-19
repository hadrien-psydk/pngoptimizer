/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POEngineSettings.h"

using namespace chustd;

static const char k_szBackupOldPngFiles[]     = "BackupOldPngFiles";
static const char k_szKeepInterlacing[]       = "KeepInterlacing";
static const char k_szAvoidGreyWithSimpleTransparency[] = "AvoidGreyWithSimpleTransparency";
static const char k_szIgnoreAnimatedGifs[]    = "IgnoreAnimatedGifs";
static const char k_szKeepFileDate[]          = "KeepFileDate";
static const char k_szKeepPixels[]          = "KeepPixels";

static const char k_szKeepBackgroundColor[]   = "KeepBackgroundColor";
static const char k_szForcedBackgroundColor[] = "ForcedBackgroundColor";

static const char k_szKeepTextualData[]       = "KeepTextualData";
static const char k_szForcedTextKeyword[]     = "ForcedTextKeyword";
static const char k_szForcedTextData[]        = "ForcedTextData";

static const char k_szKeepPhysicalPixelDimensions[] = "KeepPhysicalPixelDimensions";
static const char k_szForcedPixelsPerMeter[]     = "ForcedPixelsPerMeter";
static const char k_szForcedPixelsPerInch[]      = "ForcedPixelsPerInch";
static const int  k_PpuSeparator = 'x'; // Pixels per unit separator

static const char k_szKeepFrameControl[]       = "KeepFrameControl";
static const char k_szForcedDelayNumerator[]   = "ForcedDelayNumerator";
static const char k_szForcedDelayDenominator[] = "ForcedDelayDenominator";

///////////////////////////////////////////////////////////////////////////////////////////////////
POEngineSettings::POEngineSettings()
{
	backupOldPngFiles = true;
	keepInterlacing = false;
	avoidGreyWithSimpleTransparency = false;
	ignoreAnimatedGifs = false;
	keepFileDate = false;
	keepPixels = false;

	bkgdOption = POChunk_Remove;
	textOption = POChunk_Remove;

	physOption = POChunk_Remove;
	physPpmX = 2834; // 72 dpi: 2834.645669291339 dpm
	physPpmY = 2834;

	fctlOption = POChunk_Keep;
	fctlDelayNum = 1;
	fctlDelayDen = 10;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Gets two integers separated by a character. Outputs are set to zero if not found.
///////////////////////////////////////////////////////////////////////////////////////////////////
void GetDoubleInt(const String& str, int sep, int& first, int& second)
{
	StringArray splitted = str.Split(sep);
	String firstStr, secondStr;
	if( splitted.GetSize() >= 1 )
	{
		firstStr = splitted[0];
	}
	if( splitted.GetSize() >= 2 )
	{
		secondStr = splitted[1];
	}
	firstStr.ToInt(first);
	secondStr.ToInt(second);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Load settings from an INI file. The section should be set.
///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngineSettings::LoadFromIni(const MemIniFile& ini)
{
	ini.GetBool(k_szBackupOldPngFiles, backupOldPngFiles);
	ini.GetBool(k_szKeepInterlacing, keepInterlacing);
	ini.GetBool(k_szAvoidGreyWithSimpleTransparency, avoidGreyWithSimpleTransparency);
	ini.GetBool(k_szIgnoreAnimatedGifs, ignoreAnimatedGifs);
	ini.GetBool(k_szKeepFileDate, keepFileDate);
	ini.GetBool(k_szKeepPixels, keepPixels);

	int optionInt = int(bkgdOption);
	ini.GetInt(k_szKeepBackgroundColor, optionInt);
	bkgdOption = POChunkOption(optionInt);

	String strColBk;
	int32 nColBk = 0;
	ini.GetString(k_szForcedBackgroundColor, strColBk);
	if( strColBk.ToInt(nColBk, 'x') )
	{
		uint8 r = (nColBk >> 16) & 0x00ff;
		uint8 g = (nColBk >> 8) & 0x00ff;
		uint8 b = (nColBk) & 0x00ff;
		bkgdColor.SetRgb(r, g, b);
	}
	else
	{
		bkgdColor = Color::Black;
	}

	///////////////////////////////////////////
	optionInt = int(textOption);
	ini.GetInt(k_szKeepTextualData, optionInt);
	textOption = POChunkOption(optionInt);

	ini.GetString(k_szForcedTextKeyword, textKeyword);
	ini.GetString(k_szForcedTextData, textData);

	///////////////////////////////////////////
	optionInt = int(physOption);
	ini.GetInt(k_szKeepPhysicalPixelDimensions, optionInt);
	physOption = POChunkOption(optionInt);

	String ppmStr;
	// Note: only PPM in the INI file
	ini.GetString(k_szForcedPixelsPerMeter, ppmStr);
	GetDoubleInt(ppmStr, k_PpuSeparator, physPpmX, physPpmY);

	///////////////////////////////////////////
	optionInt = int(fctlOption);
	ini.GetInt(k_szKeepFrameControl, optionInt);
	fctlOption = POChunkOption(optionInt);

	ini.GetInt(k_szForcedDelayNumerator, fctlDelayNum);
	ini.GetInt(k_szForcedDelayDenominator, fctlDelayDen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngineSettings::SaveToIni(MemIniFile& ini) const
{
	ini.SetBool(k_szBackupOldPngFiles,  backupOldPngFiles);
	ini.SetBool(k_szKeepInterlacing,    keepInterlacing);
	ini.SetBool(k_szAvoidGreyWithSimpleTransparency, avoidGreyWithSimpleTransparency);
	ini.SetBool(k_szIgnoreAnimatedGifs, ignoreAnimatedGifs);
	ini.SetBool(k_szKeepFileDate,       keepFileDate);
	ini.SetBool(k_szKeepPixels,       keepPixels);

	ini.SetInt(k_szKeepBackgroundColor, bkgdOption);
	uint8 r, g, b;
	bkgdColor.ToRgb(r, g, b);
	int32 nColBk = (r << 16) | (g <<8) | b;
	String strColBk = String::FromInt(nColBk, 'x', 6, '0').Right(6);
	ini.SetString(k_szForcedBackgroundColor, strColBk);

	///////////////////////////////////////////
	ini.SetInt(k_szKeepTextualData, textOption);
	ini.SetString(k_szForcedTextKeyword, textKeyword);
	ini.SetString(k_szForcedTextData, textData);

	///////////////////////////////////////////
	ini.SetInt(k_szKeepPhysicalPixelDimensions, physOption);
	// Note: only PPM in the INI file
	String ppmStr = String::FromInt(physPpmX) + "x" + String::FromInt(physPpmY);
	ini.SetString(k_szForcedPixelsPerMeter, ppmStr);

	///////////////////////////////////////////
	ini.SetInt(k_szKeepFrameControl, fctlOption);
	ini.SetInt(k_szForcedDelayNumerator, fctlDelayNum);
	ini.SetInt(k_szForcedDelayDenominator, fctlDelayDen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
POChunkOption GetChunkOption(const ArgvParser& ap, const String& optionName)
{
	POChunkOption chunkOption = POChunk_Remove;
	if( ap.HasFlag(optionName) )
	{
		chunkOption = POChunk_Keep;

		String strKbcOption = ap.GetFlagString(optionName);
		if( strKbcOption == "0" || strKbcOption == "R" )
		{
			chunkOption = POChunk_Remove;
		}
		else if( strKbcOption == "2" || strKbcOption == "F" )
		{
			chunkOption = POChunk_Force;
		}
	}
	return chunkOption;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Load settings from a command line parsed with ArgvParser.
///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngineSettings::LoadFromArgv(const ArgvParser& ap)
{
	backupOldPngFiles = ap.HasFlag(k_szBackupOldPngFiles);
	keepInterlacing = ap.HasFlag(k_szKeepInterlacing);
	avoidGreyWithSimpleTransparency = ap.HasFlag(k_szAvoidGreyWithSimpleTransparency);
	ignoreAnimatedGifs = ap.HasFlag(k_szIgnoreAnimatedGifs);
	keepFileDate = ap.HasFlag(k_szKeepFileDate);
	keepPixels = ap.HasFlag(k_szKeepPixels);

	bkgdOption = GetChunkOption(ap, k_szKeepBackgroundColor);

	Color bkColor;
	if( ap.HasFlag(k_szForcedBackgroundColor) )
	{
		String strBkColor = ap.GetFlagString(k_szForcedBackgroundColor);
		if( strBkColor.GetLength() == 6 )
		{
			int bkR = 0;
			int bkG = 0;
			int bkB = 0;
			strBkColor.Mid(0, 2).ToInt(bkR, 'x');
			strBkColor.Mid(2, 2).ToInt(bkG, 'x');
			strBkColor.Mid(4, 2).ToInt(bkB, 'x');
			bkgdColor.SetRgb(uint8(bkR), uint8(bkG), uint8(bkB));
		}
	}

	///////////////////////////////////////////
	textOption = GetChunkOption(ap, k_szKeepTextualData);
	textKeyword = ap.GetFlagString(k_szForcedTextKeyword);
	textData = ap.GetFlagString(k_szForcedTextData);

	///////////////////////////////////////////
	physOption = GetChunkOption(ap, k_szKeepPhysicalPixelDimensions);
	String ppmStr = ap.GetFlagString(k_szForcedPixelsPerMeter);
	String ppiStr = ap.GetFlagString(k_szForcedPixelsPerInch);
	if( !ppmStr.IsEmpty() )
	{
		GetDoubleInt(ppmStr, k_PpuSeparator, physPpmX, physPpmY);
	}
	else if( !ppiStr.IsEmpty() )
	{
		int ppiX = 0;
		int ppiY = 0;
		GetDoubleInt(ppiStr, k_PpuSeparator, ppiX, ppiY);
		physPpmX = PpmFromPpi(ppiX);
		physPpmY = PpmFromPpi(ppiY);
	}

	///////////////////////////////////////////
	fctlOption = GetChunkOption(ap, k_szKeepTextualData);
	fctlDelayNum = ap.GetFlagInt(k_szForcedDelayNumerator);
	fctlDelayDen = ap.GetFlagInt(k_szForcedDelayDenominator);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Writes usage when parsing from argv.
///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngineSettings::WriteArgvUsage(const String& indent)
{
	Console::WriteLine(indent + "[-" + String(k_szBackupOldPngFiles) + "]");
	Console::WriteLine(indent + "[-" + String(k_szKeepInterlacing) + "]");
	Console::WriteLine(indent + "[-" + String(k_szAvoidGreyWithSimpleTransparency) + "]");
	Console::WriteLine(indent + "[-" + String(k_szIgnoreAnimatedGifs) + "]");
	Console::WriteLine(indent + "[-" + String(k_szKeepFileDate) + "]");
	Console::WriteLine(indent + "[-" + String(k_szKeepPixels) + "]");
	Console::WriteLine(indent + "[-" + String(k_szKeepBackgroundColor) + "][:R|K|F] [-" + String(k_szForcedBackgroundColor) + ":RRGGBB]");
	Console::WriteLine(indent + "[-" + String(k_szKeepTextualData) + "][:R|K|F]     [-" + String(k_szForcedTextKeyword) + ":Foo] [-" 
	                                                                                    + String(k_szForcedTextData) + ":Bar]");
	Console::WriteLine(indent + "[-" + String(k_szKeepPhysicalPixelDimensions) + "][:R|K|F] [-" 
		+ String(k_szForcedPixelsPerMeter) + ":3000x2500] ");
	Console::WriteLine(indent + "                                       [-" + String(k_szForcedPixelsPerInch) + ":72x72]");

	Console::WriteLine(indent + "[-" + String(k_szKeepFrameControl) + "][:K|F]     [-" + String(k_szForcedDelayNumerator) + ":1] [-" 
	                                                                                   + String(k_szForcedDelayDenominator) + ":30]");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Converts pixels per meter to pixels per inch.
///////////////////////////////////////////////////////////////////////////////////////////////////
int POEngineSettings::PpiFromPpm(int ppm)
{
	// Convention in graphics software implies that we use rounding for ppm --> ppi
	int ppi = ((ppm * 254) + 5000) / 10000;
	return ppi;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Converts pixels per inch to pixels per meter.
///////////////////////////////////////////////////////////////////////////////////////////////////
int POEngineSettings::PpmFromPpi(int ppi)
{
	// Convention in graphics software implies that we use truncation for ppi --> ppm
	int ppm = (ppi * 10000) / 254;
	return ppm;
}
