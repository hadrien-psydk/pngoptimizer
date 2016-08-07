///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Thread.h"

namespace chustd {

///////////////////////////////////////////////////////////////////////////////
Thread::Thread()
{
	m_handle = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
Thread::~Thread()
{
#if defined(_WIN32)
	if( m_handle )
	{
		::CloseHandle(m_handle);
	}
#endif
}

////////////////////////////////////////////////
#if defined(_WIN32)
struct StartArg
{
	int (*pfn)(void*);
	void* userArg;
};

static DWORD WINAPI ThreadProc(LPVOID arg)
{
	StartArg* pStartArg = reinterpret_cast<StartArg*>(arg);
	StartArg startArg = *pStartArg;
	delete pStartArg;
	int ret = startArg.pfn(startArg.userArg);
	return static_cast<DWORD>(ret);
}
#endif

typedef void* (*pthread_pfn)(void*);

///////////////////////////////////////////////////////////////////////////////
bool Thread::Start(int (*pfn)(void*), void* userArg)
{
#if defined(_WIN32)
	StartArg* pStartArg = new StartArg;
	pStartArg->pfn = pfn;
	pStartArg->userArg = userArg;

	DWORD threadId = 0;
	m_handle = ::CreateThread(nullptr, 0, &ThreadProc, pStartArg, 0, &threadId);
	return m_handle != nullptr;

#elif defined(__linux__)
	pthread_t th;
	int ret = pthread_create(&th, nullptr, (pthread_pfn)pfn, userArg);
	if( ret != 0 )
	{
		return false;
	}
	m_handle = reinterpret_cast<void*>(th);
	return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool Thread::IsStarted() const
{
	return m_handle != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
void Thread::Suspend()
{
#if defined(_WIN32)
	::SuspendThread(m_handle);
#endif
}

///////////////////////////////////////////////////////////////////////////////
void Thread::WaitForExit()
{
	if( m_handle == nullptr )
	{
		return;
	}

#if defined(_WIN32)
	WaitForSingleObject(m_handle, INFINITE);
	CloseHandle(m_handle);

#elif defined(__linux__)
	void* threadRet = nullptr;
	pthread_t th = reinterpret_cast<pthread_t>(m_handle);
	pthread_join(th, &threadRet);
#endif

	m_handle = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
void Thread::Sleep(int delayMs)
{
#if defined(_WIN32)
	::Sleep(delayMs);

#elif defined(__linux__)
	struct timespec ts;
	ts.tv_sec = delayMs / 1000;
	ts.tv_nsec = (delayMs % 1000) * 1000000;
	nanosleep(&ts, NULL);
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
