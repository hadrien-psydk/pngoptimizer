///////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizerCL application
// Copyright (C) 2002/2013 Hadrien Nilsson - psydk.org
//
// PngOptimizerCL is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Foobar is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Foobar; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

using namespace chustd;

class POApplicationConsole
{
public:
	void OnEngineProgressing(const POEngine::ProgressingArg& arg)
	{
		Color32 col = POEngine::ColorFromTextType(arg.textType);
		if( col.r == 0 && col.g == 0 && col.b == 0 )
		{
			Console::ResetTextColor();
		}
		else
		{
			Console::SetTextColor(col);
		}
		Console::Write(arg.text);
	}
};

#if defined(_WIN32)
// Use the W version of main on Windows to ensure we get a known text encoding (UTF-16)
int wmain(int argc, wchar_t** argv)
#elif defined(__linux__)
// Use the legacy main on Linux and assume UTF-8
int main(int argc, char** argv)
#endif

{
	String cmdLine = Process::GetCommandLine();

	ArgvParser ap(argc, argv);
	if( !ap.HasFlag("file") )
	{
		// Display usage
		String version = "PngOptimizerCL 2.4.3";
#if defined(_M_X64)
		version = version + String(" (x64)");
#elif defined(_M_IX86)  
		version = version + String(" (x86)");
#endif
		Console::WriteLine(version + " \xA9 2002/2014 Hadrien Nilsson - psydk.org");
		Console::WriteLine("Converts GIF, BMP and TGA files to optimized PNG files.");
		Console::WriteLine("Optimizes and cleans PNG files.");
		Console::WriteLine("");
		Console::WriteLine("Usage:  PngOptimizerCL -file:\"yourfile.png\" [-recurs]");
		POEngineSettings::WriteArgvUsage("  ");
		Console::WriteLine("");
		Console::WriteLine("Values enclosed with [] are optional.");
		Console::WriteLine("Chunk option meaning: R=Remove, K=Keep, F=Force. 0|1|2 can be used too.");
		Console::WriteLine("");
		Console::WriteLine("Input examples:");
		Console::WriteLine("Handle a specific file:");
		Console::WriteLine("  PngOptimizerCL -file:\"icon.png\"");
		Console::WriteLine("Handle specific file types in the current directory:");
		Console::WriteLine("  PngOptimizerCL -file:\"*.png|*.bmp\"");
		Console::WriteLine("Handle any supported file in the current directory:");
		Console::WriteLine("  PngOptimizerCL -file:\"*\"");
		Console::WriteLine("Handle specific file types in the current directory (recursive):");
		Console::WriteLine("  PngOptimizerCL -file:\".png|*.bmp\" -recurs");
		Console::WriteLine("Handle a specific directory (recursive):");
		Console::WriteLine("  PngOptimizerCL -file:\"gfx/\"");
		Console::WriteLine("");
	
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
		Console::WriteLine("Warm-up failed");
		return 1;
	}
	engine.Progressing.Handle(&app, &POApplicationConsole::OnEngineProgressing);

	engine.m_settings.LoadFromArgv(ap);

	//////////////////////////////////////////////////////////////////
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
			// For PngOptimizerCL, it is an error if we have nothing to optimize
			Color32 col = POEngine::ColorFromTextType(POEngine::TT_ErrorMsg);
			Console::SetTextColor(col);
			Console::WriteLine("File not found: " + filePath);
			Console::ResetTextColor();
			return 1;
		}
	}

	if( !engine.OptimizeFiles(filePaths, joker) )
	{
		return 1;
	}
	return 0;
}
