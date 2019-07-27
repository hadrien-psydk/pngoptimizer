/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MsgDialog.h"

MsgDialog::MsgDialog(const String& content, const String& title,
                     ChuiMsgType msgType, ChuiButtonType buttonType)
: m_content(content), m_title(title), m_msgType(msgType), m_buttonType(buttonType)
{
}

bool MsgDialog::SetupUI()
{
	GtkMessageType gmt = GTK_MESSAGE_INFO;
	GtkButtonsType gbt = GTK_BUTTONS_NONE;

	if( m_msgType == CMT_Warning )
	{
		gmt = GTK_MESSAGE_WARNING;
	}
	else if( m_msgType == CMT_Question )
	{
		gmt = GTK_MESSAGE_QUESTION;
	}

	if( m_buttonType == CBT_Ok )
	{
		gbt = GTK_BUTTONS_OK;
	}
	else if( m_buttonType == CBT_YesNo )
	{
		gbt = GTK_BUTTONS_YES_NO;
	}

	GtkWindow* parentHandle = nullptr;
	if( m_parent )
	{
		parentHandle = GTK_WINDOW(m_parent->GetHandle());
	}
	char msg[200];
	m_content.ToUtf8Z(msg);
	m_handle = gtk_message_dialog_new(parentHandle,
		GTK_DIALOG_DESTROY_WITH_PARENT,
        gmt, gbt, "%s", msg);
	return true;
}
