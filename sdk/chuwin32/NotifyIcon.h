///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_NOTIFYICON_H
#define CHUWIN32_NOTIFYICON_H

namespace chuwin32 {

class NotifyIcon
{
public:
	bool Create(HWND hWnd, HICON hIcon, int nCallbackMessage = 0);
	bool SetText(const chustd::String& str);
	void Delete();

	NotifyIcon();
	~NotifyIcon();

private:
	HWND m_hWndOwner;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_NOTIFYICON_H
