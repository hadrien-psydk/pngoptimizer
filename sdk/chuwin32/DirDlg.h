///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_DIRDLG_H
#define CHUI_DIRDLG_H

#include "Dialog.h"

namespace chuwin32 {\

class DirDlg
{
public:
	DirDlg();
	virtual ~DirDlg();

	void SetTitle(const chustd::String& title);
	const chustd::String& GetDir() const;
	void SetStartDir(const chustd::String& startDir);

	DialogResp DoModal(const Widget* parent);

protected:
	chustd::String m_title;
	chustd::String m_startDir;
	chustd::String m_dir;

protected:
	static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg,
										  LPARAM lParam, LPARAM lpData);
	LPITEMIDLIST GetPIDLFromPath(const chustd::String& strPath);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
