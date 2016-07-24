///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ContextMenu.h"

///////////////////////////////////////////////////////////////////////////////
using namespace chuwin32;
///////////////////////////////////////////////////////////////////////////////

ContextMenu::ContextMenu()
{
	m_hMenu = NULL;
	m_nPosition = TPM_LEFTALIGN;
}

ContextMenu::~ContextMenu()
{
	if( m_hMenu )
	{
		DestroyMenu(m_hMenu);
	}
}

bool ContextMenu::Create()
{
	m_hMenu = CreatePopupMenu();
	if( m_hMenu == NULL )
	{
		return false;
	}
	return true;
}

bool ContextMenu::AddItem(const chustd::String& strText, int nId)
{
	BOOL bOk = AppendMenuW(m_hMenu,
		MF_STRING,
		nId,
		strText.GetBuffer()
		);
	return bOk != FALSE;
}

void ContextMenu::Show(int x, int y, HWND hOwner)
{
	UINT nFlags = m_nPosition;
	nFlags |= TPM_RIGHTBUTTON;
	BOOL bOk = TrackPopupMenu(m_hMenu, nFlags, x, y, 0, hOwner, NULL);
	bOk;
}

void ContextMenu::SetPosition(UINT nPosition)
{
	m_nPosition = nPosition;
}
