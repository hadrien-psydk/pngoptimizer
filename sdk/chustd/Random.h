///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_RANDOM_H
#define CHUSTD_RANDOM_H

namespace chustd {

class Random  
{
public:
	int32 GetNext(int32 min, int32 max);

	Random();
	~Random();

private:
	int32 m_nModulus;
	int32 m_nMultiplier;
	int32 m_nNumber;
};

} // namespace chustd

#endif // ndef CHUSTD_RANDOM_H
