///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_PLOPENUMFORMATETC_H
#define CHUWIN32_PLOPENUMFORMATETC_H

namespace chuwin32 {

struct Both
{
	FORMATETC m_fe;
	STGMEDIUM m_sm;
	bool m_bOwner;
};

class PlopEnumFormatEtc : public IEnumFORMATETC  
{
public:
	PlopEnumFormatEtc(const chustd::Array<Both>& aFormats);
	virtual ~PlopEnumFormatEtc();

     BOOL FInit(HWND);

     //IUnknown members that delegate to m_pUnkOuter.
     STDMETHOD(QueryInterface)(REFIID, void FAR* FAR*);
     STDMETHOD_(ULONG, AddRef)(void);
     STDMETHOD_(ULONG, Release)(void);

     //IEnumFORMATETC members
     STDMETHOD(Next)(ULONG, LPFORMATETC, ULONG FAR *);
     STDMETHOD(Skip)(ULONG);
     STDMETHOD(Reset)(void);
     STDMETHOD(Clone)(IEnumFORMATETC FAR * FAR*);

private:
	chustd::Array<Both> m_aFormats;
	int32	m_nRefCount;
	int32	m_iCur;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_PLOPENUMFORMATETC_H
