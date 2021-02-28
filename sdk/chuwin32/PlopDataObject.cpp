///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlopDataObject.h"
#include "PlopEnumFormatEtc.h"
#include "ComFromHell.h"

//////////////////////////////////////////////////////////////////////
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////

PlopDataObject::PlopDataObject()
{
	m_nRefCount = 1;
}

PlopDataObject::~PlopDataObject()
{
	foreach(m_aFormats, i)
	{
		CoTaskMemFree( m_aFormats[i].m_fe.ptd);
		ReleaseStgMedium( &m_aFormats[i].m_sm);
    }
}


STDMETHODIMP  PlopDataObject::QueryInterface(REFIID refiid, void FAR* FAR* ppv)
{
	*ppv = nullptr;
	if( ComFromHell::Equals(IID_IUnknown, refiid) || ComFromHell::Equals(IID_IDataObject, refiid) )
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

STDMETHODIMP_(ULONG) PlopDataObject::AddRef(void)
{
	return ++m_nRefCount;
}

STDMETHODIMP_(ULONG) PlopDataObject::Release(void)
{
	int32 nTempCount = --m_nRefCount;
	
	if( nTempCount==0 )
		delete this;

	return nTempCount;
}

IUnknown *GetCanonicalIUnknown(IUnknown *punk)
{
    IUnknown *punkCanonical;
    if (punk && SUCCEEDED(punk->QueryInterface(IID_IUnknown,
                                               (LPVOID*)&punkCanonical)))
	{
        punkCanonical->Release();
    }
	else
	{
        punkCanonical = punk;
    }
    return punkCanonical;
}

//IDataObject members
STDMETHODIMP PlopDataObject::SetData(LPFORMATETC pFE, LPSTGMEDIUM pSM, BOOL fRelease)
{
	if( pFE == nullptr && pSM == nullptr )
		return E_INVALIDARG;
	
	if (!fRelease) 
		return E_NOTIMPL;
	
	Both* pde = nullptr;
	HRESULT hres = FindFORMATETC(pFE, &pde, TRUE);
	if (SUCCEEDED(hres))
	{
		if( pde->m_sm.tymed) 
		{
			ReleaseStgMedium(&pde->m_sm);
			chustd::Memory::Zero(&pde->m_sm, sizeof(STGMEDIUM));
		}
		
		if (fRelease)
		{
			pde->m_sm = *pSM;
			hres = S_OK;
		}
		else 
		{
			hres = AddRefStgMedium(pSM, &pde->m_sm, TRUE);
		}
		pde->m_fe.tymed = pde->m_sm.tymed;	// Keep in sync 
		
		// Subtlety!  Break circular reference loop
		if (GetCanonicalIUnknown(pde->m_sm.pUnkForRelease) ==
			GetCanonicalIUnknown(static_cast<IDataObject*>(this)))
		{
			pde->m_sm.pUnkForRelease->Release();
			pde->m_sm.pUnkForRelease = nullptr;
		}
	}
	return hres;
#if 0
	/////////////////////////////////////////////////
	// Ancien code
	bool bFound = false;
	foreach(m_aFormats, i)
	{
		Both& both = m_aFormats[i];

		if( both.m_fe.cfFormat == pFE->cfFormat )
		{
			both.m_sm = *pSM;
			both.m_fe = *pFE;
			both.m_bOwner = fRelease != 0;
			bFound = true;
		}
	}

	if( !bFound )
	{
		m_aFormats.Add();
		
		Both& both = m_aFormats.GetLast();
		both.m_sm = *pSM;
		both.m_fe = *pFE;
		both.m_bOwner = fRelease != 0;
	}
	return S_OK;
#endif
}

HGLOBAL PlopDataObject::DuplicateGlobalBuffer(HGLOBAL hSrc)
{
	DWORD nSrcSize = (DWORD) GlobalSize(hSrc);
	PVOID pSrc = GlobalLock(hSrc);

	PVOID pDst = GlobalAlloc(GMEM_FIXED, nSrcSize); // GMEM_FIXED => the return value is a pointer

	chustd::Memory::Copy(pDst, pSrc, nSrcSize);

	GlobalUnlock(hSrc);

	return pDst;
}

STDMETHODIMP PlopDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSM)
{
	if( pFE == nullptr ||pSM == nullptr )
		return E_INVALIDARG;


	Both* pde = nullptr;
    HRESULT hres = FindFORMATETC(pFE, &pde, FALSE);
    if (SUCCEEDED(hres))
	{
        hres = AddRefStgMedium(&pde->m_sm, pSM, FALSE);
    }

    return hres;
#if 0
	///////////////////////////////////////////
	// Ancien code
	pSM->hGlobal = nullptr;
	
	foreach(m_aFormats, i)
	{
		const Both& both = m_aFormats[i];

		/*
		chustd::String str;
		str.Format("pFE->cfFormat = %.4x ~ pFE->tymed = %d\n", pFE->cfFormat, pFE->tymed);
		DKTRACE(str);*/

		return DV_E_FORMATETC;

		if( pFE->tymed & both.m_fe.tymed &&
			pFE->dwAspect == both.m_fe.dwAspect &&
			pFE->cfFormat == both.m_fe.cfFormat)
		{
			pSM->tymed = both.m_sm.tymed;
			
			if( pFE->cfFormat== CF_HDROP )
			{
				// Sans cette duplication, le fonctionnement est bancal :
				// - certaines applications veulent bien du buffer
				// - le shell n'en veut pas
				pSM->hGlobal = DuplicateGlobalBuffer(both.m_sm.hGlobal);
				pSM->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
			else if( both.m_fe.tymed == TYMED_HGLOBAL )
			{
				pSM->hGlobal = DuplicateGlobalBuffer(both.m_sm.hGlobal);
				pSM->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
		}
	}
		
	return DV_E_FORMATETC;
#endif
}

STDMETHODIMP PlopDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSM)
{
	if(pFE == nullptr ||pSM == nullptr)
		return E_INVALIDARG;
	
	return E_NOTIMPL;
}

STDMETHODIMP PlopDataObject::QueryGetData(LPFORMATETC lpFormat)
{ 
	Both* pde = nullptr;
	return FindFORMATETC(lpFormat, &pde, FALSE);

#if 0
	//////////////////////////////////////
	// Ancien code
	if(!lpFormat)
		return ResultFromScode(S_FALSE);	  
	
	//
	// Check the aspects we support.  Implementations of this object will only
	// support DVASPECT_CONTENT.
	//
	if (!(DVASPECT_CONTENT & lpFormat->dwAspect))
		return (DV_E_DVASPECT);
	//
	// Now check for an appropriate clipboard format and TYMED.
	//
	foreach(m_aFormats, i)
	{
		const Both& both = m_aFormats[i];

		if(( lpFormat->cfFormat == both.m_fe.cfFormat) && 
			(lpFormat->tymed & both.m_fe.tymed))
			return S_OK;
	}
	return DV_E_TYMED;
#endif
}

STDMETHODIMP PlopDataObject::GetCanonicalFormatEtc(LPFORMATETC /*pFE1*/, LPFORMATETC /*pFE2*/)
{
	return DATA_S_SAMEFORMATETC;
}


STDMETHODIMP PlopDataObject::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC FAR *pEnum)
{
	if( dwDir == DATADIR_GET )
	{
		*pEnum = new PlopEnumFormatEtc(m_aFormats);
	}
	else if( dwDir == DATADIR_SET )
	{
		pEnum = nullptr;
	}

	if( pEnum == nullptr )
	{
		return E_NOTIMPL;
	}
	
	return S_OK;
}

