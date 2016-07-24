///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_STDAFX_H
#define CHUWIN32_STDAFX_H

#include "../chustd/chustd.h"

#ifndef STRICT
#define STRICT
#endif
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#ifndef UNICODE
#define UNICODE 
#endif

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <mmsystem.h>

#include <shlobj.h>
#include <ole2.h>
#pragma comment(lib, "ole32.lib")

#include <shellapi.h>
#pragma comment(lib, "comctl32.lib")

#include <Commdlg.h>

#if !defined(WM_MOUSEWHEEL)
#define WM_MOUSEWHEEL 0x020A
#endif

#if !defined(GET_WHEEL_DELTA_WPARAM)
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam)) 
#endif

// We define new Windows SDK functions to stay compatible with VC6 SDK with Win32 builds
#ifndef SetWindowLongPtr
#ifdef _WIN64
#error Your Windows SDK is too old
#else
#define SetWindowLongPtr   SetWindowLongW
#define GetWindowLongPtr   GetWindowLongW
#define DWLP_USER     DWL_USER
#define GWLP_USERDATA GWL_USERDATA
#define LongToPtr(ul) (void*)(ul)
#define LongToHandle(ul) HANDLE(ul)
#endif
#endif

#endif // ndef CHUWIN32_STDAFX_H
