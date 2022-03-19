/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "AskForFileNameDlg.h"

///////////////////////////////////////////////////////////////////////////////
AskForFileNameDlg::AskForFileNameDlg() : Dialog(IDD_ASKFORFILENAME)
{
	m_incrementFileName = false;
}

/////////////////////////////////////////////////////////////////////////////////////
bool AskForFileNameDlg::SetupUI()
{
	m_editFileName = GetItem(IDC_EDIT_FILENAME);
	m_editFileName.SetFocus();
	return false; // Return false as we set the focus ourselves
}

/////////////////////////////////////////////////////////////////////////////////////
void AskForFileNameDlg::SetupConnections()
{
}

/////////////////////////////////////////////////////////////////////////////////////
void AskForFileNameDlg::LoadValues()
{
	IncrementFileName();

	m_editFileName.SetText(m_fileName);
	// Select all except the .png part of the file name
	int selStop = -1;
	String strExt = ".png";
	if( m_fileName.EndsWith(strExt) )
	{
		selStop = m_fileName.GetLength() - strExt.GetLength();
	}
	m_editFileName.SetSel(0, selStop);
}

/////////////////////////////////////////////////////////////////////////////////////
bool AskForFileNameDlg::StoreValues()
{
	String strPath = m_editFileName.GetText();

	// Maybe the user put a path in the file name. I wonder who may have the idea to do this,
	// but we allow it anyway
	// Example: blabla/myshot.png
	String strDir, strFileName;
	FilePath::Split(strPath, strDir, strFileName);

	//////////////////////////////////////////////////////////////////
	// Determine the fulll target directory
	if( !strDir.IsEmpty() )
	{
		if( !File::IsAbsolutePath(strDir) )
		{
			strDir = FilePath::Combine(strDir, m_screenshotDir);
		}
	}
	else
	{
		strDir = m_screenshotDir;
	}

	//////////////////////////////////////////////////////////////////
	// Automatically add a "png" extension to the file name

	const String strWantedExt = L"png";

	String strExt = FilePath::GetExtension(strPath).ToLowerCase();
	if( strExt != strWantedExt )
	{
		strPath = strPath + L"." + strWantedExt;
	}

	//////////////////////////////////////////////////////////////////
	// Verify file existence, warn the user if necessary

	String finalPath = FilePath::Combine(strDir, strPath);
	if( File::Exists(finalPath) )
	{
		String text = finalPath + L" already exists.\nOverwrite ?";
		MsgDialog md(text, GetText(), CMT_Warning, CBT_YesNo);
		if( md.DoModal(this) == DialogResp::No )
		{
			return false;
		}
	}

	m_fileName = strPath;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if found
bool AskForFileNameDlg::GetDigitGroup(const String& str, int& nGroupStart, int& nGroupStop)
{
	// Button.png --> Button 2.png
	// Button 2.png --> Button 3.png
	// Button_2.png --> Button_3.png
	// Button 2 Disabled.png --> Button 3 Disabled.png
	// Button 2 3 xx.png --> Button 3 3 xx.png
	// Button 2 3 xx.png --> Button 3 3 xx.png
	// Button2 3 xx.png --> Button2 4 xx.png
	// 5 xx.png --> 6 xx.png

	int32 nStartAt = 0;
	for(;;)
	{
		if( nStartAt >= str.GetLength() )
		{
			break;
		}
		// Find digits
		int nPos = str.FindOneOf("0123456789", nStartAt);
		if( nPos < 0 )
		{
			// No digit found
			return false;
		}

		wchar cPrev = 0;
		int nPosPrev = nPos - 1;
		if( nPosPrev >= 0 )
		{
			cPrev = str.GetAt(nPosPrev);
		}

		if( !(cPrev == 0 || cPrev == ' ' || cPrev == '_' || cPrev == '-') )
		{
			// Continue in the string
			nStartAt = nPos + 1;
		}
		else
		{
			// A good candidate
			bool bSuccess = true;
			int i = nPos;
			for(; i < str.GetLength(); ++i)
			{
				wchar c = str.GetAt(i);
				if( '0' <= c && c <= '9' )
				{
					// Continue
				}
				else if( c == ' ' || c == '_' || c == '-' )
				{
					// We accept only "xx 123 yy" or "xx 123_yy" or "xx_123_yy" or "xx-123-yy" etc.

					// Success
					bSuccess = true;
					break;
				}
				else
				{
					bSuccess = false;
					break;
				}
			}

			if( bSuccess )
			{
				nGroupStart = nPos;
				nGroupStop = i;
				return true;
			}

			nStartAt = i;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Increments the current file name
// a4.png --> a5.png
// a04.png --> a05.png
// a4b.png --> a5b.png
// xyz.png --> xyz 2.png
void AskForFileNameDlg::IncrementFileName()
{
	if( !m_incrementFileName )
	{
		return;
	}

	String strName = m_fileName;
	String strExt = ".png";
	if( strName.EndsWith(strExt) )
	{
		int nStop = m_fileName.GetLength() - strExt.GetLength();
		strName = strName.Left(nStop);
	}

	int nGroupStart, nGroupStop;
	if( !GetDigitGroup(strName, nGroupStart, nGroupStop) )
	{
		m_fileName = strName + " 2.png";
		return;
	}

	String strNum = strName.Mid(nGroupStart, nGroupStop - nGroupStart);
	int32 nNum;
	if( !strNum.ToInt(nNum) )
	{
		m_fileName = strName + " 2.png";
		return;
	}

	// Check if there are leading 0
	String strSameNum = String::FromInt(nNum);
	String strPrefix = strNum.ReplaceAll(strSameNum, "");
	String strNewNum = strPrefix + String::FromInt(nNum + 1);

	m_fileName = strName.Left(nGroupStart) + strNewNum + strName.Mid(nGroupStop) + ".png";
}

