///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "PtrArray.h"

#define JOIN( X, Y )				DO_JOIN( X, Y )
#define DO_JOIN( X, Y )				DO_DO_JOIN(X,Y)
#define DO_DO_JOIN( X, Y )			X##Y

#define EVENT_NAME					JOIN(Event, ARGS_COUNT)
#define OBSERVER_NAME				JOIN(Observer, ARGS_COUNT)
#define OBSERVERTEMPLATE_NAME		JOIN(ObserverTemplate, ARGS_COUNT)
	
#if ARGS_COUNT == 0
#	define EVENT_TEMPLATE_ARGS		
#	define OBSERVER_TEMPLATE_ARGS	template <typename TARGET>
#	define OBSERVER_TYPE			OBSERVER_NAME
#	define OBSERVERTEMPLATE_TYPE	OBSERVERTEMPLATE_NAME<TARGET>
#else
#	define EVENT_TEMPLATE_ARGS		template <ARGS_TEMPLATE>	
#	define OBSERVER_TEMPLATE_ARGS	template <ARGS_TEMPLATE, typename TARGET>
#	define OBSERVER_TYPE			OBSERVER_NAME<ARGS_TYPE>
#	define OBSERVERTEMPLATE_TYPE	OBSERVERTEMPLATE_NAME<ARGS_TYPE, TARGET>
#endif

namespace chustd {

//////////////////////////////////////////////////////////////////////
EVENT_TEMPLATE_ARGS
class OBSERVER_NAME
{
public:
	virtual void Fire(ARGS_TYPENAME) = 0;
	
public:
	virtual ~OBSERVER_NAME() {}
};

//////////////////////////////////////////////////////////////////////
OBSERVER_TEMPLATE_ARGS
class OBSERVERTEMPLATE_NAME : public OBSERVER_TYPE
{
public:
	virtual void Fire(ARGS_TYPENAME)
	{
		(m_pTarget->*m_pMethod)(ARGS_NAME);
	}
	
public:
	OBSERVERTEMPLATE_NAME(TARGET* pTarget, void (TARGET::*pMethod)(ARGS_TYPE)) : m_pTarget(pTarget), m_pMethod(pMethod) {}
	virtual ~OBSERVERTEMPLATE_NAME() {}

private:
	TARGET* m_pTarget;
	void (TARGET::*m_pMethod)(ARGS_TYPE);
};

//////////////////////////////////////////////////////////////////////
EVENT_TEMPLATE_ARGS
class EVENT_NAME
{
	typedef chustd::PtrArrayNoDelete<OBSERVER_TYPE> ObserverPtrArray;
	
public:
	///////////////////////////////////////////////////
	class Handler
	{
		friend class EVENT_NAME;
	public:
		void Disconnect()
		{
			m_papObservers->Remove(m_pObserver);
			m_bShouldDeleteObserver = true;
		}

		void Reconnect()
		{
			m_papObservers->Add(m_pObserver);
			m_bShouldDeleteObserver = false;
		}

	public:
		Handler(ObserverPtrArray& apObservers, OBSERVER_TYPE& observer): 
			m_papObservers(&apObservers), 
			m_pObserver(&observer), 
			m_bShouldDeleteObserver(false) 
		{}

		Handler()
			: m_papObservers(nullptr), m_pObserver(nullptr), m_bShouldDeleteObserver(false) {}

		~Handler() 
		{
			if( m_bShouldDeleteObserver )
				delete m_pObserver;
		}

	private:
		ObserverPtrArray* m_papObservers;
		OBSERVER_TYPE* m_pObserver;
		bool m_bShouldDeleteObserver;
	};
	
public:
	///////////////////////////////////////////////////
	template <typename TARGET>
	Handler Connect(TARGET* pTarget, void (TARGET::*pMethod)(ARGS_TYPE)) const
	{
		OBSERVER_TYPE* pObserver = new OBSERVERTEMPLATE_TYPE(pTarget, pMethod);
		m_observers.Add(pObserver);
		return Handler(m_observers, *pObserver);
	}

	///////////////////////////////////////////////////
	bool IsConnected() const { return ( m_observers.GetSize() != 0 ); }
	
	///////////////////////////////////////////////////
	void Fire(ARGS_TYPENAME) const;
	
public:
	EVENT_NAME() {}
	~EVENT_NAME();

private:
	
private:
	mutable ObserverPtrArray m_observers;
};

} // namespace chustd

//////////////////////////////////////////////////////////////////////

#undef EVENT_TEMPLATE_ARGS
#undef OBSERVER_TEMPLATE_ARGS
#undef EVENT_NAME
#undef OBSERVER_NAME
#undef OBSERVERTEMPLATE_NAME
#undef OBSERVER_TYPE
#undef OBSERVERTEMPLATE_TYPE

#undef DO_DO_JOIN
#undef DO_JOIN
#undef JOIN

