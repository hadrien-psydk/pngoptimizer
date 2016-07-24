///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_RESOURCEVERSION_H
#define CHUWIN32_RESOURCEVERSION_H

namespace chuwin32 {

class ResourceVersion
{
public:
	static chustd::String GetVersion();
	static chustd::String GetVersionFull();
	static chustd::String GetVersionFullComa();
	static bool GetVersion(int& nA, int& nB, int& nC, int& nD);

	static chustd::String GetVersion(const chustd::String& filePath);
	static chustd::String GetVersionFull(const chustd::String& filePath);
	static chustd::String GetVersionFullComa(const chustd::String& filePath);
	static bool GetVersion(const chustd::String& filePath, int& nA, int& nB, int& nC, int& nD);
};

} // namespace chuwin32

#endif // ndef CHUWIN32_RESOURCEVERSION_H