STDMETHODIMP PlopDataObject::DAdvise(FORMATETC FAR* /*pFE*/, DWORD /*advf*/, LPADVISESINK /*pAdvSink*/, DWORD FAR* /*pdwConnection*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP PlopDataObject::DUnadvise(DWORD /*dwConnection*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP PlopDataObject::EnumDAdvise(LPENUMSTATDATA FAR* /*ppenumAdvise*/)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT PlopDataObject::FindFORMATETC(FORMATETC* pfe, Both** ppde, BOOL fAdd)
{
    *ppde = nullptr;
	
    // Comparing two DVTARGETDEVICE structures is hard, so we don't even try
    if (pfe->ptd != nullptr) return DV_E_DVTARGETDEVICE;
	
    // See if it's in our list
    foreach(m_aFormats, ide)
	{
        if (m_aFormats[ide].m_fe.cfFormat == pfe->cfFormat &&
            m_aFormats[ide].m_fe.dwAspect == pfe->dwAspect &&
            m_aFormats[ide].m_fe.lindex == pfe->lindex)
		{
            if (fAdd || (m_aFormats[ide].m_fe.tymed & pfe->tymed))
			{
                *ppde = &m_aFormats[ide];
                return S_OK;
            } else {
                return DV_E_TYMED;
            }
        }
    }
	
    if (!fAdd)
		return DV_E_FORMATETC;
	
	int32 ide = m_aFormats.Add();
   
	m_aFormats[ide].m_fe = *pfe;
	chustd::Memory::Zero(&m_aFormats[ide].m_sm, sizeof(STGMEDIUM));

	*ppde = &m_aFormats[ide];
	return S_OK;
}

HGLOBAL GlobalClone(HGLOBAL hglobIn)
{
	HGLOBAL hglobOut = nullptr;
	
	LPVOID pvIn = GlobalLock(hglobIn);
	if (pvIn)
	{
		DWORD cb = (DWORD) GlobalSize(hglobIn);
		hglobOut = GlobalAlloc(GMEM_FIXED, cb);
		if (hglobOut)
		{
			chustd::Memory::Copy(hglobOut, pvIn, cb);
		}
		GlobalUnlock(hglobIn);
	}
	
	return hglobOut;
}

HRESULT PlopDataObject::AddRefStgMedium(STGMEDIUM* pstgmIn, STGMEDIUM* pstgmOut, BOOL fCopyIn)
{
    HRESULT hres = S_OK;
    STGMEDIUM stgmOut = *pstgmIn;
	
    if (pstgmIn->pUnkForRelease == nullptr &&
        !(pstgmIn->tymed & (TYMED_ISTREAM | TYMED_ISTORAGE)))
	{
        if (fCopyIn)
		{
            // Object needs to be cloned
            if (pstgmIn->tymed == TYMED_HGLOBAL)
			{
                stgmOut.hGlobal = GlobalClone(pstgmIn->hGlobal);
                if (!stgmOut.hGlobal)
				{
                    hres = E_OUTOFMEMORY;
                }
            }
			else
			{
                hres = DV_E_TYMED;      // Don't know how to clone GDI objects
            }
        }
		else
		{
            stgmOut.pUnkForRelease = static_cast<IDataObject*>(this);
        }
    }
	
    if (SUCCEEDED(hres))
	{
        switch (stgmOut.tymed)
		{
        case TYMED_ISTREAM:
            stgmOut.pstm->AddRef();
            break;
        case TYMED_ISTORAGE:
            stgmOut.pstg->AddRef();
            break;
        }
        if (stgmOut.pUnkForRelease)
		{
            stgmOut.pUnkForRelease->AddRef();
        }
		
        *pstgmOut = stgmOut;
    }
	
    return hres;
}