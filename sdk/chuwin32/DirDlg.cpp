///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DirDlg.h"

//////////////////////////////////////////////////////////////////////
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////

DirDlg::DirDlg(HWND hParentWnd/*=NULL*/)
{
	m_hParentWnd = hParentWnd;
}

DirDlg::~DirDlg()
{

}

// Permet de se placer tout de suite dans un répertoire à l'init
// lpData contient l'ID du répertoire
int CALLBACK DirDlg::BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
	{
		// wParam == FALSE => lParam == pItemIdList
		ITEMIDLIST* pidlSel = (ITEMIDLIST*) lpData;
		if( pidlSel != NULL )
		{
			::SendMessage(hWnd, BFFM_SETSELECTION, FALSE, lpData);
		}
	}
	return 0;
}

int DirDlg::DoModal()
{
	int nReturn = IDCANCEL;
	LPMALLOC pMalloc;    // Gets the Shell's default allocator
	if( ::SHGetMalloc(&pMalloc) == NOERROR )
	{
		// Get the id matching the start dir
		LPITEMIDLIST pidlSel = GetPIDLFromPath(m_strStartDir);
	
		wchar szBuffer[MAX_PATH];

		BROWSEINFOW bi;
		bi.hwndOwner = m_hParentWnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = szBuffer;
		bi.lpszTitle = m_strTitle.GetBuffer();
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseCallbackProc; // Fonction callback
		bi.lParam = LPARAM(pidlSel);    // lpDatat récupéré dans la fonction callback
		bi.iImage = 0;
		LPITEMIDLIST pItemIDList = SHBrowseForFolderW(&bi);
		if( pItemIDList != NULL )
		{
			// Converion ID répertoire sélectionné vers chaîne de carcatères
			wchar szBuffer[MAX_PATH];
			if( ::SHGetPathFromIDListW(pItemIDList, szBuffer) )
			{ 
				// Save the result
				m_strDir = szBuffer;
				nReturn = IDOK;
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pItemIDList);
		}
		
		if( pidlSel != NULL )
			pMalloc->Free(pidlSel);

		// Release the shell's allocator.
		pMalloc->Release();
	}
	return nReturn;
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
			NULL,
			NULL,
			olePath,
			&chEaten,
			&pidl,
			NULL);
		if (FAILED(hr))
		{
			// Handle error
			return NULL;
		}
		
		// pidl now contains a pointer to an ITEMIDLIST for .\readme.txt.
		// This ITEMIDLIST needs to be freed using the IMalloc allocator
		// returned from SHGetMalloc()
	
		// Release the desktop folder object
		pDesktopFolder->Release();

		return pidl;
	}
	return NULL;
}

void DirDlg::SetTitle(const chustd::String& strTitle)
{
	m_strTitle = strTitle;
}

void DirDlg::SetStartDir(const chustd::String& strStartDir)
{
	m_strStartDir = strStartDir;

	if( m_strStartDir == L"\\" || m_strStartDir == L"/" )
	{
		// If the the initial directory is "/" or "\" then we change
		// its value to match a drive volume letter

		wchar szCurDir[MAX_PATH];
		GetCurrentDirectoryW(ARRAY_SIZE(szCurDir), szCurDir);

		m_strStartDir = chustd::File::GetDrive(szCurDir);
		m_strStartDir = m_strStartDir + L"\\";
	}
}

const chustd::String& DirDlg::GetDir() const
{
	return m_strDir;
}
