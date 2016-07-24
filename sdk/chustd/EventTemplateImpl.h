///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#define JOIN( X, Y )				DO_JOIN( X, Y )
#define DO_JOIN( X, Y )				DO_DO_JOIN(X,Y)
#define DO_DO_JOIN( X, Y )			X##Y

#define EVENT_NAME					JOIN(Event, ARGS_COUNT)
#define OBSERVER_NAME				JOIN(Observer, ARGS_COUNT)
	
#if ARGS_COUNT == 0
#	define EVENT_TEMPLATE_ARGS		
#	define EVENT_TYPE				EVENT_NAME
#	define OBSERVER_TYPE			OBSERVER_NAME
#else
#	define EVENT_TEMPLATE_ARGS		template <ARGS_TEMPLATE>	
#	define EVENT_TYPE				EVENT_NAME<ARGS_TYPE>
#	define OBSERVER_TYPE			OBSERVER_NAME<ARGS_TYPE>
#endif

namespace chustd {

EVENT_TEMPLATE_ARGS
void EVENT_TYPE::Fire(ARGS_TYPENAME) const
{
	int observerCount = m_apObservers.GetSize();
	
	for( int i = 0; i < observerCount; ++i )
	{
		m_apObservers[i]->Fire(ARGS_NAME);
	}
}

EVENT_TEMPLATE_ARGS
EVENT_TYPE::~EVENT_NAME()
{
	int observerCount = m_apObservers.GetSize();

	for( int i = 0; i < observerCount; ++i )
	{
		OBSERVER_TYPE* pObserver = m_apObservers[i];
		delete pObserver;
	}
}

} // namespace chustd

#undef EVENT_NAME
#undef OBSERVER_NAME
#undef EVENT_TEMPLATE_ARGS
#undef EVENT_TYPE
#undef OBSERVER_TYPE

#undef DO_DO_JOIN
#undef DO_JOIN
#undef JOIN
