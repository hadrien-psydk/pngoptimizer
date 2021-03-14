///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUI_WIDGET_H
#define CHUI_WIDGET_H

using namespace chustd;

#if defined(_WIN32)
typedef HWND  WIDGET_HANDLE;
typedef uint32 DIALOG_ID;
typedef HFONT FONT_HANDLE;
#elif defined(__linux__)
typedef struct GtkWidget*  WIDGET_HANDLE;
typedef const char* DIALOG_ID;
typedef void* FONT_HANDLE;
#endif

namespace chuwin32 {

///////////////////////////////////////
class Icon
{
public:
	void* m_handle;
};

/////////////////////////////
enum CmdShow
{
	CS_Hide,
	CS_Show
};

/////////////////////////////
class Widget
{
public:
	Widget() : m_handle(0) {}
	Widget(WIDGET_HANDLE handle) : m_handle(handle) {}
	virtual ~Widget() {};

	void operator=(WIDGET_HANDLE handle) { m_handle = handle; }

	void Enable(bool enable = true);
	void Disable() { Enable(false); }
	void SetIcon(const Icon& icon, bool bigIcon);
	void SetFocus();
	void Show(CmdShow cs = CS_Show);
	void Hide() { Show(CS_Hide); }
	void CenterWindow(bool avoidHide = false);
	void EnsureVisible();
	void SetText(const chustd::String& str);
	void SetTextInt(int value);
	chustd::String GetText() const;
	int GetTextInt() const;
	Rect GetWindowRect() const;
	Rect GetWindowRectAero() const;
	Rect GetClientRect() const;
	Rect GetRelativeRect();

	WIDGET_HANDLE GetHandle() const { return m_handle; }
	WIDGET_HANDLE GetParent();

	void ScreenToClient(Rect& rect);
	void ClientToScreen(RECT& rect);

	FONT_HANDLE GetFont() const;
	void SetFont(FONT_HANDLE hFont, bool redraw = true);
	bool SetSameFontThanParent();

	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  int x, int y, int w, int h, WIDGET_HANDLE hParent, HMENU hMenu = 0);
	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  int x, int y, int w, int h, WIDGET_HANDLE hParent, int nId);
	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  const RECT& rect, WIDGET_HANDLE hParent, HMENU hMenu = 0);
	bool CreateEx(uint32 nExStyle, const wchar* pszClassName, const wchar* pszTitle, uint32 nStyle,
			  const RECT& rect, WIDGET_HANDLE hParent, int nId);

	int GetScrollPos(int bar);
	int SetScrollPos(int bar, int pos, bool redraw = false);
	bool IsVisible() const;
	bool IsEnabled() const;
	bool Update();
	int GetId() const;

protected:
	WIDGET_HANDLE m_handle;

protected:
	void SetHandle(WIDGET_HANDLE);
	virtual LRESULT WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProcStatic(WIDGET_HANDLE handle, UINT nMsg, WPARAM wParam, LPARAM lParam);

private:
	// Implement this to fire the WM_COMMAND related event
	virtual void OnCommand(uintptr_t param);

	friend class Dialog; // For OnCommand()
};

} // namespace chuwin32

#endif
