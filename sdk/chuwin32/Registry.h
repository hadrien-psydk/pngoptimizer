///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_REGISTRY_H
#define CHUWIN32_REGISTRY_H

namespace chuwin32 {

class Registry  
{
public:
	bool Open(HKEY hKeyRoot, const chustd::String& strPath, bool bCreateIfDoesntExist = true);
	void Close();

	int32          ReadInt32(const chustd::String& strValueName, int32 nDefault);
	bool           ReadBool(const chustd::String& strValueName, bool bDefault);
	chustd::String ReadString(const chustd::String& strValueName, const chustd::String& strDefault);

	bool WriteInt32(const chustd::String& strValueName, int32 nValue);
	bool WriteBool(const chustd::String& strValueName, bool bValue);
	bool WriteString(const chustd::String& strValueName, const chustd::String& strValue);

	bool Flush();

	chustd::StringArray GetKeyNamesW(LPCSTR pszJoker);

	static bool StringMatch(const chustd::String& strTest, const chustd::String& strJoker);

	Registry();
	~Registry();

	HKEY GetHandle() const { return m_hKey; }
private:
	HKEY m_hKey;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_REGISTRY_H
