///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_PLOPDROPSOURCE_H
#define CHUWIN32_PLOPDROPSOURCE_H

namespace chuwin32 {

class PlopDropSource : public IDropSource
{
public:
	//IUnknown members
	STDMETHOD(QueryInterface)(REFIID,  void FAR* FAR* );
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	//IDataObject members
	STDMETHOD(GiveFeedback)( DWORD dwEffect);
	STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed,DWORD grfKeyState	);

	PlopDropSource();
	virtual ~PlopDropSource();

private:
	int32 m_nRefCount;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_PLOPDROPSOURCE_H
