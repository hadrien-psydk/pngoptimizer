#include "stdafx.h"

bool operator==(const POEngineSettings& s1, const POEngineSettings& s2)
{
	bool ret = true;
	ret &= s1.backupOldPngFiles == s2.backupOldPngFiles;
	ret &= s1.keepInterlacing == s2.keepInterlacing;
	ret &= s1.avoidGreyWithSimpleTransparency == s2.avoidGreyWithSimpleTransparency;
	ret &= s1.ignoreAnimatedGifs == s2.ignoreAnimatedGifs;
	ret &= s1.keepFileDate == s2.keepFileDate;

	ret &= s1.bkgdOption == s2.bkgdOption;
	ret &= s1.bkgdColor == s2.bkgdColor;

	ret &= s1.textOption == s2.textOption;
	ret &= s1.textKeyword == s2.textKeyword;
	ret &= s1.textData == s2.textData;

	ret &= s1.physOption == s2.physOption;
	ret &= s1.physPpmX == s2.physPpmX;
	ret &= s1.physPpmY == s2.physPpmY;

	return ret;
}

const String k_iniFilePath = "test.ini";
const String k_iniSection = "PoeSettings";

void ToIni(const POEngineSettings& settings)
{
	File::Delete(k_iniFilePath);
	
	MemIniFile iniSave;
	iniSave.SetSection(k_iniSection);
	settings.SaveToIni(iniSave);
	iniSave.Dump(k_iniFilePath, true);
}

POEngineSettings FromIni()
{
	MemIniFile iniLoad;
	iniLoad.Load(k_iniFilePath);
	iniLoad.SetSection(k_iniSection);

	POEngineSettings settings;
	settings.LoadFromIni(iniLoad);
	return settings;
}

TEST(POEngineSettings, EmptyArgv)
{
	const char* argv[] = { "app.exe" };
	ArgvParser ap(ARRAY_SIZE(argv), argv);

	POEngineSettings settings;
	settings.LoadFromArgv(ap);
	
	POEngineSettings exp;
	exp.backupOldPngFiles = false; // The only difference with INI loading
	ASSERT_TRUE(settings == exp);
}

TEST(POEngineSettings, AllArgv)
{
	const char* argv[] = {
		"app.exe",
		"-BackupOldPngFiles",
		"-KeepInterlacing",
		"-AvoidGreyWithSimpleTransparency",
		"-IgnoreAnimatedGifs",
		"-KeepFileDate",
		"-KeepBackgroundColor",
		"-KeepTextualData",
		"-KeepPhysicalPixelDimensions"
	};
	ArgvParser ap(ARRAY_SIZE(argv), argv);

	POEngineSettings settings;
	settings.LoadFromArgv(ap);
	
	POEngineSettings exp;
	exp.backupOldPngFiles = true;
	exp.keepInterlacing = true;
	exp.avoidGreyWithSimpleTransparency = true;
	exp.ignoreAnimatedGifs = true;
	exp.keepFileDate = true;
	exp.bkgdOption = POChunk_Keep;
	exp.textOption = POChunk_Keep;
	exp.physOption = POChunk_Keep;
	ASSERT_TRUE(settings == exp);

	// Test with INI
	ToIni(settings);
	POEngineSettings settings2 = FromIni();
	ASSERT_TRUE(settings2 == exp);
}

TEST(POEngineSettings, RemoveChunkArgv)
{
	const char* argv[] = {
		"app.exe",
		"-KeepBackgroundColor:0",
		"-KeepTextualData:0",
		"-KeepPhysicalPixelDimensions:0"
	};
	ArgvParser ap(ARRAY_SIZE(argv), argv);

	POEngineSettings settings;
	settings.LoadFromArgv(ap);
	
	POEngineSettings exp;
	exp.backupOldPngFiles = false;
	exp.bkgdOption = POChunk_Remove;
	exp.textOption = POChunk_Remove;
	exp.physOption = POChunk_Remove;
	ASSERT_TRUE(settings == exp);

	const char* argvAlt[] = {
		"app.exe",
		"-KeepBackgroundColor:R",
		"-KeepTextualData:R",
		"-KeepPhysicalPixelDimensions:R"
	};
	ArgvParser apAlt(ARRAY_SIZE(argvAlt), argvAlt);

	settings.LoadFromArgv(apAlt);
	ASSERT_TRUE(settings == exp);
}

