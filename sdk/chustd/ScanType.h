///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_SCANTYPE_H
#define CHUSTD_SCANTYPE_H

namespace chustd {

// Used by String::Scan
typedef bool (*PFN_SCANTYPE)(const char*, int32&, char, void*);

// Scan Double Cast. The space between T##ArgNum and the * is important
// Used by String::Scan
#define SCAN_DC(ArgNum) (PFN_SCANTYPE) (bool (*)(const char*, int32&, char, T##ArgNum *)) & chustd::ScanType, &t##ArgNum

class String;

// Gets a value from a string
// psz :      string, starting at psz + advance
// advance : current position (should be incremented after the scan)
// cScanArg : optional ScanArgument
// pResult :  pointer to the value to be assigned
// Returns true if something correct could be scanned

// UTF-16 versions
bool ScanType(const wchar* psz, int& advance, char cScanArg, int32* pResult);
bool ScanType(const wchar* psz, int& advance, char cScanArg, int16* pResult);
bool ScanType(const wchar* psz, int& advance, char cScanArg, int8*  pResult);

bool ScanType(const wchar* psz, int& advance, char cScanArg, uint32* pResult);
bool ScanType(const wchar* psz, int& advance, char cScanArg, uint16* pResult);
bool ScanType(const wchar* psz, int& advance, char cScanArg, uint8*  pResult);

bool ScanType(const wchar* psz, int& advance, char cScanArg, float64* pResult);
bool ScanType(const wchar* psz, int& advance, char cScanArg, float32* pResult);

bool ScanType(const wchar* psz, int& advance, char cScanArg, bool* pResult);

bool ScanType(const wchar* psz, int& advance, char cScanArg, chustd::String* pResult);

} // namescape chustd


// Note : you can easilly add ScanType functions in your program to scan your own types :
/*
namespace chustd
{
	bool ScanType(const char* psz, int32& advance, char cScanArg, MyType* pResult)
	{
		// The code to scan one MyType
	}
}
*/

#endif // ndef CHUSTD_SCANTYPE_H
