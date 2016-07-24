///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_THREAD_H
#define CHUSTD_THREAD_H

namespace chustd {

class Thread
{
public:
	bool Start(int (*pfn)(void*), void* userArg);

	bool IsStarted() const;
	void Suspend();

	void WaitForExit();

	Thread();
	~Thread();

	static void Sleep(int delayMs);

private:
	void* m_handle;
};

} // namespace chustd

#endif
