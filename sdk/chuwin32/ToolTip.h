///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_TOOLTIP_H
#define CHUWIN32_TOOLTIP_H

namespace chuwin32 {

class ToolTip
{
public:
	bool Create(HWND hwndParent);
	void SetText(const chustd::String& strText);

	void TrackActivate(bool bShow);
	void TrackPosition(int x, int y);
	void NewRect(const RECT& rect);

	ToolTip();
	virtual ~ToolTip();

private:
	chustd::String m_strText;
	TOOLINFO m_ti;
	bool m_bVisible;
	HWND m_handle;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_TOOLTIP_H
