///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_MSGDIALOG_H
#define CHUI_MSGDIALOG_H

#include "Dialog.h"

namespace chuwin32 {

enum ChuiMsgType
{
	CMT_Warning,
	CMT_Question
};

enum ChuiButtonType
{
	CBT_Ok,
	CBT_YesNo
};

class MsgDialog
{
public:
	MsgDialog(const String& content, const String& title, ChuiMsgType, ChuiButtonType);
	DialogResp DoModal(const Window* parent);

private:
	String m_content;
	String m_title;
	ChuiMsgType m_msgType;
	ChuiButtonType m_buttonType;
};

} // namespace chuwin32

#endif // ndef CHUI_DIALOG_H
