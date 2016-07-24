/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWnd.h"

#include "resource.h"
#include "PngOptimizer.h"
#include "DlgPngOptions.h"
#include "DlgScreenshotsOptions.h"
#include "DlgAbout.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
MainWnd::MainWnd()
{
	m_alwaysOnTop = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool MainWnd::Create(const String& title, RECT rcWnd, bool alwaysOnTop, const String& welcomeMsg)
{
	HINSTANCE hInstance = POApplication::GetInstance().m_hInstance;

	/////////////////////////////////////////////////////////////
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
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = szClassName;
	wcex.hIconSm        = NULL;

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

	if( !CreateEx(nExStyle, szClassName, title.GetBuffer(), nStyle, rcWnd, NULL) )
	{
		m_strErr = L"CreateEx failed";
		return false;
	}
	
	RECT rect = GetClientRect();
	const int clientWidth = rect.right;
	const int clientHeight = rect.bottom;

	bool bOk = m_tracectl.Create(0, 0, clientWidth, clientHeight, m_hWnd, 1200, welcomeMsg);
	if( !bOk )
	{
		m_strErr = L"Cannot create trace control";
		return false;
	}
	AllowFileDropping(true);
	return true;
}

HWND MainWnd::GetHandle()
{
	return m_hWnd;
}

String MainWnd::GetLastError() const
{
	return m_strErr;
}

void MainWnd::AllowFileDropping(bool bAllow)
{
	if( !m_hWnd )
		return;

	BOOL b2 = bAllow ? TRUE : FALSE;
	DragAcceptFiles(m_hWnd, b2);
}

void MainWnd::DoPngOptions()
{
	DlgPngOptions dlg;

	POApplication& app = POApplication::GetInstance();

	dlg.m_settings = app.m_engine.m_settings;

	int nRet = dlg.DoModal(m_hWnd);
	if( nRet == IDCANCEL )
		return;
	
	app.m_engine.m_settings = dlg.m_settings;
}

void MainWnd::DoScreenshotsOptions()
{
	DlgScreenshotsOptions dlg;

	POApplication& app = POApplication::GetInstance();

	dlg.m_useDefaultDir = app.m_bmpcd.m_settings.useDefaultDir;
	dlg.m_customDir = app.m_bmpcd.m_settings.customDir;
	dlg.m_maximizeCompression = app.m_bmpcd.m_settings.maximizeCompression;
	dlg.m_askForFileName = app.m_bmpcd.m_settings.askForFileName;
	
	int nRet = dlg.DoModal(m_hWnd);
	if( nRet == IDCANCEL )
		return;

	app.m_bmpcd.m_settings.useDefaultDir = dlg.m_useDefaultDir;
	app.m_bmpcd.m_settings.customDir = dlg.m_customDir;
	app.m_bmpcd.m_settings.maximizeCompression = dlg.m_maximizeCompression;
	app.m_bmpcd.m_settings.askForFileName = dlg.m_askForFileName;
}

void MainWnd::DoAbout()
{
	DlgAbout dlg;
	dlg.DoModal(m_hWnd);
}

void MainWnd::OnWmMouseWheel(WPARAM wParam, LPARAM lParam)
{
	::SendMessage(m_tracectl.m_hWnd, WM_MOUSEWHEEL, wParam, lParam);
}

void MainWnd::ResizeTraceCtl(int cx, int cy)
{
	m_tracectl.Resize(cx, cy);
}

void MainWnd::SetTraceCtlRedraw(bool bRedraw)
{
	const int nWParam = bRedraw ? TRUE : FALSE;
	::SendMessage(m_tracectl.m_hWnd, WM_SETREDRAW, nWParam, 0);
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

void MainWnd::Write(const String& strText, COLORREF cr)
{
	m_tracectl.AddText(strText, cr);
}

void MainWnd::Write(const String& strText, COLORREF cr, int minWidthEx, int justify)
{
	TraceCtl::TextPiece tp;
	tp.text = strText;
	tp.color = cr;
	tp.minWidthEx = (int16)minWidthEx;
	tp.justify = (int16)justify;
	m_tracectl.AddText(tp);
}


void MainWnd::WriteLine(const String& strText, COLORREF cr)
{
	m_tracectl.AddLine(strText, cr);
}

String MainWnd::GetDraggedFileName(HDROP hDrop, int nFile)
{
	String strFileName;

	UINT nLengthNeeded = DragQueryFileW(hDrop, nFile, NULL, 0);
	UINT nRet = DragQueryFileW(hDrop, nFile, strFileName.GetUnsafeBuffer(nLengthNeeded), nLengthNeeded + 1);
	nRet;

	return strFileName;
}

void MainWnd::OnWmDropFiles(WPARAM wParam, LPARAM)
{
	HDROP hDrop = (HDROP) wParam;
	int fileCount = DragQueryFileW(hDrop, MAX_UINT, NULL, 0);

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
	return hwndCompare == m_tracectl.m_hWnd;
}

/////////////////////////////////////////////////////////////
LRESULT MainWnd::WndProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	POApplication& app = POApplication::GetInstance();

	PAINTSTRUCT ps;
	HDC hdc;
	int cx, cy;

	switch(nMsg) 
	{
	case WM_PAINT:
		hdc = BeginPaint(m_hWnd, &ps);
		EndPaint(m_hWnd, &ps);
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
		return DefWindowProc(m_hWnd, nMsg, wParam, lParam);

	case WM_NOTIFY:
		{
			NMHDR* pNM = (NMHDR*) lParam;
			if( IsTraceCtlHwnd(pNM->hwndFrom) )
			{
				POINT point;
				GetCursorPos(&point);
				
				HINSTANCE hInstance = POApplication::GetInstance().m_hInstance;
				HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU));
				HMENU hSubMenu = GetSubMenu(hMenu, 0);

				HandleMenuItemStates(hSubMenu);

				DWORD checkAlwaysOnTop = MF_BYCOMMAND;
				checkAlwaysOnTop |= m_alwaysOnTop ? MF_CHECKED : MF_UNCHECKED;
				CheckMenuItem(hSubMenu, IDM_ALWAYSONTOP, checkAlwaysOnTop);

				TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
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
				String strDir = app.m_bmpcd.GetDir();
				
				// The / is a valid path, but the Shell does not like it, so we convert it
				// into something it likes, "D:\" "E:\" etc.
				strDir = File::GetAbsolutePath(strDir);
				::ShellExecuteW(m_hWnd, L"open", strDir.GetBuffer(), L"", L"", SW_SHOWNORMAL);
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
			else if( !app.IsJobRunning() )
			{
				switch(nId)
				{
				case IDM_PASTEFROMCLIPBOARD:
					app.DumpScreenshot();
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
			POApplication::GetInstance().OnJobDone();
		}
		break;

	default:
		return DefWindowProc(m_hWnd, nMsg, wParam, lParam);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////
void MainWnd::HandleMenuItemStates(HMENU hSubMenu)
{
	POApplication& app = POApplication::GetInstance();

	int nEnable = MF_GRAYED;
	if( !app.IsJobRunning() )
	{
		nEnable = MF_ENABLED;
	}
	EnableMenuItem(hSubMenu, IDM_PNGOPTIONS, nEnable);

	nEnable = MF_GRAYED;

	if( !app.IsJobRunning() )
	{
		if( app.IsBmpAvailable() )
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
	SetWindowPos(m_hWnd, hWndAfter, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

///////////////////////////////////////////////////////////////////
// Gets the AlwaysOnTop option regardless of the current top state of the window
// (the top state window could have been changed by an external program)
bool MainWnd::GetAlwaysOnTop() const
{
	return m_alwaysOnTop;
}
