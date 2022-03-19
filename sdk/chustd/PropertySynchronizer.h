///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_PROPERTYSYNCHRONIZER_H
#define CHUSTD_PROPERTYSYNCHRONIZER_H

#include "Property.h"

/*
#pragma warning( push )
#pragma warning( disable : 4355 ) // Level3 - 'this' : used in base member initializer list
*/

namespace chustd {

template <typename TYPE>
class PropertySynchronizer
{
public:
	void Connect(Property<TYPE>& property1, Property<TYPE>& property2);

public:
	PropertySynchronizer();
	


private:
	typedef typename Property<TYPE>::Event::Handler Handler;

private:
	Handler m_handler1;
	Handler m_handler2;
	
	Property<TYPE>* m_pProperty1;
	Property<TYPE>* m_pProperty2;
	
private:
	void OnProperty1Change(const TYPE&, const TYPE&);	
	void OnProperty2Change(const TYPE&, const TYPE&);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
template <typename TYPE>
PropertySynchronizer<TYPE>::PropertySynchronizer()
{
	m_pProperty1 = nullptr;
	m_pProperty2 = nullptr;
}

//////////////////////////////////////////////////////////////////////
template <typename TYPE>
void PropertySynchronizer<TYPE>::Connect(Property<TYPE>& property1, Property<TYPE>& property2)
{
	if( m_pProperty1 )
	{
		m_handler1.Disconnect();
	}
	if( m_pProperty2 )
	{
		m_handler2.Disconnect();
	}

	m_pProperty1 = &property1;
	m_pProperty2 = &property2;
	m_handler1 = property1.Connect(this, &PropertySynchronizer<TYPE>::OnProperty1Change);
	m_handler2 = property2.Connect(this, &PropertySynchronizer<TYPE>::OnProperty2Change);
}

//////////////////////////////////////////////////////////////////////
template <typename TYPE>
void PropertySynchronizer<TYPE>::OnProperty1Change(const TYPE&, const TYPE&)	
{
	m_handler2.Disconnect();
	*m_pProperty2 = *m_pProperty1;
	m_handler2.Reconnect();
}

//////////////////////////////////////////////////////////////////////
template <typename TYPE>
void PropertySynchronizer<TYPE>::OnProperty2Change(const TYPE&, const TYPE&)
{
	m_handler1.Disconnect();
	*m_pProperty1 = *m_pProperty2;
	m_handler1.Reconnect();
}

} // namespace chustd

//////////////////////////////////////////////////////////////////////

//#pragma warning( pop )

#endif // ndef CHUSTD_PROPERTYSYNCHRONIZER_H
