///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_SEMAPHORE_H
#define CHUSTD_SEMAPHORE_H

namespace chustd {

class Semaphore
{
public:
	Semaphore();
	~Semaphore();

	bool Create(int initialCount = 0);
	void Close();

	bool Increment();
	int  Wait(int timeout = -1);

private:
	void* m_impl[5];
};

} // namespace chustd

#endif
