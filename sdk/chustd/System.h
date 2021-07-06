///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_SYSTEM_H
#define CHUSTD_SYSTEM_H

namespace chustd {

class String;
class System  
{
public:
	static uint32 GetTime();
	static uint64 GetTime64();
	static String GetFontsDirectory(); // Replace with FontFilePathFromFamilyName() ?
	
	// Get the user specific directory where to store applications settings
	// On Windows, typically : C:\Documents and Settings\Gwendoline\Application Data
	// On Linux: ~/.config
	static String GetUserConfigDirectory();
};

} // namespace chustd

#endif // ndef CHUSTD_SYSTEM_H
