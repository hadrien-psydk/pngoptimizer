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
#elif defined(__linux__)
	static_assert(sizeof(m_impl) >= sizeof(pthread_mutex_t), "increase array");
	auto mutex = reinterpret_cast<pthread_mutex_t*>(m_impl);
	pthread_mutex_init(mutex, nullptr);
#endif
}

CriticalSection::~CriticalSection()
{
#ifdef _WIN32
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*) m_impl;
	DeleteCriticalSection(pCS);
#elif defined(__linux__)
	auto mutex = reinterpret_cast<pthread_mutex_t*>(m_impl);
	pthread_mutex_destroy(mutex);
#endif
}

void CriticalSection::Enter()
{
#ifdef _WIN32
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*) m_impl;
	EnterCriticalSection(pCS);
#elif defined(__linux__)
	auto mutex = reinterpret_cast<pthread_mutex_t*>(m_impl);
	pthread_mutex_lock(mutex);
#endif
}

void CriticalSection::Leave()
{
#ifdef _WIN32
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*) m_impl;
	LeaveCriticalSection(pCS);
#elif defined(__linux__)
	auto mutex = reinterpret_cast<pthread_mutex_t*>(m_impl);
	pthread_mutex_unlock(mutex);
#endif
}
