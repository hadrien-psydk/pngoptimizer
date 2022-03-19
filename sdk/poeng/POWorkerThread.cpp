/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POWorkerThread.h"

/////////////////////////////////////////////////////////////////////////////////////
POWorkerThread::POWorkerThread()
{
	m_jobType = -1;
	m_success = false;
	m_created = false;
	m_pPdd = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////
POWorkerThread::~POWorkerThread()
{
	Exit();
}

/////////////////////////////////////////////////////////////////////////////////////
int POWorkerThread::ThreadProcStatic(void* arg)
{
	POWorkerThread* that = (POWorkerThread*)arg;
	return that->ThreadProc();
}

/////////////////////////////////////////////////////////////////////////////////////
bool POWorkerThread::DoJob()
{
	const PngDumpData& dd = *m_pPdd;

	PngDumpSettings ds;
	ds.zlibCompressionLevel = 9;

	if( m_jobType == 0 )
	{
		// Test 1
		// - No filtering
		// - ZLib default strategy
		// - ZLib normal memory load
		ds.filtering = 0;
		ds.zlibStrategy = PngDumpSettings::zlibStrategyDefault;
		ds.zlibWindowBitsAndMem = 0;

		if( !PngDumper::Dump(m_dmf, *m_pPdd, ds) )
		{
			return false;
		}
	}
	else if( m_jobType == 1 )
	{
		// This can work for low bits per pixel
		int bitsPerPix = ImageFormat::SizeofPixelInBits(dd.pixelFormat);
		if( bitsPerPix <= 8 )
		{
			// Test 2
			// - No filtering
			// - ZLib default strategy
			// - ZLib low memory load (yes, sometimes it can increase compression ! I don't know why... ZLib master, anyone ?
			ds.filtering = 0;
			ds.zlibStrategy = PngDumpSettings::zlibStrategyDefault;
			ds.zlibWindowBitsAndMem = PngDumpSettings::zlibWindowBitsAndMemLow;
			if( !PngDumper::Dump(m_dmf, dd, ds) )
			{
				return false;
			}
		}
	}
	else if( m_jobType == 2 )
	{
		// Test 3
		// - Filtering
		// - ZLib filter strategy
		// - ZLib normal memory load
		ds.filtering = 1;
		ds.zlibStrategy = PngDumpSettings::zlibStrategyFilter;
		ds.zlibWindowBitsAndMem = 0;
		if( !PngDumper::Dump(m_dmf, dd, ds) )
		{
			return false;
		}
	}
	else if( m_jobType == 3 )
	{
		// Test 4
		// - Filtering
		// - ZLib default strategy
		// - ZLib normal memory load
		ds.filtering = 1;
		ds.zlibStrategy = PngDumpSettings::zlibStrategyDefault;
		ds.zlibWindowBitsAndMem = 0;
		if( !PngDumper::Dump(m_dmf, dd, ds) )
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
int POWorkerThread::ThreadProc()
{
	for(;;)
	{
		if( m_semBegin.Wait() != 0 )
		{
			break;
		}
		// Perform job
		if( m_jobType < 0 )
		{
			// Exit requested
			break;
		}
		m_success = DoJob();
		m_semWait.Increment();
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// Creates the working thread and related resources.
// Returns true upon success
bool POWorkerThread::Create()
{
	const int firstAlloc = 256 * 1024 - 64;
	m_dmf.Open(firstAlloc);
	if( !m_semBegin.Create() )
	{
		return false;
	}
	if( !m_semWait.Create() )
	{
		return false;
	}
	if( !m_thread.Start(&ThreadProcStatic, this) )
	{
		return false;
	}
	m_created = true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Begins working. Creates the worker if Create() was not previously called.
// [in] jobType   Kind of job to do. [0..3]
// [in] pPds      Input image to work on
// Returns true upon success. If success, a call to Wait() will be needed to get the result.
bool POWorkerThread::Begin(int jobType, const PngDumpData* pPds)
{
	if( !m_created && !Create() )
	{
		return false;
	}
	if( !(0 <= jobType && jobType <= 3) )
	{
		return false;
	}
	m_pPdd = pPds;
	m_jobType = jobType;
	m_success = false;
	m_dmf.SetPosition(0);
	m_semBegin.Increment();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Waits for a job to finish.
/////////////////////////////////////////////////////////////////////////////////////
void POWorkerThread::Wait()
{
	if( m_jobType < 0 )
	{
		return;
	}
	m_semWait.Wait();
	m_jobType = -1;
	m_pPdd = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////
// Asks the thread to exit if it is waiting for a command
/////////////////////////////////////////////////////////////////////////////////////
void POWorkerThread::Exit()
{
	if( !m_created )
	{
		return;
	}
	m_pPdd = nullptr;
	m_jobType = -1; // Ask for exit
	m_success = false;
	m_semBegin.Increment();
	m_thread.WaitForExit();
	m_semBegin.Close();
}

/////////////////////////////////////////////////////////////////////////////////////
// Gets the result status of a job.
// Returns true if the job was successful
bool POWorkerThread::Succeeded() const
{
	return m_success;
}

/////////////////////////////////////////////////////////////////////////////////////
// Gets the job product: an optimized PNG in a memory buffer.
DynamicMemoryFile& POWorkerThread::GetResult()
{
	return m_dmf;
}
