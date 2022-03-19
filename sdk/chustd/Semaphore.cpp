///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Semaphore.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
template<int TSIZE>
static HANDLE ToHandle(void* (&impl)[TSIZE]) { return *(reinterpret_cast<HANDLE*>(impl)); }

template<int TSIZE>
static void SetHandle(void* (&impl)[TSIZE], HANDLE h) { impl[0] = h; }

#elif defined(__linux__)
template<int TSIZE>
static sem_t* ToSem(void* (&impl)[TSIZE])
{
	static_assert(sizeof(sem_t) <= sizeof(impl), "m_impl not big enough");
	return reinterpret_cast<sem_t*>(impl);
}

#endif

///////////////////////////////////////////////////////////////////////////////
Semaphore::Semaphore()
{
	memset(m_impl, 0, sizeof(m_impl));
}

///////////////////////////////////////////////////////////////////////////////
Semaphore::~Semaphore()
{
	Close();
}

///////////////////////////////////////////////////////////////////////////////
bool Semaphore::Create(int initialCount)
{
#if defined(_WIN32)
	HANDLE handle = CreateSemaphore(nullptr, initialCount, MAX_INT32, nullptr);
	if( handle == nullptr)
	{
		return false;
	}
	SetHandle(m_impl, handle);
	return true;
#elif defined(__linux__)
	return sem_init(ToSem(m_impl), 0, initialCount) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
void Semaphore::Close()
{
	static void* const empty[ARRAY_SIZE(m_impl)] = {};
	if( memcmp(m_impl, empty, sizeof(empty)) == 0 )
	{
		// Not created
		return;
	}

#if defined(_WIN32)
	CloseHandle(ToHandle(m_impl));
#elif defined(__linux__)
	sem_destroy(ToSem(m_impl));
#endif
	memset(m_impl, 0, sizeof(m_impl));
}

///////////////////////////////////////////////////////////////////////////////
bool Semaphore::Increment()
{
#if defined(_WIN32)
	LONG previousCount = 0;
	return ReleaseSemaphore(ToHandle(m_impl), 1, &previousCount) != 0;
#elif defined(__linux__)
	return sem_post(ToSem(m_impl)) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Returns 0 upon signal
int Semaphore::Wait(int timeout)
{
#if defined(_WIN32)
	DWORD wret = WaitForSingleObject(ToHandle(m_impl), timeout);
	if( wret == WAIT_OBJECT_0 )
	{
		return 0;
	}
	else if( wret == WAIT_TIMEOUT )
	{
		return -1;
	}
	else
	{
		return -2;
	}
#elif defined(__linux__)
	// TODO: use timeout
	(void)timeout;
	int ret = sem_wait(ToSem(m_impl));
	if( ret == 0 )
	{
		return 0;
	}
	return -1;
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
