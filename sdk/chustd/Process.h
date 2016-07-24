///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_PROCESS_H
#define CHUSTD_PROCESS_H

namespace chustd {

class String;
class StringArray;

class Process  
{
public:
	// Note: fonctions statiques ou dynamique avec une seule statique : Process* GetCurrent() ?

	// Returns the current directory of the process
	static String GetCurrentDirectory();

	// Changes the current directory of the process
	static void SetCurrentDirectory(const String& dirPath);

	// Gets the current process ID
	static uint16 GetCurrentId();

	// Gets the process command line
	// Note: sans le nom de l'exe serait bien
	static String GetCommandLine();

	// Split a command line into words
	// Words enclosed in double-quotes are considered as one word
	static StringArray CommandLineToArgv(const String& cmdLine);

	// Returns the path of the executable that originated the process
	static String GetExecutablePath();

	// Returns the directory of the executable that originated the process
	static String GetExecutableDirectory();
};

} // namespace chustd

#endif // ndef CHUSTD_PROCESS_H
