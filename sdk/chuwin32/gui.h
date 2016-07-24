///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_GUI_H
#define CHUWIN32_GUI_H

#include "Window.h"

namespace chuwin32 {

class CheckButton : public Window
{

public:
	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	void Check(bool bCheck = true) { SendMessage(m_hWnd, BM_SETCHECK, bCheck ? BST_CHECKED : BST_UNCHECKED, 0); }
	bool IsChecked() const
	{
		if( SendMessage(m_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED )
		{
			return true;
		}
		return false;
	}
};
typedef CheckButton RadioButton;

///////////////////////////////
class EditBox : public Window
{
public:
	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	bool Create(const RECT& rc, HWND hParent, int nId);
	void SetSel(int nStart, int nStop)
	{
		::SendMessage(m_hWnd, EM_SETSEL, nStart, nStop);
	}
	void SetSelAll()
	{
		SetSel(0, -1);
	}
};

///////////////////////////////
class Button : public Window
{
public:
	void operator = (HWND hWnd) { m_hWnd = hWnd; }
};

///////////////////////////////
class ComboBox : public Window
{
public:
	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	int AddString(const chustd::String& str)
	{
		return (int)::SendMessage(m_hWnd, CB_ADDSTRING, 0, (LPARAM) str.GetBuffer());
	}

	void SetItemData(int nIndex, uint32 nData)
	{
		::SendMessage(m_hWnd, CB_SETITEMDATA, nIndex, nData);
	}

	int GetCurSel()
	{
		return (int)::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
	}

	uint32 GetItemData(int nIndex)
	{
		return (uint32)::SendMessage(m_hWnd, CB_GETITEMDATA, nIndex, 0);
	}

	void LimitText(int limit)
	{
		::SendMessage(m_hWnd, CB_LIMITTEXT, limit, 0);
	}
};

///////////////////////////////
class Slider : public Window
{
public:
	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	void SetRangeMin(int32 nMin, bool bRedraw = false)
	{
		::SendMessage(m_hWnd, TBM_SETRANGEMIN, BOOL(bRedraw), nMin);
	}

	void SetRangeMax(int32 nMax, bool bRedraw = false)
	{
		::SendMessage(m_hWnd, TBM_SETRANGEMAX, BOOL(bRedraw), nMax);
	}

	void SetRange(int32 nMin, int32 nMax, bool bRedraw = false)
	{
		SetRangeMin(nMin, bRedraw);
		SetRangeMax(nMax, bRedraw);
	}

	void SetPos(int32 nPos)
	{
		::SendMessage(m_hWnd, TBM_SETPOS, TRUE, nPos);
	}

	int32 GetPos()
	{
		return (int32) ::SendMessage(m_hWnd, TBM_GETPOS, 0, 0l);
	}
};

///////////////////////////////
class SysLink : public Window
{
public:
	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	bool Create(const RECT& rc, HWND hParent, int nId);
};

///////////////////////////////
class ColorButton : public Window
{
public:
	void operator = (HWND hWnd) { m_hWnd = hWnd; }

	bool Create(const RECT& rc, HWND hParent, int nId);
	void SetColor(COLORREF cr);
	COLORREF GetColor() const;
protected:
	COLORREF m_cr;
protected:
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	ATOM RegisterClass(const wchar* pszClassName);
};

////////////////////////////////////////
class DeviceContext
{
public:
	HDC m_hDC;

public:
	DeviceContext(HDC hDC) : m_hDC(hDC) {}

	void FillSolidRect(RECT& rect, COLORREF cr)
	{
		::SetBkColor(m_hDC, cr);
		::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	}

	void FillRect(RECT& rect, HBRUSH hbr)
	{
		::FillRect(m_hDC, &rect, hbr);
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

#endif // ndef CHUWIN32_GUI_H
