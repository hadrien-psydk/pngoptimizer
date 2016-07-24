/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
/////////////////////////////////////////////////////////////////////////////////////
#ifndef POENG_POWORKERTHREAD_H
#define POENG_POWORKERTHREAD_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Use to perform a threaded asynchronous optimization.
class POWorkerThread
{
public:
	bool Create();
	bool Begin(int jobType, const PngDumpData* pPdd);
	void Wait();
	bool Succeeded() const;
	DynamicMemoryFile& GetResult();

	POWorkerThread();
	~POWorkerThread();

private:
	Thread    m_thread;        // The thread that works and waits for a command
	Semaphore m_semBegin;      // Incremented by the caller to notify the thread that it needs to work
	Semaphore m_semWait;       // Incremented by the thread to notify it finished working
	int  m_jobType;            // Parameter for the thread (type of optimization)
	bool m_success;            // Work status when the thread finished
	bool m_created;            // true if Create() was called
	DynamicMemoryFile m_dmf;   // Work result buffer
	const PngDumpData* m_pPdd; // Parameter for the thread (image data)
private:
	static int ThreadProcStatic(void*);
	int ThreadProc();
	bool DoJob();
	void Exit();
};

#endif
