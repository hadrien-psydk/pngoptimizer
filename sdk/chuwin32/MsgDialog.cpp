///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MsgDialog.h"

///////////////////////////////////////////////////////////////////////////////
namespace chuwin32 {\

MsgDialog::MsgDialog(const String& content, const String& title,
                     ChuiMsgType msgType, ChuiButtonType buttonType)
: m_content(content), m_title(title), m_msgType(msgType), m_buttonType(buttonType)
{
}

DialogResp MsgDialog::DoModal(const Window* parent)
{
	HWND hParent = nullptr;
	if( parent )
	{
		hParent = parent->GetHandle();
	}
	uint32 flags = 0;

	if( m_msgType == CMT_Warning )
	{
		flags |= MB_ICONEXCLAMATION;
	}
	else if( m_msgType == CMT_Question )
	{
		flags |= MB_ICONQUESTION;
	}

	if( m_buttonType == CBT_Ok )
	{
		flags |= MB_OK;
	}
	else if( m_buttonType == CBT_YesNo )
	{
		flags |= MB_YESNO;
	}

	int ret = MessageBoxW(hParent, m_content.GetBuffer(), m_title.GetBuffer(), flags);
	return DialogResp(ret);
}

///////////////////////////////////////////////////////////////////////////////
}
