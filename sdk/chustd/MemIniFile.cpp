///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemIniFile.h"
#include "FormatType.h"
#include "StringBuilder.h"
#include "File.h"
#include "TextEncoding.h"

using namespace chustd;

static const char k_szRealEndLine[] = "\n";
static const char k_szDosEndLine[] = "\r\n";

//////////////////////////////////////////////////////////////////////
void MemIniFile::Section::Dump(StringBuilder& sb, bool bCrLf) const
{
	const char* const pszEndLine = bCrLf ? k_szDosEndLine : k_szRealEndLine;

	sb += "[";
	sb += m_name;
	sb += "]";
	sb += pszEndLine;

	const int32 valueCount = m_apValues.GetSize();
	for(int i = 0; i < valueCount; ++i)
	{
		const Value* pValue = m_apValues[i];

		sb += pValue->m_name;
		sb += " = ";
		sb += pValue->m_strValue;
		sb += pszEndLine;
	}
}

int32 MemIniFile::Section::GetArrayIndex(const String& strValueName) const
{
	const uint16 firstCodeUnit = strValueName.GetAt(0);
	
	int index = -1;

	// Simple letter ?
	if( uint16('a') <= firstCodeUnit && firstCodeUnit <= uint16('z') )
	{
		index = firstCodeUnit - uint16('a');
	}
	else if( uint16('A') <= firstCodeUnit && firstCodeUnit <= uint16('Z') )
	{
		index = firstCodeUnit - uint16('A');
	}
	else
	{
		// Last array cell for variable names which do not start with a letter
		index = 26;
	}

	return index;
}

void MemIniFile::Section::SetValue(const String& strName, const String& strValue)
{
	if( strName.IsEmpty() )
		return;

	const int index = GetArrayIndex(strName);
	Array<Value*>& apValues = m_aapValues[index];

	String strUpperName = strName.ToUpperCase();

	// Check if the value is not already there
	for(int i = 0; i < apValues.GetSize(); ++i)
	{
		if( apValues[i]->m_strUpperName == strUpperName )
		{
			// Found !
			apValues[i]->m_strValue = strValue;
			return;
		}
	}

	// The value does not exist in the section, we create it
	Value* pValue = new Value;
	pValue->m_strUpperName = strUpperName;
	pValue->m_name = strName;
	pValue->m_strValue = strValue;

	// Add it to the linear array
	m_apValues.Add(pValue);

	// Add it to the map
	apValues.Add(pValue);
}

bool MemIniFile::Section::GetValue(const String& strName, String& strValue) const
{
	if( strName.IsEmpty() )
		return false;

	const int index = GetArrayIndex(strName);
	const Array<Value*>& apValues = m_aapValues[index];

	String strUpperName = strName.ToUpperCase();

	// Check if the value is not already there
	for(int i = 0; i < apValues.GetSize(); ++i)
	{
		if( apValues[i]->m_strUpperName == strUpperName )
		{
			// Found !
			strValue = apValues[i]->m_strValue;
			return true;
		}
	}
	return false;
}

const MemIniFile::ValuePtrArray& MemIniFile::Section::GetValues() const
{
	return m_apValues;
}

//////////////////////////////////////////////////////////////////////

MemIniFile::MemIniFile()
{
	m_pCurrentSection = nullptr;
}

MemIniFile::~MemIniFile()
{

}

bool MemIniFile::Load(const String& filePath)
{
	File file;
	if( !file.Open(filePath) )
	{
		return false;
	}

	ByteArray aContent = file.GetContent();
	file.Close();

	// Convert the file content into a string
	String strContent;

	// Byte order mark found for UTF-8 ?
	if( aContent.GetSize() >= 3 
		&& aContent[0] == 0xEF && aContent[1] == 0xBB && aContent[2] == 0xBF)
	{
		// Byte order mark found, remove it before parsing

		// Create a smaller array from the content array
		ByteArrayBox bab( aContent.GetPtr() + 3, aContent.GetSize() - 3);

		strContent = String::FromBytes(bab.ToByteArray(), TextEncoding::Utf8());
	}
	else
	{
		strContent = String::FromBytes(aContent, TextEncoding::Utf8());
	}

	StringArray astr = strContent.SplitByEndlines();

	m_aSections.Clear();
	m_aSections.EnsureCapacity(16);
	m_pCurrentSection = nullptr;

	// Find the sections
	const int32 stringCount = astr.GetSize();
	for(int iLine = 0; iLine < stringCount; ++iLine)
	{
		String str = astr[iLine].Trim();
		if( !str.IsEmpty() )
		{
			const int32 length = str.GetLength();
			if( str.GetAt(0) == ';' )
			{
				// Line comment
			}
			else if( str.GetAt(0) == '[' && str.GetAt(length - 1) == ']' )
			{
				String strSectionName(str.GetBuffer() + 1, length - 2);

				SetSection(strSectionName);
			}
			else
			{
				// Une valeur
				int32 foundAt = str.Find("=", 0);
				if( foundAt >= 0 )
				{
					if( m_pCurrentSection )
					{
						String strValueName = str.Left(foundAt).Trim();
						String strValue = str.Mid(foundAt + 1).Trim();
						
						// Remove any end of line comment
						const int32 firstLength = strValue.GetLength();
						for(int iChar = 0; iChar < firstLength; ++iChar)
						{
							uint16 c = strValue.GetAt(iChar);
							if( c == ';' )
							{
								strValue = strValue.Left(iChar).Trim();
								break;
							}
						}

						m_pCurrentSection->SetValue(strValueName, strValue);
					}
				}
			}
		}
	}

	m_pCurrentSection = nullptr;
	m_strCurrentSection.Empty();

	return true;
}

