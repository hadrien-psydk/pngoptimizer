///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_ATOMIC_H
#define CHUSTD_ATOMIC_H

namespace chustd {\

class Atomic
{
public:
	static int32 Increment(int32* pVal);
	static int32 Decrement(int32* pVal);
};

} // namespace chustd

#endif
