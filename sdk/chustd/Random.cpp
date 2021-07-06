///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Random.h"
#include "System.h"

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
using namespace chustd;
//////////////////////////////////////////////////////////////////////

Random::Random()
{
	m_nModulus	= 2147483647;	// 2^31 - 1
	m_nMultiplier = 16807;		// 7^5
	m_nNumber = System::GetTime();
}

Random::~Random()
{

}

// Not a very good algorithm, but enough for now
int32 Random::GetNext(int32 min, int32 max)
{
	if( !(max > min) )
	{
		return 0;	
	}

	int64 tmp = m_nNumber;
	tmp *= m_nMultiplier;
	tmp = tmp % m_nModulus;

	tmp &= 0x7fffffff;
	m_nNumber = uint32(tmp);

	int64 result = m_nNumber;
	int32 range = (max - min) + 1;
	result *= range;
	result >>= 31;

	int32 rand = min + int32(result);

	return rand;
}