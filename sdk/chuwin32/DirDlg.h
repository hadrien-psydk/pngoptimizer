///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_DIRDLG_H
#define CHUWIN32_DIRDLG_H

namespace chuwin32 {

class DirDlg
{
public:
	void SetTitle(const chustd::String& strTitle);

	const chustd::String& GetDir() const;
	void SetStartDir(const chustd::String& strDir);

	int DoModal();

	DirDlg(HWND hParentWnd=NULL);	
	virtual ~DirDlg();

protected:
	HWND m_hParentWnd;
	chustd::String m_strTitle;
	chustd::String m_strStartDir;
	chustd::String m_strDir;

protected:
	static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg,
										  LPARAM lParam, LPARAM lpData);
	LPITEMIDLIST GetPIDLFromPath(const chustd::String& strPath);
};

} // namespace chuwin32

#endif // ndef CHUWIN32_DIRDLG_H
