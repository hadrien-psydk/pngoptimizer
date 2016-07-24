/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DlgAskForFileName.h"

///////////////////////////////////////////////////////////////////////////////
using namespace chuwin32;
///////////////////////////////////////////////////////////////////////////////

DlgAskForFileName::DlgAskForFileName()
{
	m_bIncrementFileName = false;
}

DlgAskForFileName::~DlgAskForFileName()
{

}

LRESULT DlgAskForFileName::DlgProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch(nMsg)
	{
	case WM_INITDIALOG:
		return OnInitDialog(HWND(wParam), lParam);

	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{ 
		case IDCANCEL:
			EndDialog(IDCANCEL);
			return TRUE;

		case IDOK:
			if( StoreControlValues() )
			{
				EndDialog(IDOK);
			}
			return TRUE;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

// Returns true if found
bool DlgAskForFileName::GetDigitGroup(const String& str, int& nGroupStart, int& nGroupStop)
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
void DlgAskForFileName::IncrementFileName()
{
	if( !m_bIncrementFileName )
	{
		return;
	}

	String strName = m_strFileName;
	String strExt = ".png";
	if( strName.EndsWith(strExt) )
	{
		int nStop = m_strFileName.GetLength() - strExt.GetLength();
		strName = strName.Left(nStop);
	}

	int nGroupStart, nGroupStop;
	if( !GetDigitGroup(strName, nGroupStart, nGroupStop) )
	{
		m_strFileName = strName + " 2.png";
		return;
	}

	String strNum = strName.Mid(nGroupStart, nGroupStop - nGroupStart);
	int32 nNum;
	if( !strNum.ToInt(nNum) )
	{
		m_strFileName = strName + " 2.png";
		return;
	}

	// Check if there are leading 0
	String strSameNum = String::FromInt(nNum);
	String strPrefix = strNum.ReplaceAll(strSameNum, "");
	String strNewNum = strPrefix + String::FromInt(nNum + 1);

	m_strFileName = strName.Left(nGroupStart) + strNewNum + strName.Mid(nGroupStop) + ".png";
}

BOOL DlgAskForFileName::OnInitDialog(HWND, LPARAM)
{
	IncrementFileName();

	m_editFileName = GetItem(IDC_EDIT_FILENAME);
	m_editFileName.SetText(m_strFileName);
	m_editFileName.SetFocus();
	
	// Select all except the .png part of the file name
	int nSelStop = -1;
	String strExt = ".png";
	if( m_strFileName.EndsWith(strExt) )
	{
		nSelStop = m_strFileName.GetLength() - strExt.GetLength();
	}
	m_editFileName.SetSel(0, nSelStop);
	
	CenterWindow(true);
	return FALSE; // Return FALSE as we set the focus ourselves
}

int DlgAskForFileName::DoModal(HWND hParent)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	return DoModalHelper(hInstance, IDD_ASKFORFILENAME, hParent);
}

bool DlgAskForFileName::StoreControlValues()
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
			strDir = FilePath::Combine(strDir, m_strScreenshotDir);
		}
	}
	else
	{
		strDir = m_strScreenshotDir;
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
	
	String strFinalPath = FilePath::Combine(strDir, strPath);
	if( File::Exists(strFinalPath) )
	{
		String strText = strFinalPath + L" already exists.\nOverwrite ?";
		int nRet = ::MessageBoxW(m_hWnd, strText.GetBuffer(), GetText().GetBuffer(), MB_ICONEXCLAMATION | MB_YESNO);
		if( nRet == IDNO )
		{
			return false;
		}
	}

	m_strFileName = strPath;

	return true;
}
