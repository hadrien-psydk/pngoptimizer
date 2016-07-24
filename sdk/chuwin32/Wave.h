///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_WAVE_H
#define CHUWIN32_WAVE_H

namespace chuwin32 {

/////////////////////////////////////////////////////////////////////////////////////////////////////
typedef MMRESULT (WINAPI*PFN_waveOutOpen) (LPHWAVEOUT phwo, UINT uDeviceID,
    LPCWAVEFORMATEX pwfx, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
typedef MMRESULT (WINAPI*PFN_waveOutPrepareHeader) (HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
typedef MMRESULT (WINAPI*PFN_waveOutWrite) (HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
typedef MMRESULT (WINAPI*PFN_waveOutReset) (HWAVEOUT hwo);
typedef MMRESULT (WINAPI*PFN_waveOutUnprepareHeader) (HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
typedef MMRESULT (WINAPI*PFN_waveOutClose) (HWAVEOUT hwo);
/////////////////////////////////////////////////////////////////////////////////////////////////////

class Wave  
{
public:
	struct DkWAVEFORMATEX
	{
		uint16 wFormatTag;         // format type
		uint16 nChannels;          // number of channels (i.e. mono, stereo...)
		uint32 nSamplesPerSec;     // sample rate
		uint32 nAvgBytesPerSec;    // for buffer estimation
		uint16 nBlockAlign;        // block size of data
		uint16 wBitsPerSample;     // number of bits per sample of mono data
		uint16 cbSize;
	} m_wfx;

public:
	int GetSampleCount() const;
	void* GetBuffer(bool& b16Bits) const;

	void Load(const chustd::String& strFilePath);
	void Dump(const chustd::String& strFilePath);
	void SetBuffer(int16* pBuffer, int nCount, bool bStereo);
	void Play(bool bLoop);
	void Stop();

	void SetProperties(int nSampleCount, bool bStereo);

	Wave();
	virtual ~Wave();
private:
	int32 m_nWaveSize;
	void* m_pWaveBuffer;

	bool m_bPlaying;
	
	HWAVEOUT m_hWaveOut;
	WAVEHDR m_wh;
	
	// Ne pas modifier ce bloc
	PFN_waveOutOpen				m_pfnWaveOutOpen;
	PFN_waveOutPrepareHeader	m_pfnWaveOutPrepareHeader;
	PFN_waveOutWrite			m_pfnWaveOutWrite;
	PFN_waveOutReset			m_pfnWaveOutReset;
	PFN_waveOutUnprepareHeader	m_pfnWaveOutUnprepareHeader;
	PFN_waveOutClose			m_pfnWaveOutClose;
};

} // namespace chuwin32

#endif // ndef CHUWIN32_WAVE_H
