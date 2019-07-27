/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_MSGDIALOG_H
#define PO_MSGDIALOG_H

#include "Dialog.h"

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

class MsgDialog : public Dialog
{
public:
	MsgDialog(const String& content, const String& title,
		ChuiMsgType, ChuiButtonType);

private:
	String m_content;
	String m_title;
	ChuiMsgType m_msgType;
	ChuiButtonType m_buttonType;

private:
	virtual bool SetupUI();
};

#endif
