///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_ARRAY_H
#define CHUSTD_ARRAY_H

#include "stdafx.h"
#include "Memory.h"
#include "String.h"

// The "new" operator has been redefined in CppExtension.h with CHUDEBUG_NEW
// We undef the "new" preprocessor macro so we can use the real new to
// perform "placement new"s. The macro is defined again at the end of this file
#ifdef CHUDEBUG_NEW
#define ARRAY_OLDNEW 1
#undef new
#endif

namespace chustd {\

// Array: manages a linear dynamic array of elements:
// classes, structures, atomic types
// Warning:
// - stored classes must have a default constructor
// - do not use pointer or references on element of the array if you plan
// to resize the array
// If you want to keep pointers valid, you may use PtrArray instead
template <typename T>
class Array
{
public:
	Array();
	Array(int count);
	Array(const Array<T>& arr);
	Array(Array<T>&& arr);
	Array(const T* paTs, int count);
	virtual ~Array();

	// Adds an element at the end of the array
	int Add();
	int Add(const T& t);
	int Add(const T* paTs, int count);
	int Add(const Array<T>& arr);

	// Adds an unexisting element to the list
	// Returns true if the element could be added
	bool AddUnique(const T& t);

	// Initializes the array with an external array
	// The array is resized and contains copies of the given values
	void Set(const T* paTs, int count);

	// Initializes all element with a value
	void Set(const T& t);

	// Gets the number of elements stored
	inline int GetSize() const { return m_count; }

	// Returns true if the array contains no element
	inline bool IsEmpty() const { return ( m_count == 0 ); }

	// Sets the number of elements (does not free memory, see Clear and FreeExtra)
	// Returns true upon success
	bool SetSize(int size);
	
	// Gets an element at a specific position
	inline T& operator [] (int index)
	{
		ASSERTMSG(index >= 0 && index < m_count, "Array::[] - Bad index");
		return m_paTs[index];
	}

	// Gets an element at a specific position
	inline const T& operator [] (int index) const
	{
		ASSERTMSG(index >= 0 && index < m_count, "Array::[] - Bad index");
		return m_paTs[index];
	}

	// Gets an element at a specific position
	inline T& GetAt(int index)
	{
		ASSERTMSG(index >= 0 && index < m_count, "Array::GetAt - Bad index");
		return m_paTs[index];
	}

	// Gets an element at a specific position
	inline const T& GetAt(int index) const
	{
		ASSERTMSG(index >= 0 && index < m_count, "Array::GetAt - Bad index");
		return m_paTs[index];
	}


	const T& GetLast() const
	{
		ASSERTMSG(m_count > 0, "Array::GetLast - Count is 0");
		return m_paTs[m_count - 1];
	}

	T& GetLast()
	{
		ASSERTMSG(m_count > 0, "Array::GetLast - Count is 0");
		return m_paTs[m_count - 1];
	}

	// Removes an element at a specific position
	// Extra elements are shifted in order to cap the hole
	void RemoveAt(int index);
	
	// Removes a given element of the list
	// Returns true if the element could be found
	bool Remove(const T& t);

	/// Removes the last element of the list
	void RemoveLast();

	// Removes all the elements and frees all memory allocated
	void Clear();

	// Gets the position of an element
	// Returns -1 if no element found
	int Find(const T& t) const;
	int Find(const T* paTs, int count) const;

	// Inserts an element at a specific position
	void InsertAt(int index);
	void InsertAt(int index, const T& t);

	void InsertUniqueAt(int index, const T& t);

	// Moves all elements from the current array to another array
	void MoveTo(Array<T>& aReceiver);

	// Swap the content of two arrays
	void Swap(Array<T>& aFriend);

	// Swap two values
	void Swap(int index1, int index2);
	
	// Gets a pointer on the first element of the array
	inline T* GetBuffer() const { return m_paTs; }
	inline T* GetPtr() const { return m_paTs; }

	inline int SizeofElement() { return sizeof(T); }

	///////////////////////////////////////////
	// Sets the minimum wanted allocation
	// Returns true if the allocation could be done
	bool EnsureCapacity(int capacity)
	{
		return EnsureCapacityEx(capacity, m_count, true);
	}
	
	// Gets the number of allocated elements
	int GetCapacity() const { return m_capacity; }

