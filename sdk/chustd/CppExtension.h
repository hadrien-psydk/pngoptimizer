///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_CPPEXTENSION_H
#define CHUSTD_CPPEXTENSION_H

////////////////////////////////
// Set here the byte order of your platform (x86)
namespace chustd {

	enum ByteOrder
	{
		boLittleEndian, // x86
		boBigEndian     // PowerPC
	};
	
	const ByteOrder k_ePlatformByteOrder = boLittleEndian;

} // namespace chustd;

#include "ChuTypes.h"

///////////////////////////////////////////////////////////////////////////
// For arrays ans lists, this macro replaces
//	int count = m_aSections.GetSize(); 
//	for(int i = 0; i < count; ++i ) 
// with : 
//  foreach(m_aSections, i) 
#define foreach(varArray, varStep) for(int32 varStep = 0, count = varArray.GetSize(); varStep < count ; ++varStep) 
///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// Debug helper functions
// This is a very implementation specific part
// Compiles with MS Visual C++ 6.0
// Compiles with G++ (GCC version 3.2)

#if _MSC_VER >= 1400
#define INTERRUPT_EXECUTION() __debugbreak(); // VC8 generic break, works with x86, x64...
#elif _MSC_VER >= 1100
#define INTERRUPT_EXECUTION() __asm { int 3 }     // Works with x86
#else
#define INTERRUPT_EXECUTION() ;
#endif

//////////////////////////////////
#ifndef ASSERT
	#ifdef NDEBUG
		#define ASSERT(exp) ((void)0);
	#else
		#define ASSERT(exp) if(!(exp)) { static bool bOnce = false; if( !bOnce) { bOnce = true;\
			ChustdAssertMessage(__FILE__, __LINE__, 0);\
			INTERRUPT_EXECUTION() } };
	#endif
#endif

//////////////////////////////////
#ifndef VERIFY
	#define VERIFY(exp) if(!(exp)) { static bool bOnce = false; if( !bOnce) { bOnce = true;\
			ChustdAssertMessage(__FILE__, __LINE__, 0);\
			INTERRUPT_EXECUTION() } };
#endif

//////////////////////////////////
#ifndef ASSERTMSG
	#ifdef NDEBUG
		#define ASSERTMSG(exp, str) ((void)0);
	#else
#define ASSERTMSG(exp, psz) if(!(exp)) { static bool bOnce = false; if( !bOnce) { bOnce = true; \
			ChustdAssertMessage(__FILE__, __LINE__, psz);\
			INTERRUPT_EXECUTION() } };
	#endif
#endif

//////////////////////////////////
#ifndef VERIFYMSG
#define VERIFYMSG(exp, psz) if(!(exp)) { static bool bOnce = false; if( !bOnce) { bOnce = true; \
			ChustdAssertMessage(__FILE__, __LINE__, psz);\
			INTERRUPT_EXECUTION() } };
#endif

#if defined(_MSC_VER)
// Used by the Color class, recode Color if portability problems
#pragma warning (disable: 4201) // nonstandard extension used : nameless struct/union
// Use enum underlying type C++0x feature
#pragma warning (disable: 4480) // nonstandard extension used: specifying underlying type for enum
#endif

// Writes traces in an output window if your debugger allows it
void ChustdDebugMessage(const char* pszMsg);

// Warns the user of an assertion failure
void ChustdAssertMessage(const char* pszFileName, int32 line, const char* pszMsg);

// Prints the allocated memory blocks
void ChustdDebugPrintLeak(void* p, int size, const char* pszFileName, int32 line);

// Traces a string to a UTF-16 file called chustd-trace.txt
namespace chustd { class String; };
void ChustdTraceLine(const class chustd::String& str);

//#define double please_use_float64_instead_of_double_alone
//#define float please_use_float32_instead_of_float_alone

#define BYTE_FROM_BITS(a7,a6,a5,a4,a3,a2,a1,a0) ((a7<<7)|(a6<<6)|(a5<<5)|(a4<<4)|(a3<<3)|(a2<<2)|(a1<<1)|a0)
#define BYTE_FROM_BITS2(a6,a4,a2,a0) ((a6<<6)|(a4<<4)|(a2<<2)|a0)
#define ROUND32(arg) ((arg + 3) & (~3))
#define ROUND64(arg) ((arg + 7) & (~7))

template<typename T>
inline T MIN(T a, T b) { return (a < b) ? a : b; }

template<typename T>
inline T MAX(T a, T b) { return (a > b) ? a : b; }

template<typename T, int TSIZE>
uint8 (*ArraySizeHelper(T (&)[TSIZE]))[TSIZE];
#define ARRAY_SIZE(arr) int(sizeof(*ArraySizeHelper(arr)))

#endif // ndef CHUSTD_CPPEXTENSION_H
