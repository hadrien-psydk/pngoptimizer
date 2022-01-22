///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_CONSOLE_H
#define CHUSTD_CONSOLE_H

#include "ImageFormat.h"

namespace chustd {\

class String;

enum class StdFileType
{
	Stdin,
	Stdout,
	Stderr
};

class IConsoleWriter
{
public:
	virtual void Write(const String& str) = 0;
	virtual void WriteLine(const String& str) = 0;

	virtual void SetTextColor(Color col) = 0;
	virtual void ResetTextColor() = 0;

protected:
	virtual ~IConsoleWriter() {}
};

class Console
{
public:
	static IConsoleWriter& Stdout();
	static IConsoleWriter& Stderr();

	static void Write(const String& str)     { Stdout().Write(str); }
	static void WriteLine(const String& str) { Stdout().WriteLine(str); }

	// Gets an input string from the user. The user must validate its data with the 'Enter' key.
	// Returns true if an input string could be get
	static bool WaitForInput(String& str);

	// strPrompt : string shown before user input characters
	static bool WaitForInput(const String& prompt, String& input);

	// Gets a character from the user.
	// Returns the unicode code point of the corresponding pressed key
	static uint32 WaitForKey();

	static bool IsOwned();
};

} // namespace chustd

#endif // ndef CHUSTD_CONSOLE_H
