// This file is part of the PngOptimizerCL application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

using namespace chustd;

#define PNGO_VERSION "2.6.2"

// Do not write to stdout at all except for the result file
// when the -stdio flag is used
bool g_stdioMode = false;

class POApplicationConsole
{
public:
	void OnEngineProgressing(const POEngine::ProgressingArg& arg)
	{
		const bool useStderr = (
			   arg.textType == POEngine::TT_ActionFail
			|| arg.textType == POEngine::TT_ErrorMsg);

		IConsoleWriter* pCw;
		if( useStderr )
		{
			pCw = &Console::Stderr();
		}
		else
		{
			if( g_stdioMode )
			{
				// -stdio mode, do not output progress messages to stdout,
				// only the result png
				return;
			}
			pCw = &Console::Stdout();
		}
		IConsoleWriter& cw = *pCw; // I want Rust block expressions

		Color col = POEngine::ColorFromTextType(arg.textType);
		if( col.r == 0 && col.g == 0 && col.b == 0 )
		{
			cw.ResetTextColor();
		}
		else
		{
			cw.SetTextColor(col);
		}
		cw.Write(arg.text);
	}
};

static void WriteVersion()
{
	String version = "PngOptimizerCL " PNGO_VERSION;
#if defined(_M_X64)
	version = version + String(" (x64)");
#elif defined(_M_IX86)
	version = version + String(" (x86)");
#endif
	Console::WriteLine(version);
}

static void WriteHelp()
{
	WriteVersion();
	Console::WriteLine("Copyright \xA9 2002/2019 Hadrien Nilsson - psydk.org");
	Console::WriteLine("Converts GIF, BMP and TGA files to optimized PNG files.");
	Console::WriteLine("Optimizes and cleans PNG files.");
	Console::WriteLine("");
	Console::WriteLine("Usage:  pngoptimizercl (FILE [FILE2 [FILE3...]] | -file:\"yourfile.png\" | -stdio) [-recurs]");
	POEngineSettings::WriteArgvUsage("  ");
	Console::WriteLine("");
	Console::WriteLine("-file option specifies a file pattern to match files to be read from and written to.");
	Console::WriteLine("      To be used when no specific file path is given.");
	Console::WriteLine("-stdio option specifies that the input will be read from stdin and the");
	Console::WriteLine("       result will be written to stdout.");
	Console::WriteLine("-recurs is valid only if the -file option is specified.");
	Console::WriteLine("");
	Console::WriteLine("Values enclosed with [] are optional.");
	Console::WriteLine("Chunk option meaning: R=Remove, K=Keep, F=Force. 0|1|2 can be used too.");
	Console::WriteLine("");
	Console::WriteLine("Input examples:");
	Console::WriteLine("Handle a specific file:");
	Console::WriteLine("  pngoptimizercl icon.png");
	Console::WriteLine("Handle a specific file (alternate syntax):");
	Console::WriteLine("  pngoptimizercl -file:\"icon.png\"");
	Console::WriteLine("Handle specific file types in the current directory:");
	Console::WriteLine("  pngoptimizercl -file:\"*.png|*.bmp\"");
	Console::WriteLine("Handle any supported file in the current directory:");
	Console::WriteLine("  pngoptimizercl -file:\"*\"");
	Console::WriteLine("Handle specific file types in the current directory (recursive):");
	Console::WriteLine("  pngoptimizercl -file:\".png|*.bmp\" -recurs");
	Console::WriteLine("Handle a specific directory (recursive):");
	Console::WriteLine("  pngoptimizercl -file:\"gfx/\"");
	Console::WriteLine("Handle a file written to stdin and capture stdout to make a new file:");
	Console::WriteLine("  pngoptimizercl -stdio < icon.png > icon2.png");
	Console::WriteLine("");
}

#if defined(_WIN32)
// Use the W version of main on Windows to ensure we get a known text encoding (UTF-16)
int wmain(int argc, wchar_t** argv)
#elif defined(__linux__)
// Use the legacy main on Linux and assume UTF-8
int main(int argc, char** argv)
#endif

{
	ArgvParser ap(argc, argv);
	if( ap.HasFlag("help") )
	{
		// Explicitely ask to display help
		WriteHelp();
		return 0;
	}

	if( ap.HasFlag("version") )
	{
		WriteVersion();
		return 0;
	}

	StringArray argFilePaths = ap.GetRegularArgs();
	const bool fileFlag = ap.HasFlag("file");
	const bool stdioFlag = ap.HasFlag("stdio");

	if( argFilePaths.IsEmpty() && !fileFlag && !stdioFlag )
	{
		// No input, display help
		WriteHelp();

		// For Windows when we just double-click on the executable file from the file explorer
		if( Console::IsOwned() )
		{
			Console::Write("Press any key to continue");
			Console::WaitForKey();
		}
		return 0;
	}

	POApplicationConsole app;

	// PngOptimizer.exe -file:"myfile.png"

	POEngine engine;
	if( !engine.WarmUp() )
	{
		Console::Stderr().WriteLine("Warm-up failed");
		return 1;
	}
	engine.Progressing.Connect(&app, &POApplicationConsole::OnEngineProgressing);

	engine.m_settings.LoadFromArgv(ap);

	//////////////////////////////////////////////////////////////////
	if( !argFilePaths.IsEmpty() )
	{
		// Explicit file paths
		if( !engine.OptimizeMultiFilesDisk(argFilePaths) )
		{
			Console::Stderr().WriteLine(engine.GetLastErrorString());
			return 1;
		}
	}
	else if( fileFlag )
	{
		// -file
		String filePath = ap.GetFlagString("file");
		String dir, fileName;
		FilePath::Split(filePath, dir, fileName);
		StringArray filePaths;
		String joker;
		if( ap.HasFlag("recurs") )
		{
			filePaths.Add(dir);
			joker = fileName;
		}
		else
		{
			filePaths = Directory::GetFileNames(dir, fileName, true);
			if( filePaths.IsEmpty() )
			{
				// For pngoptimizercl, it is an error if we have nothing to optimize
				Color col = POEngine::ColorFromTextType(POEngine::TT_ErrorMsg);
				Console::Stderr().SetTextColor(col);
				Console::Stderr().WriteLine("File not found: " + filePath);
				Console::Stderr().ResetTextColor();
				return 1;
			}
		}

		if( !engine.OptimizeMultiFilesDisk(filePaths, joker) )
		{
			Console::Stderr().WriteLine(engine.GetLastErrorString());
			return 1;
		}
	}
	else
	{
		ASSERT(stdioFlag);
		g_stdioMode = true;

		if( !engine.OptimizeFileStdio() )
		{
			Console::Stderr().WriteLine(engine.GetLastErrorString());
			return 1;
		}
	}
	return 0;
}