TEST(POEngineSettings, KeepChunkArgv)
{
	const char* argv[] = {
		"app.exe",
		"-KeepBackgroundColor:1",
		"-KeepTextualData:1",
		"-KeepPhysicalPixelDimensions:1"
	};
	ArgvParser ap(ARRAY_SIZE(argv), argv);

	POEngineSettings settings;
	settings.LoadFromArgv(ap);
	
	POEngineSettings exp;
	exp.backupOldPngFiles = false;
	exp.bkgdOption = POChunk_Keep;
	exp.textOption = POChunk_Keep;
	exp.physOption = POChunk_Keep;
	ASSERT_TRUE(settings == exp);

	const char* argvAlt[] = {
		"app.exe",
		"-KeepBackgroundColor:K",
		"-KeepTextualData:K",
		"-KeepPhysicalPixelDimensions:K"
	};
	ArgvParser apAlt(ARRAY_SIZE(argvAlt), argvAlt);

	settings.LoadFromArgv(apAlt);
	ASSERT_TRUE(settings == exp);
}

#define UTF8_CAPITAL_E_WITH_ACUTE "\xc3\x89"
#define UTF8_SMALL_E_WITH_ACUTE "\xc3\xa9"

TEST(POEngineSettings, ForceChunkArgv)
{
	const char* argv[] = {
		"app.exe",
		
		"-KeepBackgroundColor:2",
		"-ForcedBackgroundColor:a1b2c3",

		"-KeepTextualData:2",
		"-ForcedTextKeyword:" UTF8_CAPITAL_E_WITH_ACUTE "toffe",
		"-ForcedTextData:Soie brod" UTF8_SMALL_E_WITH_ACUTE "e",
		
		"-KeepPhysicalPixelDimensions:2",
		"-ForcedPixelsPerMeter:2000x1000"
	};
	ArgvParser ap(ARRAY_SIZE(argv), argv);

	POEngineSettings settings;
	settings.LoadFromArgv(ap);
	
	POEngineSettings exp;
	exp.backupOldPngFiles = false;
	exp.bkgdOption = POChunk_Force;
	exp.bkgdColor.SetRgb(0xa1u, 0xb2u, 0xc3u);
	exp.textOption = POChunk_Force;
	exp.textKeyword = String::FromUtf8Z(UTF8_CAPITAL_E_WITH_ACUTE "toffe");
	exp.textData = String::FromUtf8Z("Soie brod" UTF8_SMALL_E_WITH_ACUTE "e");
	exp.physOption = POChunk_Force;
	exp.physPpmX = 2000;
	exp.physPpmY = 1000;
	ASSERT_TRUE(settings == exp);

	const char* argvAlt[] = {
		"app.exe",
		
		"-KeepBackgroundColor:F",
		"-ForcedBackgroundColor:a1b2c3",

		"-KeepTextualData:F",
		"-ForcedTextKeyword:" UTF8_CAPITAL_E_WITH_ACUTE "toffe",
		"-ForcedTextData:Soie brod" UTF8_SMALL_E_WITH_ACUTE "e",
		
		"-KeepPhysicalPixelDimensions:F",
		"-ForcedPixelsPerMeter:2000x1000"
	};
	ArgvParser apAlt(ARRAY_SIZE(argvAlt), argvAlt);

	settings.LoadFromArgv(apAlt);
	ASSERT_TRUE(settings == exp);

	// Test with INI
	ToIni(settings);
	POEngineSettings settings2 = FromIni();
	ASSERT_TRUE(settings2 == exp);
}

// PNG stores pixel density in pixels-per-meter, but the ARGV accepts pixels-per-inch too
TEST(POEngineSettings, ForcePixelsPerInchArgv)
{
	const char* argv[] = {
		"app.exe",
		"-KeepPhysicalPixelDimensions:2",
		"-ForcedPixelsPerInch:254x127"
	};
	ArgvParser ap(ARRAY_SIZE(argv), argv);

	POEngineSettings settings;
	settings.LoadFromArgv(ap);
	
	POEngineSettings exp;
	exp.backupOldPngFiles = false;
	exp.physOption = POChunk_Force;
	exp.physPpmX = 10000;
	exp.physPpmY = 5000;
	ASSERT_TRUE(settings == exp);
}


