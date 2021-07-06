///////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWnd.h"

#include "resource.h"
#include "POApplication.h"

#include "PngOptionsDlg.h"
#include "ScreenshotsOptionsDlg.h"
#include "AboutDlg.h"

///////////////////////////////////////////////////////////////////////////////
MainWnd::MainWnd()
{
	m_alwaysOnTop = false;
	m_pApp = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// [in] welcomeMsg  Message to display when the content is empty (first creation
//                  or when a clear command is done)
bool MainWnd::Create(const String& title, RECT rcWnd, bool alwaysOnTop,
                     const String& welcomeMsg, POApplication* pApp)
{
	m_pApp = pApp;

	HINSTANCE hInstance = m_pApp->m_hInstance;
	// Register window class

	static const wchar szClassName[] = L"PngOptimizer Main Window Class";

	WNDCLASSEXW wcex;
	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = (WNDPROC)WndProcStatic;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_MAIN);
	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = nullptr;
	wcex.lpszClassName  = szClassName;
	wcex.hIconSm        = nullptr;

	ATOM atom = RegisterClassExW(&wcex);
	if( atom == 0 )
	{
		m_strErr = L"RegisterClassExW failed";
		return false;
	}

	/////////////////////////////////////////////////////////////
	// Create window
	m_alwaysOnTop = alwaysOnTop;

	DWORD nExStyle = 0;
	if( alwaysOnTop )
	{
		nExStyle |= WS_EX_TOPMOST;
	}
	DWORD nStyle = WS_OVERLAPPEDWINDOW;

	if( !CreateEx(nExStyle, szClassName, title.GetBuffer(), nStyle, rcWnd, nullptr) )
	{
		m_strErr = L"CreateEx failed";
		return false;
	}

	Rect rect = GetClientRect();
	const int clientWidth = rect.Width();
	const int clientHeight = rect.Height();

	m_tracectl.SetApplication(m_pApp);
	bool bOk = m_tracectl.Create(0, 0, clientWidth, clientHeight, m_handle, 1200, welcomeMsg);
	if( !bOk )
	{
		m_strErr = L"Cannot create trace control";
		return false;
	}
	AllowFileDropping(true);
	return true;
}

String MainWnd::GetLastError() const
{
	return m_strErr;
}

void MainWnd::AllowFileDropping(bool bAllow)
{
	if( !m_handle )
		return;

	BOOL b2 = bAllow ? TRUE : FALSE;
	DragAcceptFiles(m_handle, b2);
}

void MainWnd::DoPngOptions()
{
	PngOptionsDlg dlg;
	dlg.m_settings = m_pApp->m_engine.m_settings;
	if( dlg.DoModal(this) == DialogResp::Cancel )
		return;

	m_pApp->m_engine.m_settings = dlg.m_settings;
}

void MainWnd::DoScreenshotsOptions()
{
	ScreenshotsOptionsDlg dlg;
	dlg.m_settings = m_pApp->m_bmpcd.m_settings;
	DialogResp ret = dlg.DoModal(this);
	if( ret == DialogResp::Cancel )
		return;

	m_pApp->m_bmpcd.m_settings = dlg.m_settings;
}

void MainWnd::DoAbout()
{
	AboutDlg dlg;
	dlg.DoModal(this);
}

void MainWnd::OnWmMouseWheel(WPARAM wParam, LPARAM lParam)
{
	::SendMessage(m_tracectl.GetHandle(), WM_MOUSEWHEEL, wParam, lParam);
}

void MainWnd::ResizeTraceCtl(int cx, int cy)
{
	m_tracectl.Resize(cx, cy);
}

void MainWnd::SetTraceCtlRedraw(bool bRedraw)
{
	const int nWParam = bRedraw ? TRUE : FALSE;
	::SendMessage(m_tracectl.GetHandle(), WM_SETREDRAW, nWParam, 0);
}

void MainWnd::ClearLine()
{
	m_tracectl.ClearLine();
}

void MainWnd::Clear()
{
	m_tracectl.Clear();

	ListCleared.Fire();
}

void MainWnd::AddLink(const String& strFileNameFinal, const String& strTotalFinalPath)
{
	m_tracectl.AddLink(strFileNameFinal, strTotalFinalPath);
}

void MainWnd::Write(const String& strText, Color cr)
{
	m_tracectl.AddText(strText, cr);
}

void MainWnd::Write(const String& strText, Color cr, int minWidthEx, int justify)
{
	TraceCtl::TextPiece tp;
	tp.text = strText;
	tp.color = cr;
	tp.minWidthEx = (int16)minWidthEx;
	tp.justify = (int16)justify;
	m_tracectl.AddText(tp);
}


void MainWnd::WriteLine(const String& strText, Color cr)
{
	m_tracectl.AddLine(strText, cr);
}

static String GetDraggedFileName(HDROP hDrop, int nFile)
{
	String strFileName;

	UINT nLengthNeeded = DragQueryFileW(hDrop, nFile, nullptr, 0);
	UINT nRet = DragQueryFileW(hDrop, nFile, strFileName.GetUnsafeBuffer(nLengthNeeded), nLengthNeeded + 1);
	nRet;

	return strFileName;
}

