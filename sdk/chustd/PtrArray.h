///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_PTRARRAY_H
#define CHUSTD_PTRARRAY_H

#include "Array.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
// Simple array of pointers
template <typename T>
class PtrArrayNoDelete : public Array<void*>
{
public:
	T*& operator [] (int index)
	{
		return (T*&) (m_paTs[index]);
	}

	T* operator [] (int index) const
	{
		return (T*) (m_paTs[index]);
	}
};

///////////////////////////////////////////////////////////////////////////////
// Manages a pointers array
// The array owns the data, so it deletes the pointed data
template <typename T>
class PtrArray : public Array<T*>
{
public:
	PtrArray();
	PtrArray(const PtrArray<T>& arr);
	PtrArray(PtrArray<T>&& arr);
	virtual ~PtrArray();

	int Add(T* pT)
	{
		// Every address must be unique
		if(pT)
		{
			ASSERT( Array<T*>::Find(pT) < 0);
		}

		return Array<T*>::Add(pT);
	}

	int Add(T* const * papTs, int count) {	return Array<T*>::Add(papTs, count); }
	int Add(const PtrArray<T>& arr) { return Array<T*>::Add(arr); }

	PtrArray<T>& operator = (const PtrArray<T>& arr);

protected:
	virtual void DeletePointedData(int startIndex, int count);
};

///////////////////////////////////////////////////////////////////////////////
template <class T>
PtrArray<T>::PtrArray()
{
}

///////////////////////////////////////////////////////////////////////////////
// Copy constructor
template <class T>
PtrArray<T>::PtrArray(const PtrArray<T>& arr)
{
	this->m_paTs = nullptr;
	this->m_count = 0;
	this->m_capacity = 0;

	Add(arr.m_paTs, arr.m_count);

	// Create copies of pointed data
	for(int i = 0; i < this->m_count; i++)
	{
		this->m_paTs[i] = new T( *this->m_paTs[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Move constructor
template <class T>
PtrArray<T>::PtrArray(PtrArray<T>&& arr)
{
	this->m_paTs = arr.m_paTs;
	this->m_count = arr.m_count;
	this->m_capacity = arr.m_capacity;

	arr.m_paTs = nullptr;
	arr.m_count = 0;
	arr.m_capacity = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Assignment operator
template <class T>
PtrArray<T>& PtrArray<T>::operator = (const PtrArray<T>& arr)
{
	Array<T*>::operator = (arr);
	
	// Create copies of pointed data
	for(int i = 0; i < this->m_count; i++)
	{
		this->m_paTs[i] = new T( *this->m_paTs[i]);
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
PtrArray<T>::~PtrArray()
{
	for(int i = 0; i < this->m_count; ++i)
	{
		delete this->m_paTs[i];
	}
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void PtrArray<T>::DeletePointedData(int startIndex, int count)
{
	for(int i = 0; i < count; i++)
	{
		T* pElement = this->m_paTs[startIndex + i];
		delete pElement;
	}
}

///////////////////////////////////////////////////////////////////////////////
} // namespace chustd

#endif // ndef CHUSTD_PTRARRAY_H
