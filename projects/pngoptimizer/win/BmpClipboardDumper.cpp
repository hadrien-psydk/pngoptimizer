/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BmpClipboardDumper.h"

///////////////////////////////////////////////////////////////////////////////
static const char k_szNoBitmapInClipboard[]      = "No bitmap in clipboard";
static const char k_szOpenClipboardFailed[]      = "OpenClipboard failed";
static const char k_szGetClipboardDataFailed[]   = "Fake bitmap data in clipboard";
static const char k_szCreateCompatibleDCFailed[] = "CreateCompatibleDC failed";
static const char k_szSelectObjectFailed[]       = "SelectObject failed";
static const char k_szGetObjectFailed[]          = "GetObject failed";
static const char k_szCannotCreate32BitsDib[]    = "Cannot create 32 bits dib";

static const char k_szNotEnoughMemory[]        = "Not enough memory";
static const char k_szCannotRetrieveTempPath[] = "Cannot retrieve temp path";
static const char k_szCustomDirIsEmpty[]       = "Screenshots directory is empty";
static const char k_szCustomDirIsEmpty2[]      = "Choose one with \"Screenshots options...\" in the context menu";
static const char k_szFileAlreadyExists[]      = "Screenshot auto filename already exists";
static const char k_szDirectoryDoesNotExists[] = "Directory does not exist: ";
static const char k_szCannotOpenOutput[]       = "Cannot open screenshot output file, check screenshots directory";
static const char k_szErrorWhileDumping[]      = "Error while dumping screenshot";
///////////////////////////////////////////////////////////////////////////////

BmpClipboardDumper::BmpClipboardDumper()
{
	m_bAbort = false;
}

BmpClipboardDumper::~BmpClipboardDumper()
{

}

// Gets the bitmap data from the clipboard and put it into the dib argument
static bool PrepareDib(chuwin32::DibBitmap& dib, String& strErr)
{
	if( !IsClipboardFormatAvailable(CF_BITMAP) )
	{
		strErr = k_szNoBitmapInClipboard;
		return false;
	}

	if( !OpenClipboard(NULL) )
	{
		strErr = k_szOpenClipboardFailed;
		return false;
	}

	// I don't like this kind of code indentation but there are too much resource alloc/desalloc 
	// function calls to use the "quit as soon as possible upon error" strategy.
	// We test everything in a paranoid mode as sometimes you get a weird bitmap in the clipboard
	// that you cannot copy (like a MSN messenger smiley copied from the messages window)

	bool bResult = false;

	HBITMAP hBitmap = (HBITMAP) GetClipboardData(CF_BITMAP);
	if( hBitmap != NULL )
	{
		// <CreateCompatibleDC>
		HDC hdcSrc = CreateCompatibleDC(NULL);
		if( hdcSrc != NULL )
		{
			// <SelectObject>
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcSrc, hBitmap);
			if( hOldBitmap != NULL )
			{
				BITMAP bm;
				Memory::Zero(&bm, sizeof(bm));
				const int nByteStored = GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
				if( nByteStored > 0 )
				{
					int32 nWidth = bm.bmWidth;
					int32 nHeight = bm.bmHeight;

					if( dib.Create(NULL, nWidth, nHeight) )
					{
						HDC hdcMem = dib.m_hDC;
						BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
						
						bResult = true; // Success !
					}
					else
					{
						strErr = k_szCannotCreate32BitsDib;
					}
				}
				else
				{
					strErr = k_szGetObjectFailed;
				}

				// </SelectObject>
				SelectObject(hdcSrc, hOldBitmap);
			}
			else
			{
				strErr = k_szSelectObjectFailed;
			}
			
			// </CreateCompatibleDC>
			DeleteDC(hdcSrc);
		}
		else
		{
			strErr = k_szCreateCompatibleDCFailed;
		}
	}
	else
	{
		strErr = k_szGetClipboardDataFailed;
	}

	CloseClipboard();

	return bResult;
}