void MainWnd::OnWmDropFiles(WPARAM wParam, LPARAM)
{
	HDROP hDrop = (HDROP) wParam;
	int fileCount = DragQueryFileW(hDrop, MAX_UINT, nullptr, 0);

	m_droppedFilePaths.SetSize(0);
    for(int i = 0; i < fileCount; ++i)
	{
		m_droppedFilePaths.Add( GetDraggedFileName(hDrop, i));
	}

	DragFinish(hDrop);

	FilesDropped.Fire(m_droppedFilePaths);
}

bool MainWnd::IsTraceCtlHwnd(HWND hwndCompare)
{
	return hwndCompare == m_tracectl.GetHandle();
}

/////////////////////////////////////////////////////////////
LRESULT MainWnd::WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	int cx, cy;

	switch(nMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint(m_handle, &ps);
		EndPaint(m_handle, &ps);
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_SETFOCUS:
		m_tracectl.SetFocus();
		break;

	case WM_SIZE:
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);

		ResizeTraceCtl(cx, cy);
		break;

	case WM_DROPFILES:
		OnWmDropFiles(wParam, lParam);
		break;

	case WM_NCACTIVATE:
		if( LOWORD(wParam) == WA_ACTIVE )
		{
			Activated.Fire();
		}
		return DefWindowProc(m_handle, nMsg, wParam, lParam);

	case WM_NOTIFY:
		{
			NMHDR* pNM = (NMHDR*) lParam;
			if( IsTraceCtlHwnd(pNM->hwndFrom) )
			{
				POINT point;
				GetCursorPos(&point);

				HINSTANCE hInstance = m_pApp->m_hInstance;
				HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU));
				HMENU hSubMenu = GetSubMenu(hMenu, 0);

				HandleMenuItemStates(hSubMenu);

				DWORD checkAlwaysOnTop = MF_BYCOMMAND;
				checkAlwaysOnTop |= m_alwaysOnTop ? MF_CHECKED : MF_UNCHECKED;
				CheckMenuItem(hSubMenu, IDM_ALWAYSONTOP, checkAlwaysOnTop);

				TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_handle, nullptr);
			}
		}
		break;

	case WM_COMMAND:
		{
			int nId = LOWORD(wParam);
			int nEvent = HIWORD(wParam);
			nEvent;

			if( nId == IDM_PNGOPTIONS )
			{
				DoPngOptions();
				return 0;
			}
			else if( nId == IDM_SCREENSHOTSOPTIONS )
			{
				DoScreenshotsOptions();
				return 0;
			}
			else if( nId == IDM_SHOWSCREENSHOTSDIRECTORY )
			{
				String strDir = m_pApp->m_bmpcd.GetDir();

				// The / is a valid path, but the Shell does not like it, so we convert it
				// into something it likes, "D:\" "E:\" etc.
				strDir = File::GetAbsolutePath(strDir);
				::ShellExecuteW(m_handle, L"open", strDir.GetBuffer(), L"", L"", SW_SHOWNORMAL);
				return 0;
			}
			else if( nId == IDM_ALWAYSONTOP )
			{
				SetAlwaysOnTop(!m_alwaysOnTop);
				return 0;
			}
			else if( nId == IDM_CLEARLIST )
			{
				// We are allowed to clear the tracectl while the thread is working
				Clear();
				return 0;
			}
			else if( nId == IDM_ABOUT )
			{
				DoAbout();
				return 0;
			}
			else if( !m_pApp->IsJobRunning() )
			{
				switch(nId)
				{
				case IDM_PASTEFROMCLIPBOARD:
					m_pApp->DumpScreenshot();
					return 0;
				default:
					break;
				}
			}
		}
		break;

	case WM_KEYDOWN:
	case WM_MOUSEWHEEL:
		OnWmMouseWheel(wParam, lParam);
		return 0;

	case WM_DESTROY:
		{
			Destroying.Fire();
		}
		break;

	case WM_APP_THREADJOBDONE:
		{
			// The working thread notified us that its job is done
			m_pApp->OnJobDone();
		}
		break;

	default:
		return DefWindowProc(m_handle, nMsg, wParam, lParam);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////
void MainWnd::HandleMenuItemStates(HMENU hSubMenu)
{
	int nEnable = MF_GRAYED;
	if( !m_pApp->IsJobRunning() )
	{
		nEnable = MF_ENABLED;
	}
	EnableMenuItem(hSubMenu, IDM_PNGOPTIONS, nEnable);

	nEnable = MF_GRAYED;

	if( !m_pApp->IsJobRunning() )
	{
		if( m_pApp->IsBmpAvailable() )
		{
			nEnable = MF_ENABLED;
		}
	}
	EnableMenuItem(hSubMenu, IDM_PASTEFROMCLIPBOARD, nEnable);
}

///////////////////////////////////////////////////////////////////
void MainWnd::SetAlwaysOnTop(bool alwaysOnTop)
{
	m_alwaysOnTop = alwaysOnTop;
	HWND hWndAfter = alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST;
	SetWindowPos(m_handle, hWndAfter, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

///////////////////////////////////////////////////////////////////
// Gets the AlwaysOnTop option regardless of the current top state of the window
// (the top state window could have been changed by an external program)
bool MainWnd::GetAlwaysOnTop() const
{
	return m_alwaysOnTop;
}
