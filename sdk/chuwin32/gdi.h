///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_GDI_H
#define CHUWIN32_GDI_H

using namespace chustd;

namespace chuwin32 {

////////////////////////////////////////
class DeviceContext
{
public:
	HDC m_hDC;

public:
	DeviceContext(HDC hDC) : m_hDC(hDC) {}

	void FillSolidRect(Rect& rect, COLORREF cr)
	{
		RECT rc = {rect.x1, rect.y1, rect.x2, rect.y2 };
		::SetBkColor(m_hDC, cr);
		::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}

	void FillRect(Rect& rect, HBRUSH hbr)
	{
		RECT rc = {rect.x1, rect.y1, rect.x2, rect.y2 };
		::FillRect(m_hDC, &rc, hbr);
	}

	HGDIOBJ SelectObject(HGDIOBJ hObj)
	{
		return ::SelectObject(m_hDC, hObj);
	}

	COLORREF SetBkColor(COLORREF colBk)
	{
		return ::SetBkColor(m_hDC, colBk);
	}

	int SetBkMode(int mode)
	{
		return ::SetBkMode(m_hDC, mode);
	}

	void MoveTo(int x, int y)
	{
		POINT ptprev;
		::MoveToEx(m_hDC, x, y, &ptprev);
	}

	void LineTo(int x, int y)
	{
		::LineTo(m_hDC, x, y);
	}

	void MoveTo(POINT pt)
	{
		MoveTo(pt.x, pt.y);
	}

	void LineTo(POINT pt)
	{
		LineTo(pt.x, pt.y);
	}

};

class Pen
{
public:
	HPEN m_hPen;

public:
	operator HPEN() { return m_hPen; }

	Pen(int nWidth, COLORREF cr)
	{
		m_hPen = ::CreatePen(PS_SOLID, nWidth, cr);
	}

	~Pen()
	{
		::DeleteObject(m_hPen);
	}
};


} // namespace chuwin32

#endif // ndef CHUWIN32_DIRDLG_H

