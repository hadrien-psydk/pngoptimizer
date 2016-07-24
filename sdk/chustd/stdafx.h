///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

// This is the common include file for the chustd library, it can be used
// as a precompiled header for compilers which support this feature.

// This header is private and contains OS dependent includes

#ifndef CHUSTD_STDAFX_H
#define CHUSTD_STDAFX_H

////////////////////////////////
#include <math.h> // cos, sin, tan, pow
#include <new> // Placement new
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
#if defined(_WIN32)

// Windows

//////////////////////////////////////////////////////////////////////
// Avoid calls to undefined function

#define _INC_STRING // string.h declares mem* functions, we do not want them
#define _OLE32PRIV_
#define _OLE32_
#define _SYS_GUID_OPERATORS_
#define __STRALIGN_H_ // Unaligned wide interface (inline call to wcscpy)

#include <memory.h>
//////////////////////////////////////////////////////////////////////

#ifndef STRICT
#define STRICT 1
#endif

#ifndef UNICODE
#define UNICODE
#endif

#define _WIN32_WINNT 0x501  // Windows XP and superior
#define WIN32_LEAN_AND_MEAN

// Sockets
#define int32 int // int32 redefined in VC++5 include files
#include <winsock2.h> // Includes <windows.h>
#undef int32
typedef int socklen_t;    // "int" on Windows/VC++, unsigned int on Linux/G++
typedef SOCKET SOCKET_ID; // HANDLE on Windows/VC++, int on Linux/G++

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

#include <windowsx.h>

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam)) 
#endif

#include <shlobj.h>

// Auto-link
// Does not seem to work, does anybody know why ?
// You may add this lib in your project settings to force linking

// Regular libs : kernel32.lib user32.lib gdi32

// For drag-and-drop
#pragma comment(lib, "shell32.lib")

// For sockets
#pragma comment(lib, "ws2_32.lib")

#define Rectangle chustd_Rectangle

#undef MOD

// Missing definition for Win32/SetFilePointer
#ifndef INVALID_SET_FILE_POINTER 
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

#undef GetFirstChild // Otherwise, conflict with XmlDocument

// In winbase.h #define GetCurrentTime == GetTickCount()
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

/////////////////////////////////////////////////////////////////////
#elif defined(__linux__)

#define __declspec(x) __attribute__((x))
#define __cdecl   __attribute__((cdecl))
#define __stdcall __attribute__((stdcall))

#include <wchar.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> // for close()
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h> // malloc
#include <malloc.h> // malloc_useable_size
#include <sys/stat.h> // fstat
#include <semaphore.h>
#include <memory.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <dirent.h> 
#include <sys/time.h>

const int INVALID_SOCKET = -1;
typedef int SOCKET_ID;

#else
#error oops
#endif
//////////////////////////////////////////////////////////////////////

// Common header not OS dependant
#include "CppExtension.h"

#endif
