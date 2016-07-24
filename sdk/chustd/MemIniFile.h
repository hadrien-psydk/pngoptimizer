///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_MEMINIFILE_H
#define CHUSTD_MEMINIFILE_H

#include "String.h"
#include "Array.h"
#include "FixArray.h"
#include "PtrArray.h"
#include "File.h"

namespace chustd {

class StringBuilder;

// Load or store a .INI type file
class MemIniFile
{
public:
	// Loads in memory a INI file
	// Note: the encoding used is UTF-8
	bool Load(const String& filePath);

	// Dumps to file the stored sections and values
	// bCrLf: true to use \r\n at the end of lines instead of \n
	// Note: the encoding used is UTF-8
	bool Dump(const String& filePath, bool bCrLf = false);

	// Returns true if the section exists
	// Else, creates the section and returns false
	bool SetSection(const String& strSection);
	
	// Returns true if the variable was found
	bool GetString(const String& strName, String& strValue) const;
	bool GetInt(const String& strName, int& value) const;
	bool GetBool(const String& strName, bool& bValue) const;

	void SetString(const String& strName, const String& strValue);
	void SetInt(const String& strName, int value);
	void SetBool(const String& strName, bool bValue);

	// Appears at the top of the file in the dumped file
	void SetCommentLine1(const String& strCommentLine1);
	void SetCommentLine2(const String& strCommentLine2);

	MemIniFile();
	~MemIniFile();

public:
	class Value
	{
	public:
		String m_name;
		String m_strUpperName; // Upper case name for comparisons
		String m_strValue;
	};

	typedef PtrArray<Value> ValuePtrArray;

	class Section
	{
	public:
		String m_name;
		String m_strUpperName; // Upper case name for comparisons

	public:
		void SetValue(const String& strName, const String& strValue);
		bool GetValue(const String& strName, String& strValue) const;

		void Dump(chustd::StringBuilder& sb, bool bCrLf) const;

		const ValuePtrArray& GetValues() const;
	private:
		// Fast read access
		FixArray< Array<Value*>, 27 > m_aapValues; // 26 letters + 1 for others
		// Linear access
		PtrArray<Value> m_apValues;
	private:
		int32 GetArrayIndex(const String& strValueName) const;
	};

	typedef Array< Section > SectionArray;
	const SectionArray& GetSections() const;

private:
	SectionArray m_aSections;
	
	Section* m_pCurrentSection; // null if m_strCurrentSection does not exist
	String m_strCurrentSection; // Name given by SetSection

	String m_strCommentLine1;
	String m_strCommentLine2;
};

} // namespace chustd

#endif // ndef CHUSTD_MEMINIFILE_H
