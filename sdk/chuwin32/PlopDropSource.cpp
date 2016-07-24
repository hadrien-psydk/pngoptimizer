///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlopDropSource.h"
#include "ComFromHell.h"

//////////////////////////////////////////////////////////////////////
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////

PlopDropSource::PlopDropSource()
{
	m_nRefCount = 1;
}

PlopDropSource::~PlopDropSource()
{
	
}

STDMETHODIMP  PlopDropSource::QueryInterface(REFIID refiid, void FAR* FAR* ppv)
{
	*ppv = NULL;
	if( ComFromHell::Equals(IID_IUnknown, refiid) || ComFromHell::Equals(IID_IDropSource, refiid) )
		*ppv=this;
	
	if (NULL != *ppv)
	{
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) PlopDropSource::AddRef(void)
{
	return ++m_nRefCount;
}

STDMETHODIMP_(ULONG) PlopDropSource::Release(void)
{
	int32 nTempCount = --m_nRefCount;
	
	if( nTempCount == 0 )
		delete this;

	return nTempCount; 
}

STDMETHODIMP PlopDropSource::GiveFeedback(DWORD /*dwEffect*/)
{
	return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

STDMETHODIMP PlopDropSource::QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState)
{
	if( fEscapePressed )
		return ResultFromScode(DRAGDROP_S_CANCEL);

	if( !(grfKeyState & MK_LBUTTON) )
		return ResultFromScode(DRAGDROP_S_DROP);
	
	return ResultFromScode(S_OK);
}
