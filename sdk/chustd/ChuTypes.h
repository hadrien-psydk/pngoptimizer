///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_CHUTYPES_H
#define CHUSTD_CHUTYPES_H

////////////////////////////////
// Basic types definition

#if _MSC_VER >= 1100
// VC++ : we use those cool MS specific types defined by the compiler
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

#ifdef _WCHAR_T_DEFINED
typedef wchar_t wchar; // Use native VC8 wchar_t type
#else
typedef unsigned short wchar; // VC6 & VC7 & VC8 C mode
#endif

typedef float float32;
typedef double float64;

#else
// We do what we can with standard C++ types. Check out the real size defined
// by your compiler!
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef char16_t wchar;

typedef float float32;
typedef double float64;

#endif // _MSC_VER >= 1100 // 1200 Visual C++ 6.0

#define MAX_INT8   0x7f
#define MIN_INT8   0x80
#define MAX_UINT8  0xffu

#define MAX_INT16  0x7fff
#define MIN_INT16  0x8000
#define MAX_UINT16 0xffffu

#define MAX_INT32  0x7fffffff
#define MIN_INT32  0x80000000
#define MAX_UINT32 0xffffffffu

#define MAX_INT64  0x7fffffffffffffff
#define MIN_INT64  0x8000000000000000
#define MAX_UINT64 0xffffffffffffffffu

#define MAX_UINT (~0u)

#define MAX_FLOAT32 3.402823466e+38F
#define MIN_FLOAT32 -3.402823466e+38F

#define MAX_FLOAT64 1.7976931348623158e+308
#define MIN_FLOAT64 -1.7976931348623158e+308

#define MAX_DOUBLE MAX_FLOAT64

////////////////////////////////
// Little trick to overpass Visual C++ non conformity with the standard
// regarding variables inside for loops
#if _MSC_VER <= 1200
#define for if(0); else for
#endif // _MSC_VER <= 1200

////////////////////////////////
#ifdef _MSC_VER
#pragma warning( disable : 4097 ) // Level4 - typedef-name 'parent' used as synonym for class-name
#pragma warning( disable : 4127 ) // Level4 - conditional expression is constant
#pragma warning( disable : 4710 ) // Level4 - function xxx not inlined
#pragma warning( disable : 4711 ) // Level4 - function xxx selected for automatic inline expansion
#pragma warning( disable : 4514 ) // Level4 - unreferenced inline function has been removed
#pragma warning( disable : 4511 ) // Level4 - copy constructor could not be generated
#pragma warning( disable : 4512 ) // Level4 - assignment operator could not be generated
#endif

#define MAKE16(u0,u1)       ((u0 << 8) | u1)
#define MAKE32(u0,u1,u2,u3) ((u0 << 24) | (u1 << 16) | (u2 << 8) | u3)

#ifdef _MSC_VER
#define UTF16(s) L##s
#else
#define UTF16(s) u##s
#endif

#endif // ndef CHUSTD_CHUTYPES_H

