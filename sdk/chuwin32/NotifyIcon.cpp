#include "stdafx.h"
#include "NotifyIcon.h"

using namespace chuwin32;

NotifyIcon::NotifyIcon()
{
	m_hWndOwner = nullptr;
}

NotifyIcon::~NotifyIcon()
{
	Delete();
}

bool NotifyIcon::Create(HWND hWnd, HICON hIcon, int nCallbackMessage)
{
	NOTIFYICONDATAW nid;
	chustd::Memory::Zero(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON;
	if( nCallbackMessage != 0 )
	{
		nid.uFlags |= NIF_MESSAGE;
	}
	nid.uCallbackMessage = nCallbackMessage;
	nid.hIcon = hIcon;
	nid.szTip[0] = 0;

	m_hWndOwner = hWnd;

	BOOL bAddOk = Shell_NotifyIcon(NIM_ADD, &nid);
	return bAddOk != FALSE;
}

bool NotifyIcon::SetText(const chustd::String& str)
{
	NOTIFYICONDATAW nid;
	chustd::Memory::Zero(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWndOwner;
	nid.uID = 1;
	nid.uFlags = NIF_TIP;
	
	chustd::String strCut = str.Left(63);
	const int nLength = strCut.GetLength();
	chustd::Memory::Copy16(nid.szTip, str.GetBuffer(), nLength);
	nid.szTip[nLength] = 0;
	
	BOOL bOk = Shell_NotifyIcon(NIM_MODIFY, &nid);
	return bOk != FALSE;
}

void NotifyIcon::Delete()
{
	NOTIFYICONDATAW nid;
	chustd::Memory::Zero(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWndOwner;
	nid.uID = 1;

	BOOL bDelOk = Shell_NotifyIcon(NIM_DELETE, &nid);
	bDelOk;
}