bool MemIniFile::Dump(const String& filePath, bool bCrLf)
{
	// Put byte order mark
	StringBuilder sb;
	sb += 0x0000FEFF; // Will generate 0xEF, 0xBB, 0xBF as UTF-8

	const char* const pszEndLine = bCrLf ? k_szDosEndLine : k_szRealEndLine;

	StringBuilder sbTopComment;
	if( !m_strCommentLine1.IsEmpty() )
	{
		sbTopComment += "; ";
		sbTopComment += m_strCommentLine1;
		sbTopComment += pszEndLine;
	}

	if( !m_strCommentLine2.IsEmpty() )
	{
		sbTopComment += "; ";
		sbTopComment += m_strCommentLine2;
		sbTopComment += pszEndLine;
	}
	
	if( !sbTopComment.IsEmpty() )
	{
		sbTopComment += pszEndLine; // Put an additional endline
		sb += sbTopComment.ToString();
	}

	const int32 sectionCount = m_aSections.GetSize();
	for(int iSection = 0; iSection < sectionCount; ++iSection)
	{
		m_aSections[iSection].Dump(sb, bCrLf);

		// One empty line for clearer file, but only if we are not dumping the last section
		// because we do not want two enlines at the end of the file
		if( iSection < (sectionCount - 1) )
		{
			sb += pszEndLine;
		}
	}

	File file;
	if( !file.Open(filePath, File::modeWrite) )
	{
		return false;
	}

	ByteArray aResult = sb.ToString().ToBytes(TextEncoding::Utf8());
	int32 written = file.Write(aResult.GetPtr(), aResult.GetSize());
	file.Close();

	return written == aResult.GetSize();
}

bool MemIniFile::SetSection(const String& strSection)
{
	if( m_pCurrentSection )
	{
		// If we are lucky we are just doing a fast pointer comparison
		if( m_strCurrentSection == strSection )
			return true;
	}

	m_strCurrentSection = strSection;

	String strUpperSection = strSection.ToUpperCase();
	
	m_pCurrentSection = nullptr;

	// Find the section (maybe it already exists)
	for(int i = 0; i < m_aSections.GetSize(); ++i)
	{
		if( m_aSections[i].m_strUpperName == strUpperSection )
		{
			// Yep !
			m_pCurrentSection = &m_aSections[i];
			break;
		}
	}

	if( m_pCurrentSection == nullptr )
	{
		// Create the section
		int32 pos = m_aSections.Add();
		m_pCurrentSection = &m_aSections[pos];
		m_pCurrentSection->m_name = strSection;
		m_pCurrentSection->m_strUpperName = strUpperSection;

		return false; // The section did not exist
	}
	return true;
}

bool MemIniFile::GetString(const String& strName, String& strValue) const
{
	if( m_pCurrentSection == nullptr )
		return false;

	return m_pCurrentSection->GetValue(strName, strValue);
}

bool MemIniFile::GetInt(const String& strName, int& value) const
{
	if( m_pCurrentSection == nullptr )
		return false;

	String strValue;
	if( !m_pCurrentSection->GetValue(strName, strValue) )
		return false;

	return strValue.ToInt(value);
}

bool MemIniFile::GetBool(const String& strName, bool& bValue) const
{
	if( m_pCurrentSection == nullptr )
		return false;

	String strValue;
	if( !m_pCurrentSection->GetValue(strName, strValue) )
		return false;

	return strValue.ToBool(bValue);
}

void MemIniFile::SetString(const String& strName, const String& strValue)
{
	if( m_pCurrentSection == nullptr )
		return;

	m_pCurrentSection->SetValue(strName, strValue);	
}

void MemIniFile::SetInt(const String& strName, int value)
{
	if( m_pCurrentSection == nullptr )
		return;

	String strValue = String::FromInt(value);
	m_pCurrentSection->SetValue(strName, strValue);
}

void MemIniFile::SetBool(const String& strName, bool bValue)
{
	if( m_pCurrentSection == nullptr )
		return;

	String strValue = bValue ? "1" : "0";
	m_pCurrentSection->SetValue(strName, strValue);
}

const MemIniFile::SectionArray& MemIniFile::GetSections() const
{
	return m_aSections;
}

void MemIniFile::SetCommentLine1(const String& strCommentLine1)
{
	m_strCommentLine1 = strCommentLine1;
}

void MemIniFile::SetCommentLine2(const String& strCommentLine2)
{
	m_strCommentLine2 = strCommentLine2;
}

