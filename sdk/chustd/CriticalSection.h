///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_CRITICALSECTION_H
#define CHUSTD_CRITICALSECTION_H

namespace chustd {

class CriticalSection  
{
public:
	CriticalSection();
	~CriticalSection();

	void Enter();
	void Leave();

private:
	int64 m_impl[6]; // Opaque
};


class TmpLock
{
public:
	TmpLock(CriticalSection& cs) : m_cs(cs) { m_cs.Enter(); }
	~TmpLock() { m_cs.Leave(); }

private:
	CriticalSection& m_cs;
};

} // namespace chustd

#endif // ndef CHUSTD_CRITICALSECTION_H
