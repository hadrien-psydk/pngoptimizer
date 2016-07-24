///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_DEFLATESTREAM_H
#define CHUSTD_DEFLATESTREAM_H

namespace chustd {

enum DeflateFlush
{
	DF_FLUSH_NONE    = 0,
	DF_FLUSH_PARTIAL = 1, // will be removed, use DF_SYNC_FLUSH instead
	DF_FLUSH_SYNC    = 2,
	DF_FLUSH_FULL    = 3,
	DF_FLUSH_FINISH  = 4,
	DF_FLUSH_BLOCK   = 5
};

// Return codes for the compression/decompression functions. Negative
// values are errors, positive values are used for special but normal events.
enum DeflateRet
{
	DF_RET_OK            =  0,
	DF_RET_STREAM_END    =  1,
	DF_RET_NEED_DICT     =  2,
	DF_RET_ERRNO         = -1,
	DF_RET_STREAM_ERROR  = -2,
	DF_RET_DATA_ERROR    = -3,
	DF_RET_MEM_ERROR     = -4,
	DF_RET_BUF_ERROR     = -5,
	DF_RET_VERSION_ERROR = -6
};

class DeflateStream
{
public:
	void SetBuffers(const uint8* pInNext, uint32 nInAvailable, uint8* pOutNext, uint32 outAvailable);
	uint32 GetOutAvailable() const;
	uint32 GetOutTotalRead() const;
	
	const char* GetLastError() const;

protected:
	class DeflateStreamImpl* m_pImpl;

protected:
	DeflateStream();
	~DeflateStream();
};

} // namespace chustd

#endif // ndef CHUSTD_DEFLATESTREAM_H
