/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_POTRACECTL_H
#define PO_POTRACECTL_H

class POApplication;

// Trace control used by PngOptimizer main window

// What's new in comparison with the base class (TraceCtl) ?
// --> we add link management so PngOptimizer can act as a drag-and-drop source

class POTraceCtl : public chuwin32::TraceCtl
{
public:
	POTraceCtl();
	void SetApplication(POApplication* pApp);

protected:
	virtual int  OnLinkDragBegin(const String& strUrl);
	virtual void OnLinkDoubleClick(const String& strUrl);

	HGLOBAL CreateDropFilesW(const String& strUrl);

private:
	POApplication* m_pApp;
};

#endif // ndef PO_POTRACECTL_H
