///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirDlg.h"

namespace chuwin32 {\

///////////////////////////////////////////////////////////////////////////////
DirDlg::DirDlg()
{
}

///////////////////////////////////////////////////////////////////////////////
DirDlg::~DirDlg()
{

}

///////////////////////////////////////////////////////////////////////////////
// Allows going immediately into a specific directory at init
// lpData contains the directory ID
int CALLBACK DirDlg::BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
	{
		// wParam == FALSE => lParam == pItemIdList
		ITEMIDLIST* pidlSel = (ITEMIDLIST*) lpData;
		if( pidlSel )
		{
			::SendMessage(hWnd, BFFM_SETSELECTION, FALSE, lpData);
		}
	}
	return 0;
}

DialogResp DirDlg::DoModal(const Widget* parent)
{
	HWND hParent = 0;
	if( parent )
	{
		hParent = parent->GetHandle();
	}

	int nReturn = IDCANCEL;
	LPMALLOC pMalloc;    // Gets the Shell's default allocator
	if( ::SHGetMalloc(&pMalloc) == NOERROR )
	{
		// Get the id matching the start dir
		LPITEMIDLIST pidlSel = GetPIDLFromPath(m_startDir);
	
		wchar szBuffer[MAX_PATH];

		BROWSEINFOW bi;
		bi.hwndOwner = hParent;
		bi.pidlRoot = nullptr;
		bi.pszDisplayName = szBuffer;
		bi.lpszTitle = m_title.GetBuffer();
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseCallbackProc; // Fonction callback
		bi.lParam = LPARAM(pidlSel);  // lpData got from callback function
		bi.iImage = 0;
		LPITEMIDLIST pItemIDList = SHBrowseForFolderW(&bi);
		if( pItemIDList )
		{
			// Convert selected directory ID to a string
			wchar szBuffer2[MAX_PATH];
			if( ::SHGetPathFromIDListW(pItemIDList, szBuffer2) )
			{ 
				// Save the result
				m_dir = szBuffer2;
				nReturn = IDOK;
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pItemIDList);
		}
		
		if( pidlSel )
			pMalloc->Free(pidlSel);

		// Release the shell's allocator.
		pMalloc->Release();
	}
	return DialogResp(nReturn);
}

LPITEMIDLIST DirDlg::GetPIDLFromPath(const chustd::String& strPath)
{
	LPITEMIDLIST  pidl;
	LPSHELLFOLDER pDesktopFolder;
	OLECHAR       olePath[MAX_PATH];
	ULONG         chEaten;
	
	// Get a pointer to the Desktop's IShellFolder interface.
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
		// IShellFolder::ParseDisplayName requires the file name be in Unicode.
		int32 nByteCount = chustd::Math::Max(MAX_PATH, (strPath.GetLength() + 1 )* 2);
		chustd::Memory::Copy16(olePath, strPath.GetBuffer(), nByteCount);

		// Convert the path to an ITEMIDLIST.
		HRESULT hr = pDesktopFolder->ParseDisplayName(
			nullptr,
			nullptr,
			olePath,
			&chEaten,
			&pidl,
			nullptr);
		if (FAILED(hr))
		{
			// Handle error
			return nullptr;
		}
		
		// pidl now contains a pointer to an ITEMIDLIST for .\readme.txt.
		// This ITEMIDLIST needs to be freed using the IMalloc allocator
		// returned from SHGetMalloc()
	
		// Release the desktop folder object
		pDesktopFolder->Release();

		return pidl;
	}
	return nullptr;
}

void DirDlg::SetTitle(const chustd::String& title)
{
	m_title = title;
}

void DirDlg::SetStartDir(const chustd::String& startDir)
{
	m_startDir = startDir;

	if( m_startDir == L"\\" || m_startDir == L"/" )
	{
		// If the the initial directory is "/" or "\" then we change
		// its value to match a drive volume letter

		wchar szCurDir[MAX_PATH];
		GetCurrentDirectoryW(ARRAY_SIZE(szCurDir), szCurDir);

		m_startDir = chustd::File::GetDrive(szCurDir);
		m_startDir = m_startDir + L"\\";
	}
}

const chustd::String& DirDlg::GetDir() const
{
	return m_dir;
}

///////////////////////////////////////////////////////////////////////////////
}