	// Remove all extra allocated memory
	void FreeExtra();

	bool operator == (const Array<T>& arr);
	bool operator != (const Array<T>& arr);

	///////////////////////////////////////////
	// Makes a copy of another array
	Array<T>& operator = (const Array<T>& arr);

protected:
	T*  m_paTs;      // 'T' buffer
	int m_count;     // Number of elements in the array
	int m_capacity; // Number of allocated elements (but not necessarely constructed)
	
protected:
	inline void CallConstructors(int startIndex, int count);
	inline void CallDestructors(int startIndex, int count);

	virtual void DeletePointedData(int startIndex, int count);

private:
	// When one element is added to the array, this compute how much memory should be allocated
	static int GetMinCapacity();
	bool EnsureCapacityEx(int capacity, int elementToCopy, bool freeMemory = true);
	void InsertAtNoConstruct(int index);
	bool Equals(const Array<T>& arr);
};

///////////////////////////////////////////////////////////////////////////////
template <class T>
Array<T>::Array()
{
	m_paTs = nullptr;
	m_count = 0;
	m_capacity = 0;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
Array<T>::Array(int count)
{
	m_paTs = nullptr;
	m_count = 0;
	m_capacity = 0;

	if( !SetSize(count) )
	{
		ASSERT(0); // Better than nothing
	}
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
Array<T>::Array(const Array<T>& arr)
{
	m_paTs = nullptr;
	m_count = 0;
	m_capacity = 0;

	Add(arr.m_paTs, arr.m_count);
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
Array<T>::Array(Array<T>&& arr)
{
	m_paTs = arr.m_paTs;
	m_count = arr.m_count;
	m_capacity = arr.m_capacity;

	arr.m_paTs = nullptr;
	arr.m_count = 0;
	arr.m_capacity = 0;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
Array<T>::Array(const T* paTs, int count)
{
	m_paTs = nullptr;
	m_count = 0;
	m_capacity = 0;

	Add(paTs, count);
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
Array<T>::~Array()
{
	Clear();
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::Clear()
{
	if(m_paTs == nullptr)
		return;

	CallDestructors(0, m_count);
	Memory::Free(m_paTs);

	m_paTs = nullptr;
	m_count = 0;
	m_capacity = 0;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::MoveTo(Array<T>& aReceiver)
{
	aReceiver.Clear();
	aReceiver.m_count = m_count;
	aReceiver.m_capacity = GetCapacity(); // The rbv bit is set to 0
	aReceiver.m_paTs = m_paTs;

	m_paTs = nullptr;
	m_count = 0;
	m_capacity = 0;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::Swap(Array<T>& aFriend)
{
	// Swap count
	int tmp = m_count;
	m_count = aFriend.m_count;
	aFriend.m_count = tmp;

	// Swap m_capacity
	tmp = m_capacity;
	m_capacity = aFriend;
	aFriend.m_capacity = tmp;

	// Swap buffers
	T* pTmp = m_paTs;
	m_paTs = aFriend.m_paTs;
	aFriend.m_paTs = pTmp;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::Swap(int index1, int index2)
{
	ASSERTMSG( 0 <= index1 && index1 <= m_count, "Array::Swap - Bad index 1");
	ASSERTMSG( 0 <= index2 && index2 <= m_count, "Array::Swap - Bad index 2");

	Memory::Swap(m_paTs + index1, m_paTs + index2, sizeof(T));
}

///////////////////////////////////////////////////////////////////////////////
// Assignment operator
template <class T>
Array<T>& Array<T>::operator = (const Array<T>& arr)
{
	if( this == &arr )
		return *this;

	if( m_count < arr.m_count )
	{
		// For each element call the assignment operator
		for(int i = 0; i < m_count; i++)
		{
			m_paTs[i] = arr.m_paTs[i];
		}
		
		// Construct new elements by copy
		Add(arr.m_paTs + m_count, arr.m_count - m_count);
	}
	else
	{
		// THIS array is bigger than the OTHER array

		// Destroy extra elements
		int destroyStartIndex = arr.m_count;
		int destroyCount = m_count - arr.m_count;
		CallDestructors(destroyStartIndex, destroyCount);

		// For each element call the assignment operator
		for(int i = 0; i < arr.m_count; i++)
		{
			m_paTs[i] = arr.m_paTs[i];
		}
		m_count = arr.m_count;
	}
	
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Adds an empty element at the end of the array
// Returns the index of the added element, -1 upon error
template <class T>
int Array<T>::Add()
{
	if( m_capacity <= m_count )
	{
		// Not enough memory, must reallocate
		int newCapacity = (m_count == 0) ? 1 : m_count * 2;
		if( !EnsureCapacity(newCapacity) )
		{
			return -1;
		}
	}

	CallConstructors(m_count, 1);

	const int currentPos = m_count;
	m_count++;

	return currentPos;
}

///////////////////////////////////////////////////////////////////////////////
// Adds an element to the end of the array
// Returns the index of the added element, -1 upon error
template <class T>
int Array<T>::Add(const T& t)
{
	if( m_capacity <= m_count )
	{
		// Not enough memory, must reallocate
		int newCapacity = (m_count == 0) ? 1 : m_count * 2;
		if( !EnsureCapacity(newCapacity) )
		{
			return -1;
		}
	}

	// Call the copy constructor for the new array element
	::new( (void*)(m_paTs + m_count)) T(t);

	const int currentPos = m_count;
	m_count++;

	return currentPos;
}

///////////////////////////////////////////////////////////////////////////////
// Adds several elements to the end of the array
// Returns the index of the last added element, -1 upon error
template <class T>
int Array<T>::Add(const T* paTs, int count)
{
	if( count == 0 )
		return m_count - 1;

	if( !EnsureCapacity(m_count + count) )
	{
		return -1;
	}
	T* pNew = m_paTs + m_count;

	for(int i = 0; i < count; i++)
	{
		// Call the copy constructor for the new array element
		::new( (void*)(pNew + i)) T( paTs[i] );
	}
	m_count += count;
	return m_count - 1;
}

///////////////////////////////////////////////////////////////////////////////
// Adds several elements to the end of the array
// Returns the index of the last added element
template <class T>
int Array<T>::Add(const Array<T>& arr)
{
	return Add(arr.m_paTs, arr.m_count);
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::Set(const T* paTs, int count)
{
	if( !SetSize(count) )
	{
		ASSERT(0); // Better than nothing
		return;
	}

	// Use the assignment operator
	for(int i = 0; i < m_count; i++)
	{
		m_paTs[i] = paTs[i];
	}
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::Set(const T& t)
{
	for(int i = 0; i < m_count; i++)
	{
		m_paTs[i] = t;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Remove the element at a specific index
template <class T>
void Array<T>::RemoveAt(int index)
{
	ASSERTMSG(index >= 0 && index < m_count, "Array::RemoveAt - Bad index");
	
	// Number of element on the right of the element to remove, including the element to remove
	const int rightCount = m_count - index;
	
	// Destroy the element to be removed
	CallDestructors(index, 1);
	if( rightCount > 1 )
	{
		// Translate the last elements to the left
		// memove is needed instead of memcpy in our case
		Memory::Move(m_paTs + index, m_paTs + index + 1, sizeof(T) * (rightCount - 1) );
	}
	m_count--;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::InsertAtNoConstruct(int index)
{
	const int shiftCount = m_count - index; // Number of element to translate

	// Is there any free space so we can translate the current content of the array ?
	if( GetCapacity() <= m_count )
	{
		// Not enough memory
		T* pOld = m_paTs;

		int newCapacity = (m_count == 0) ? 1 : m_count * 2;
		if( !EnsureCapacityEx(newCapacity, index, false) )
		{
			ASSERT(0); // Better than nothing
			return;
		}

		if( shiftCount > 0 )
		{
			Memory::Copy(m_paTs + index + 1, pOld + index, sizeof(T) * shiftCount);
		}
		Memory::Free(pOld);
	}
	else
	{
		if( shiftCount > 0 )
		{
			// Element translation to the right
			Memory::Move(m_paTs + index + 1, m_paTs + index, sizeof(T) * shiftCount);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::InsertAt(int index)
{
	ASSERTMSG(index >= 0 && index <= m_count, "Array::InsertAt - Bad index");
	if( index == m_count )
	{
		Add();
		return;
	}

	InsertAtNoConstruct(index);

	CallConstructors(index, 1);

	m_count++;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::InsertAt(int index, const T& t)
{
	ASSERTMSG(index >= 0 && index <= m_count, "Array::InsertAt - Bad index");
	if( index == m_count )
	{
		Add(t);
		return;
	}

	InsertAtNoConstruct(index);

	// Call the copy constructor for the new array element
	::new( (void*)(m_paTs + index)) T(t);

	m_count++;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::InsertUniqueAt(int index, const T& t)
{
	ASSERTMSG(index >= 0 && index <= m_count, "Array::InsertAt - Bad index");
	if( index == m_count )
	{
		AddUnique(t);
		return;
	}

	if( Find(t) >= 0 )
	{
		// Already exists
		return;
	}

	InsertAtNoConstruct(index);

	// Call the copy constructor for the new array element
	::new( (void*)(m_paTs + index)) T(t);

	m_count++;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::FreeExtra()
{
	if( GetCapacity() == m_count )
		return;

	// If GetCapacity() == 0, it means m_count is 0. So the previous test check this case.
	// If m_count is not 0, the object may by corrupted.
	ASSERT( GetCapacity() > 0);

	if( m_count > 0 )
	{
		// Some data to copy
		const int byteCount = sizeof(T) * m_count;
		void* paNewTs = Memory::Alloc( ROUND64(byteCount) );
		if( paNewTs == nullptr )
		{
			ASSERT(0); // Better than nothing
			return;
		}

		// Copy of the old element into the new array
		int int64Count = Memory::ByteCountToInt64Count(byteCount);
		Memory::Copy64(paNewTs, m_paTs, int64Count);
	
		// Free the memory used by the old array
		Memory::Free(m_paTs);

		m_paTs = (T*) paNewTs;
	}
	else
	{
		// The array is empty
		Memory::Free(m_paTs);
		m_paTs = nullptr;
	}
	m_capacity = m_count;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
int Array<T>::GetMinCapacity()
{
	// 480 bytes or 2 elements is the first size
	// 480 bytes -> 240 bytes for each element
	// With 480 bytes this allows an overhead of 32 bytes to reach 512 bytes,
	// thus making the memory allocator happy

	int wantsByteCount = 480;
	int allocate = wantsByteCount / sizeof(T);
	if( allocate < 2 )
	{
		allocate = 2;
	}
	return allocate;
}

///////////////////////////////////////////////////////////////////////////////
// Allocate as much memory as wanted if necessary
// If allocation is done, previous elements are copied to the new buffer
//
// [in] capacity      Allocation wanted in number of elements
// [in] elementToCopy Number of elements to copy from the old buffer (used for insertion)
// [in] freeMemory    true to free the previous buffer (used for insertion)
//
// Returns true upon success (allocation succeed)
///////////////////////////////////////////////////////////////////////////////
template <class T>
bool Array<T>::EnsureCapacityEx(int capacity, int elementToCopy, bool freeMemory /* = true */)
{
	ASSERT(capacity >= 0);

	if( GetCapacity() < capacity )
	{
		// Needs reallocation

		//////////////////////////////////////////////////////////
		// Check that we do not allocate a too small buffer
		int minCount = GetMinCapacity();
		if( capacity < minCount )
		{
			capacity = minCount;
		}
		//////////////////////////////////////////////////////////

		int newByteCount = capacity * sizeof(T);
		void* paNewTs = Memory::Alloc( ROUND64(newByteCount) );
		if( paNewTs == nullptr )
		{
			return false;
		}

		if( elementToCopy > 0 )
		{
			// Copy of the old element into the new array
			const int oldByteCount = sizeof(T) * elementToCopy;
			int int64Count = Memory::ByteCountToInt64Count(oldByteCount);
			Memory::Copy64(paNewTs, m_paTs, int64Count);
		}

		// freeMemory is used for insertion functions :
		// one should not free memory when inserting elements, because the second part of the old
		// array must be copied
		if( GetCapacity() > 0 && freeMemory )
		{
			// Free the memory used by the old array
			Memory::Free(m_paTs);
		}

		m_paTs = (T*) paNewTs;
		
		// Keep the rbv bit
		m_capacity = capacity;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Shrinks or narrows the array
template <class T>
bool Array<T>::SetSize(int size)
{
	ASSERT(size >= 0);

	if( size == m_count )
		return true;
	
	if( size > 0 )
	{
		if( !EnsureCapacity(size) )
		{
			return false;
		}
	}

	if( size < m_count )
	{
		// Shrinking, must destroy some elements
		CallDestructors(size, m_count - size);
	}
	else
	{
		// Growing, must construct some elements
		CallConstructors(m_count, size - m_count);
	}
	m_count = size;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void Array<T>::CallConstructors(int startIndex, int count)
{
	T* paEls = m_paTs + startIndex;

	for(int i = 0; i < count; ++i)
	{
		T* pElToConstruct = paEls + i;
		::new( (void*)(pElToConstruct)) T;
	}
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void Array<T>::CallDestructors(int startIndex, int count)
{
	(void)startIndex;
	for(int i = 0; i < count; ++i)
	{
		(m_paTs + startIndex + i)->~T(); // Do not forget the parenthesis !
	}
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::DeletePointedData(int /*nStartIndex*/, int /*nCount*/)
{

}

///////////////////////////////////////////////////////////////////////////////
// Returns true if the element could be inserted
template <class T>
bool Array<T>::AddUnique(const T& t)
{
	for(int i = 0; i < m_count; ++i)
	{
		if( m_paTs[i] == t )
			return false; // Element already there
	}
	
	Add(t);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if the element could be removed
template <class T>
bool Array<T>::Remove(const T& t)
{
	for(int i = 0; i < m_count; ++i)
	{
		if( m_paTs[i] == t )
		{
			// Found !
			RemoveAt(i);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
void Array<T>::RemoveLast()
{
	if( m_count == 0 )
	{
		return;
	}

	const int lastIndex = m_count - 1;
	RemoveAt(lastIndex);
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
int Array<T>::Find(const T& t) const
{
	for(int i = 0; i < m_count; ++i)
	{
		if( m_paTs[i] == t )
		{
			// Found !
			return i;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
int Array<T>::Find(const T* paTs, int count) const
{
	const int maxMain = m_count - count;
	if( maxMain < 0 )
		return -1;

	for(int i = 0; i <= maxMain; ++i)
	{
		int j = 0;
		for(; j < count; ++j)
		{
			if( m_paTs[i + j] != paTs[j] )
			{
				break;
			}
		}

		if( j == count )
			return i;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
bool Array<T>::operator == (const Array<T>& arr)
{
	return Equals(arr);
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
bool Array<T>::operator != (const Array<T>& arr)
{
	return !Equals(arr);
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
bool Array<T>::Equals(const Array<T>& arr)
{
	if( m_count != arr.m_count )
	{
		return false;
	}
	for(int i = 0; i < m_count; ++i)
	{
		if( m_paTs[i] != arr.m_paTs[i] )
		{
			return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Predefined arrays
class ByteArray : public Array<uint8>
{
public:
	ByteArray() {}
	ByteArray(const uint8* p, int size) : Array<uint8>(p, size) {}
	~ByteArray() {}
};

class Int32Array : public Array<int>
{
public:
	Int32Array() {}
	~Int32Array() {}
};

class UintArray : public Array<uint32>
{
public:
	UintArray() {}
	~UintArray() {}
};

class StringArray : public Array<chustd::String>
{
public:
	StringArray() {}
	~StringArray() {}

	StringArray Sort() const;
};


// This class is used to create a ByteArray from an external buffer
// TODO : maybe we need a better architecture to convert external buffers to ByteArrays
class ByteArrayBox
{
public:
	ByteArrayBox(uint8* pExternalBuffer, int size);
	virtual ~ByteArrayBox(); // TODO : remove the virtual when ByteArray will be a self-contained class with no virtual

	const ByteArray& ToByteArray() const;

	void Set(uint8* pExternalBuffer, int size);

private:
	uint8* m_paTs;      // byte buffer
	int    m_count;     // Number of elements in the array
	int    m_capacity;
};
/////////////////////////////////////////////////////////////////////////////////////

} // namespace chustd

/////////////////////////////////////////////////////////////////////////////////////

// Redefined the CHUDEBUG_NEW macro
#ifdef ARRAY_OLDNEW
#define new CHUDEBUG_NEW
#endif

#endif // ndef CHUSTD_ARRAY_H

