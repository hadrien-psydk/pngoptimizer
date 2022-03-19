///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Math.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;
//////////////////////////////////////////////////////////////////////
const float64 Math::fPiDiv2_64 = 0.5 * 3.1415926535897932384626433832795029;
const float64 Math::fPi_64     = 3.1415926535897932384626433832795029;
const float64 Math::f2Pi_64    = 2 * 3.1415926535897932384626433832795029;

const float32 Math::fPiDiv2_32 = 0.5 * 3.1415926535897932384626433832795029f;
const float32 Math::fPi_32     = 3.1415926535897932384626433832795029f;
const float32 Math::f2Pi_32    = 2 * 3.1415926535897932384626433832795029f;

const float64 Math::fSqrt2_64     = 1.4142135623730950488016887242097;
const float64 Math::fSqrt3_64     = 1.7320508075688772935274463415059;
const float64 Math::fSqrt3Div2_64 = 0.8660254037844386;

const float32 Math::fSqrt2_32     = 1.4142135623730950488016887242097f;
const float32 Math::fSqrt3_32     = 1.7320508075688772935274463415059f;
const float32 Math::fSqrt3Div2_32 = 0.8660254037844386f;

#define ISQRT_STEP(k) s = t + r; r >>= 1; if (s <= v) { v -= s; r |= t;}
uint32 Math::IntegerSqrt(uint32 v)
{
    uint32 t = 1L << 30;
	uint32 r = 0;
	uint32 s;

    ISQRT_STEP(15);  t >>= 2;
    ISQRT_STEP(14);  t >>= 2;
    ISQRT_STEP(13);  t >>= 2;
    ISQRT_STEP(12);  t >>= 2;
    ISQRT_STEP(11);  t >>= 2;
    ISQRT_STEP(10);  t >>= 2;
    ISQRT_STEP(9);   t >>= 2;
    ISQRT_STEP(8);   t >>= 2;
    ISQRT_STEP(7);   t >>= 2;
    ISQRT_STEP(6);   t >>= 2;
    ISQRT_STEP(5);   t >>= 2;
    ISQRT_STEP(4);   t >>= 2;
    ISQRT_STEP(3);   t >>= 2;
    ISQRT_STEP(2);   t >>= 2;
    ISQRT_STEP(1);   t >>= 2;
    ISQRT_STEP(0);

    return r;
}

uint32 Math::Log2(uint32 val)
{
	int32 mask = 0x80000000; // Signed so the bit sign bleeds on the right
	uint32 i = 0;
	for(;i < 32; ++i)
	{
		if( (val & mask) != 0 )
			break;
		
		mask >>= 1;
	}
	uint32 log = 31 - i;
	log &= ~mask;
	return log;
}

extern "C"
{
#if defined(_DEBUG) && defined(_M_X86)
// sin becomes intrinsic in Release mode
__declspec(naked)
double  __cdecl sin(double)
{
	__asm
	{
        push  ebp
        mov   ebp, esp
        fld   qword ptr [ebp + 8]
        fsin
        pop   ebp
        ret
	}
}
#endif

} // extern "C"