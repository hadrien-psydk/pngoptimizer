///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CriticalSection.h"

using namespace chustd;

//////////////////////////////////////////////////////////////////////
CriticalSection::CriticalSection()
{
#ifdef _WIN32
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*) m_impl;
	InitializeCriticalSection(pCS);
#endif
}

CriticalSection::~CriticalSection()
{
#ifdef _WIN32
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*) m_impl;
	DeleteCriticalSection(pCS);
#endif
}

void CriticalSection::Enter()
{
#ifdef _WIN32
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*) m_impl;
	EnterCriticalSection(pCS);
#endif
}

void CriticalSection::Leave()
{
#ifdef _WIN32
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*) m_impl;
	LeaveCriticalSection(pCS);
#endif
}
