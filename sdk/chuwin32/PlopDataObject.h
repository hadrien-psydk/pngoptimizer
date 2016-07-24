///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_PLOPDATAOBJECT_H
#define CHUWIN32_PLOPDATAOBJECT_H

#include "PlopEnumFormatEtc.h"

namespace chuwin32 {

class PlopDataObject : public IDataObject
{
public:
	// IUnknown members that delegate to m_pUnkOuter.
	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	
	// IDataObject methods
	STDMETHOD(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium );
	STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
	STDMETHOD(QueryGetData)(LPFORMATETC pformatetc );
	STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
	STDMETHOD(SetData)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium,
		BOOL fRelease);
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
	STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf, 
		LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
	STDMETHOD(DUnadvise)(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);
	
	PlopDataObject();
	virtual ~PlopDataObject();
	
private:
	int32 m_nRefCount;
	chustd::Array<Both> m_aFormats;

private:
	HGLOBAL DuplicateGlobalBuffer(HGLOBAL hSrc);

	HRESULT FindFORMATETC(FORMATETC* pfe, Both** ppde, BOOL fAdd);
	HRESULT AddRefStgMedium(STGMEDIUM* pstgmIn, STGMEDIUM* pstgmOut, BOOL fCopyIn);
};


} // namespace chuwin32

#endif // ndef CHUWIN32_PLOPDATAOBJECT_H