bool BmpClipboardDumper::Dump(POEngine* pEngine)
{
	m_strErr.Empty();
	m_bAbort = false;

	chuwin32::DibBitmap dib;
	if( !PrepareDib(dib, m_strErr) )
	{
		return false;
	}
				
	uint32* const pSrc = dib.m_pBits;
	const int32 nWidth = dib.GetWidth();
	const int32 nHeight = dib.GetHeight();
	const int32 nPixelCount = nWidth * nHeight;

	Buffer buf24;
	if( !buf24.SetSize(nPixelCount * 3) )
	{
		m_strErr = k_szNotEnoughMemory;
		return false;
	}

	uint8* const pBuf24 = buf24.GetWritePtr();
	uint8* pBufTmp24 = pBuf24;

	for(int32 i = 0; i < nPixelCount; ++i)
	{
		uint32 nSrc = pSrc[i];
		uint8 r = uint8( (nSrc >> 16) & 0x000000ff);
		uint8 g = uint8( (nSrc >> 8) & 0x000000ff);
		uint8 b = uint8( (nSrc >> 0) & 0x000000ff);

		pBufTmp24[0] = r;
		pBufTmp24[1] = g;
		pBufTmp24[2] = b;
		pBufTmp24 += 3;
	}

	PngDumpData dd;
	dd.pixels = buf24;
	dd.width = nWidth;
	dd.height = nHeight;
	dd.pixelFormat = PF_24bppRgb;
	
	// Determine the file name
	DateTime date = DateTime::GetNow();
	String strDate = date.FormatDate();
	String strTime = date.FormatTime();

	String strFileNameBase = L"shot-" + date.Format(DateFormat::Compact);

	String strDestDir;
	if( m_settings.useDefaultDir )
	{
		strDestDir = GetTempDir();
		if( strDestDir.IsEmpty() )
		{
			// Cannot get the temp directory. I wonder when this can happen (maybe a bad registry settings ?)
			// Anyway, we inform the user
			m_strErr = k_szCannotRetrieveTempPath;
			return false;
		}
	}
	else
	{
		if( m_settings.customDir.IsEmpty() )
		{
			m_strErr = k_szCustomDirIsEmpty;
			m_strErr = m_strErr + L"\n";
			m_strErr = m_strErr + k_szCustomDirIsEmpty2;
			return false;
		}
		strDestDir = m_settings.customDir;
	}
	strDestDir = FilePath::AddSeparator(strDestDir);

	String strFileNameFinal = strFileNameBase + L".png";
	String strTotalFinalPath = FilePath::Combine(strDestDir, strFileNameFinal);

	// Find a unique name
	// With the date/time in the name, there is very little chance to get twice the same name,
	// but when pasting very fast, one second of delay might be not enough
	if( File::Exists(strDestDir + strFileNameFinal) )
	{
		// Oops
		String strCountW;

		// 100 tries max, oterwise er... 100 dump in less than one second ?
		int32 iCount = 1;
		const int32 nMaxCount = 100;

		for(; iCount <= nMaxCount; ++iCount)
		{
			strCountW = L"(" + String::FromInt(iCount) + L")";
			strFileNameFinal = strFileNameBase + strCountW + L".png";

			if( !File::Exists(strDestDir + strFileNameFinal) )
			{
				// Found ! ^^
				break;
			}
		}

		if( iCount == nMaxCount )
		{
			// Warn the user anyway that him and his 8 THz computer are too fast
			m_strErr = k_szFileAlreadyExists;
			return false;
		}
	}

	m_strFileName = strFileNameFinal;
	
	if( m_settings.askForFileName )
	{
		// Give the opportunity to the user to change the file name
		DumpStateChanged.Fire(DS_AskForFileName);

		if( m_bAbort )
		{
			// Aborted
			return false;
		}

		strFileNameFinal = m_strFileName;
	}
	
	if( !Directory::Exists(strDestDir) )
	{
		m_strErr = k_szDirectoryDoesNotExists + strDestDir;
		return false;
	}

	strTotalFinalPath = FilePath::Combine(strDestDir, strFileNameFinal);
	m_strFilePath = strTotalFinalPath;

	DumpStateChanged.Fire(DS_Creating);

	bool bResult = false;

	if( m_settings.maximizeCompression )
	{
		bResult = pEngine->OptimizeExternalBuffer(dd, strTotalFinalPath);
		if( !bResult )
		{
			m_strErr = pEngine->GetLastErrorString();
		}
	}
	else
	{
		File fileOut;
		if( !fileOut.Open(strTotalFinalPath, File::modeWrite) )
		{
			m_strErr = k_szCannotOpenOutput;
		}
		else
		{
			PngDumpSettings ds;
			ds.zlibCompressionLevel = 6;
			if( !PngDumper::Dump(fileOut, dd, ds) )
			{
				m_strErr = k_szErrorWhileDumping;
			}
			else
			{
				bResult = true;
			}
		}
	}
	
	if( bResult )
	{
		m_strPreviousFileName = m_strFileName;
	}
	return bResult;
}

String BmpClipboardDumper::GetLastError() const
{
	return m_strErr;
}

bool BmpClipboardDumper::IsBmpAvailable() const
{
	return IsClipboardFormatAvailable(CF_BITMAP) != FALSE;
}

String BmpClipboardDumper::GetTempDir() const
{
	String strDir;

	DWORD nTempPathBufferSize = GetTempPathW(0, NULL);
	if( nTempPathBufferSize > 0 )
	{
		::GetTempPathW(nTempPathBufferSize, strDir.GetUnsafeBuffer(nTempPathBufferSize - 1));	
	}
	return strDir;
}

String BmpClipboardDumper::GetDir() const
{
	if( m_settings.useDefaultDir )
	{
		return GetTempDir();
	}
	return m_settings.customDir;
}

String BmpClipboardDumper::GetPreviousFileName() const
{
	return m_strPreviousFileName;
}

String BmpClipboardDumper::GetFileName() const
{
	return m_strFileName;
}

String BmpClipboardDumper::GetFilePath() const
{
	return m_strFilePath;
}

void BmpClipboardDumper::SetFileName(const String& strFileName)
{
	m_strFileName = strFileName;
}

void BmpClipboardDumper::Abort()
{
	m_bAbort = true;
}
