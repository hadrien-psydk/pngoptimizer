/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POTraceCtl.h"
#include "POApplication.h"

///////////////////////////////////////////////////////////////////////////////
POTraceCtl::POTraceCtl()
{
	m_pApp = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
void POTraceCtl::SetApplication(POApplication* pApp)
{
	m_pApp = pApp;
}

///////////////////////////////////////////////////////////////////////////////
HGLOBAL POTraceCtl::CreateDropFilesW(const String& strUrl)
{
	int nBufferSize = sizeof(DROPFILES) + sizeof(wchar) * (strUrl.GetLength() + 1) + sizeof(wchar);

	// Allocate memory from the heap for the DROPFILES struct.
	HGLOBAL hgDrop = GlobalAlloc( GHND | GMEM_SHARE, nBufferSize);
	if( nullptr == hgDrop )
		return 0;

	DROPFILES* pDrop = (DROPFILES*) GlobalLock(hgDrop);
	if( nullptr == pDrop )
	{
		GlobalFree(hgDrop);
		return 0;
	}

	pDrop->pFiles = sizeof(DROPFILES);
	pDrop->fWide = TRUE;

	wchar* pszBuff = (wchar*) (LPBYTE(pDrop) + sizeof(DROPFILES));
	chustd::Memory::Copy16(pszBuff, strUrl.GetBuffer(), strUrl.GetLength() + 1);

	GlobalUnlock(hgDrop);

	return hgDrop;
}

// The user started to move a link in the trace ctl
int POTraceCtl::OnLinkDragBegin(const String& strUrl)
{
	int nDragResult = dragNone;

	///////////////////////////////////////////////////////////
	// Allocate resource for the data to share between PngOptimizer and the target application
	// Here it will be some bytes to store a file path
	HGLOBAL  hgDrop = CreateDropFilesW(strUrl);
	/////////////////////////////////////////////////////////

	FORMATETC etc;
	etc.cfFormat = CF_HDROP;	// CLIPFORMAT
	etc.ptd = nullptr;				// DVTARGETDEVICE*
	etc.dwAspect = DVASPECT_CONTENT;	// DWORD
	etc.lindex = -1;			// LONG
	etc.tymed = TYMED_HGLOBAL;	// DWORD

	STGMEDIUM med;
	med.tymed = TYMED_HGLOBAL;
	med.hGlobal = hgDrop;
	med.pUnkForRelease = nullptr;

	IDataObject* pDataObject = new PlopDataObject;
	if( pDataObject )
	{
		pDataObject->SetData(&etc, &med, TRUE);

		IDropSource* pDropSource = new PlopDropSource;
		if( pDropSource )
		{
			// Forbid drag-and-drop on ourself
			::DragAcceptFiles( ::GetParent(m_handle), FALSE);

			// DoDragDrop will manage the rest of the action and will return when a drag-and-drop action is done
			DWORD nEffect = 0;
			HRESULT hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK, &nEffect);

			// Unless an optimization is in progress, enable again drag-and-drop on ourself
			if( !m_pApp->IsJobRunning() )
			{
				::DragAcceptFiles( ::GetParent(m_handle), TRUE);
			}

			if( hr == DRAGDROP_S_DROP )
			{
				// Testing proved that we cannot rely on the return code given by Windows to know the exact
				// operation that occurred during DoDragDrop. It depends on the target application,
				// it depends on the Windows version and maybe the position of the stars in the sky

				// So we check ourselves if DoDragDrop performed a copy or a move by testing if our file is still here

				if( !File::Exists(strUrl) )
				{
					// The source file does not exist anymore, then it was a move operation
					nDragResult = dragMove;
				}
				else
				{
					nDragResult = dragCopy;
				}
			}
			pDropSource->Release();
		}
		pDataObject->Release();
	}

	GlobalFree(hgDrop);

	return nDragResult;
}

// Open the favorite viewing application do display the screenshot
void POTraceCtl::OnLinkDoubleClick(const String& strUrl)
{
	::ShellExecuteW(m_handle, L"open", strUrl.GetBuffer(), L"", L"", SW_SHOW);
}

