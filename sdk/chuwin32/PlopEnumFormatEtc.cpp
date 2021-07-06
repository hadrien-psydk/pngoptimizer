///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlopEnumFormatEtc.h"
#include "ComFromHell.h"

//////////////////////////////////////////////////////////////////////
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////

PlopEnumFormatEtc::PlopEnumFormatEtc(const chustd::Array<Both>& aFormats)
{
	m_nRefCount = 1;
	m_iCur = 0;
	m_aFormats = aFormats;
}

PlopEnumFormatEtc::~PlopEnumFormatEtc()
{
}

STDMETHODIMP  PlopEnumFormatEtc::QueryInterface(REFIID refiid, void FAR * FAR* ppv)
{
	//	 OutputDebugString("PlopEnumFormatEtc::QueryInterface()\n");
	*ppv = nullptr;
	if( ComFromHell::Equals(IID_IUnknown, refiid) || ComFromHell::Equals(IID_IEnumFORMATETC, refiid) )
	{
		*ppv = this;
	}
	
	if( nullptr != *ppv )
	{
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) PlopEnumFormatEtc::AddRef(void)
{
	return ++m_nRefCount;
}

STDMETHODIMP_(ULONG) PlopEnumFormatEtc::Release(void)
{
	int32 nTempCount = --m_nRefCount;
	
	if( nTempCount==0 )
		delete this;

	return nTempCount; 
}

STDMETHODIMP PlopEnumFormatEtc::Next( ULONG celt, LPFORMATETC lpFormatEtc, ULONG FAR * pceltFetched)
{
	ULONG cReturn = 0L;
	
	if(celt <=0 ||lpFormatEtc == nullptr || m_iCur >= m_aFormats.GetSize())
		return ResultFromScode(S_FALSE);
	
	if(pceltFetched == nullptr && celt != 1)
		return ResultFromScode(S_FALSE);
	
	if(pceltFetched!=nullptr)
		*pceltFetched =0;
	
	while (m_iCur < m_aFormats.GetSize() && celt > 0)
	{
		*lpFormatEtc++ = m_aFormats[m_iCur++].m_fe;
		cReturn++;
		celt--;
	}
	if (nullptr != pceltFetched)
		*pceltFetched = (cReturn - celt);
	return ResultFromScode(S_OK);
}

STDMETHODIMP PlopEnumFormatEtc::Skip(ULONG celt)
{
	if( int32(m_iCur + celt) >= m_aFormats.GetSize() )
		return (ResultFromScode(S_FALSE));
	
	m_iCur += celt;
	return (NOERROR);
}

STDMETHODIMP PlopEnumFormatEtc::Reset(void)
{
	m_iCur = 0;
	return ResultFromScode(S_OK);
}

STDMETHODIMP PlopEnumFormatEtc::Clone(IEnumFORMATETC FAR * FAR * ppCloneEnumFormatEtc)
{
	PlopEnumFormatEtc* pNewEnum = nullptr;

	if( ppCloneEnumFormatEtc == nullptr )
		return ResultFromScode(S_FALSE);
	
	pNewEnum = new PlopEnumFormatEtc(m_aFormats);
	if( pNewEnum == nullptr )
		return (ResultFromScode(E_OUTOFMEMORY));	

	pNewEnum->m_iCur = m_iCur;
	*ppCloneEnumFormatEtc = pNewEnum;
	return ResultFromScode(S_OK);
}
