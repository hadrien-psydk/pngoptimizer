///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_FIXARRAY_H
#define CHUSTD_FIXARRAY_H

namespace chustd {

// Manages a fixed sized array of elements
template <class T, int32 t_nCount>
class FixArray
{
public:
	// Gets an element at a specific position
	inline T& operator [] (int index)
	{
		ASSERTMSG(index >= 0 && index < t_nCount, "FixArray - Bad index");
		return m_aTs[index];
	}

	// Gets an element at a specific position
	inline const T& operator [] (int index) const
	{
		ASSERTMSG(index >= 0 && index < t_nCount, "CFixArray - Bad index");
		return m_aTs[index];
	}

	inline int32 GetSize() const
	{
		return t_nCount;
	}

	inline T* GetUnsafePtr()
	{
		return &m_aTs[0];
	}

	inline const T* GetPtr() const
	{
		return &m_aTs[0];
	}

	inline int32 Find(const T& t) const
	{
		for(int i = 0; i < t_nCount; ++i)
		{
			if( m_aTs[i] == t )
			{
				// Found !
				return i;
			}
		}
		return -1;
	}

	template <class T2>
	inline int32 FindPtr(const T2* pT) const
	{
		for(int i = 0; i < t_nCount; ++i)
		{
			if( (m_aTs + i) == pT )
			{
				// Found !
				return i;
			}
		}
		return -1;
	}

	// Initializes all element with a value
	void Set(const T& t)
	{
		for(int i = 0; i < t_nCount; i++)
		{
			m_aTs[i] = t;
		}
	}

// Public instead of private in order to allow static initialisation, but DO NOT USE DIRECTLY
public:
	T m_aTs[t_nCount];
};

} // namespace chustd

#endif // ndef CHUSTD_FIXARRAY_H
