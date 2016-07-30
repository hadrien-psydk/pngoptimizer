///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_PROPERTY_H
#define CHUSTD_PROPERTY_H

#include "Event2.h"

/*
#pragma warning( push )
#pragma warning( disable : 4284 ) // Level3 - return type for 'Property<TYPE>::operator ->' is 'TYPE *' 
                                  // (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)
*/
namespace chustd {

template <typename TYPE>
class Property
{
public:
	typedef Event2<const TYPE&, const TYPE&> Event;
	typedef typename Event::Handler Handler;
	
public:
	operator TYPE() const;

	const Property<TYPE>& operator = (const TYPE& value);
	const Property<TYPE>& operator = (const Property<TYPE>& property);

	TYPE* operator -> ();
	const TYPE* operator -> () const;

	const TYPE& GetValue() const { return m_value; }
	
	template <typename TARGET>
	Handler Connect(TARGET* pTarget, void(TARGET::*method)(const TYPE&, const TYPE&)) const
	{
		return m_event.Connect(pTarget, method);
	}

public:
	Property(const TYPE& value);
	Property();

private:
	TYPE m_value;
	Event m_event;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
template <typename TYPE>
Property<TYPE>::operator TYPE() const
{
	return m_value;
}


///////////////////////////////////////////////////////////////
template <typename TYPE>
const Property<TYPE>& Property<TYPE>::operator = (const TYPE& value)
{
	if( !(m_value == value) )
	{
		TYPE oldValue = m_value;
		m_value = value;
		m_event.Fire(*this, oldValue);
	}
	return *this;
}

///////////////////////////////////////////////////////////////
template <typename TYPE>
const Property<TYPE>& Property<TYPE>::operator = (const Property<TYPE>& property)
{
	return (*this = property.m_value);
}

///////////////////////////////////////////////////////////////
template <typename TYPE>
TYPE* Property<TYPE>::operator -> ()
{
	return &m_value;
}

///////////////////////////////////////////////////////////////
template <typename TYPE>
const TYPE* Property<TYPE>::operator -> () const
{
	return &m_value;
}


///////////////////////////////////////////////////////////////
template <typename TYPE>
Property<TYPE>::Property(const TYPE& value)
	: m_value(value)
{
}

///////////////////////////////////////////////////////////////
template <typename TYPE>
Property<TYPE>::Property()
{
}

} // namespace chustd

///////////////////////////////////////////////////////////////

//#pragma warning( pop )

#endif // ndef CHUSTD_PROPERTY_H
