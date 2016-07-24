///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Wave.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;
using namespace chuwin32;
//////////////////////////////////////////////////////////////////////

Wave::Wave()
{
	m_hWaveOut = 0;
	m_pWaveBuffer = 0;
	m_bPlaying = false;
	m_pWaveBuffer = new int16[44100 * 10];

	m_wfx.nChannels			= 1;

	/////////////////////////////////////////////////////////////////////
	HMODULE hm = LoadLibraryA("winmm");
	
	/*
	static const uint8 aOrdinals[] =
	{
		188, // waveOutOpen
			190, // waveOutPrepareHeader
			197, // waveOutWrite
			191, // waveOutReset
			196, // waveOutUnprepareHeader
			176  // waveOutClose
	};
	
	void** ppfn = (void**) &m_pfnWaveOutOpen;
	uint8 i = 6;
	do
	{
		i--;
		ppfn[i] = GetProcAddress(hm, MAKEINTRESOURCE( aOrdinals[i]));
	}while( i != 0);*/

	static const char* aszNames[] =
	{
		"waveOutOpen",
		"waveOutPrepareHeader",
		"waveOutWrite",
		"waveOutReset",
		"waveOutUnprepareHeader",
		"waveOutClose"
	};
	
	void** ppfn = (void**) &m_pfnWaveOutOpen;
	uint8 i = 6;
	do
	{
		i--;
		ppfn[i] = GetProcAddress(hm, aszNames[i]);
	}while( i != 0);

}

Wave::~Wave()
{
	if( m_hWaveOut != 0 )
	{
		MMRESULT res;
		
		res = m_pfnWaveOutReset(m_hWaveOut);
		res = m_pfnWaveOutUnprepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR));      
		res = m_pfnWaveOutClose(m_hWaveOut);
	}
	
	delete[] m_pWaveBuffer;
}

void Wave::SetProperties(int nSampleCount, bool bStereo)
{
	Stop();

	if( bStereo )
		m_wfx.nChannels	= 2;
	else
		m_wfx.nChannels	= 1;

	m_nWaveSize = nSampleCount * 2 * m_wfx.nChannels;
}

void Wave::SetBuffer(int16* pBuffer, int nCount, bool bStereo)
{
	Stop();

	if( bStereo )
		m_wfx.nChannels	= 2;
	else
		m_wfx.nChannels	= 1;

	m_nWaveSize = nCount * 2 * m_wfx.nChannels;
	chustd::Memory::Copy(m_pWaveBuffer, pBuffer, m_nWaveSize);
}

void Wave::Stop()
{
	if( m_bPlaying )
	{
		m_pfnWaveOutReset(m_hWaveOut);
		m_pfnWaveOutUnprepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR));      
		m_pfnWaveOutClose(m_hWaveOut);
		m_bPlaying = false;
	}
}

void Wave::Play(bool bLoop)
{
	Stop();

	m_wfx.wFormatTag		= WAVE_FORMAT_PCM;
	m_wfx.wBitsPerSample	= 16; 
	m_wfx.nBlockAlign		= m_wfx.nChannels * m_wfx.wBitsPerSample / 8;
	m_wfx.nSamplesPerSec	= 44100; 
	m_wfx.nAvgBytesPerSec	= m_wfx.nSamplesPerSec * m_wfx.nBlockAlign; 
	m_wfx.cbSize			= 0;

	MMRESULT res;
	res = m_pfnWaveOutOpen(&m_hWaveOut, WAVE_MAPPER, (WAVEFORMATEX*)&m_wfx, 0, 0, 0);

	m_wh.lpData = (LPSTR) m_pWaveBuffer;
	m_wh.dwBufferLength = m_nWaveSize;
	m_wh.dwBytesRecorded = 0;
	m_wh.dwFlags = bLoop ? (WHDR_BEGINLOOP | WHDR_ENDLOOP) : 0;
	m_wh.dwLoops = DWORD(-1);
	
	res = m_pfnWaveOutPrepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR));
	res = m_pfnWaveOutWrite(m_hWaveOut, &m_wh, sizeof(WAVEHDR));

	m_bPlaying = true;
}

void Wave::Dump(const String& strFilePath)
{
	File file;
	if( !file.Open(strFilePath, chustd::File::modeWrite | chustd::File::modeLittleEndian) )
		return;

	uint32 nHeaderSize = 16;
	uint32 nFileSize = uint32(16 + m_nWaveSize);

	uint16 nBytesPerSample = (m_wfx.wBitsPerSample / 8);

	file.Write("RIFF", 4);
	file.Write32(nFileSize);
	file.Write("WAVE", 4);
	file.Write("fmt ", 4);
	file.Write32(nHeaderSize);
	file.Write16(m_wfx.wFormatTag);
	file.Write16(m_wfx.nChannels);
	file.Write32(m_wfx.nSamplesPerSec);
	file.Write32(m_wfx.nAvgBytesPerSec);
	file.Write16(nBytesPerSample);
	file.Write16(m_wfx.wBitsPerSample);
	file.Write("data", 4);
	file.Write32(m_nWaveSize);
	
	file.Write(m_pWaveBuffer, m_nWaveSize);
	
	file.Close();
}

void Wave::Load(const String& strFilePath)
{
	File file;
	if( !file.Open(strFilePath, chustd::File::modeRead | chustd::File::modeLittleEndian) )
		return;

	char szRIFF[5] = {0, 0, 0, 0, 0};
	char szWAVE[5] = {0, 0, 0, 0, 0};
	char szFMT[5] = {0, 0, 0, 0, 0};
	char szDATA[5] = {0, 0, 0, 0, 0};

	uint32 nHeaderSize = 0;
	uint32 nFileSize = 0;
	uint16 wBytesPerSample = 0;

	file.Read(szRIFF, 4);
	file.Read32(nFileSize);
	file.Read(szWAVE, 4);
	file.Read(szFMT, 4);
	file.Read32(nHeaderSize);
	file.Read16(m_wfx.wFormatTag);
	file.Read16(m_wfx.nChannels);
	file.Read32(m_wfx.nSamplesPerSec);
	file.Read32(m_wfx.nAvgBytesPerSec);
	file.Read16(wBytesPerSample);
	file.Read16(m_wfx.wBitsPerSample);
	file.Read(szDATA, 4);
	file.Read32(m_nWaveSize);
	
	delete[] m_pWaveBuffer;
	m_pWaveBuffer = new uint8[m_nWaveSize];
	file.Read(m_pWaveBuffer, m_nWaveSize);
	
	file.Close();
}

int Wave::GetSampleCount() const
{
	if( m_wfx.wBitsPerSample == 8 )
		return m_nWaveSize;

	return m_nWaveSize / 2;
}

void* Wave::GetBuffer(bool& b16Bits) const
{
	if( m_wfx.wBitsPerSample == 8 )
	{
		b16Bits = false;
	}
	else
	{
		b16Bits = true;
	}

	return m_pWaveBuffer;
}