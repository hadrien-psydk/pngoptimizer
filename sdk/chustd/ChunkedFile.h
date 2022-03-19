///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_CHUNKEDFILE_H
#define CHUSTD_CHUNKEDFILE_H

#include "IFile.h"

namespace chustd {

// Note: Chunk Format:
//   SIZEOF (uint32) (does not include the size of the 3 uint32 SIZEOF, NAME and CRC)
//   NAME (uint32)
//   data (...)
//   CRC (uint32)


// Manages a stream including a set of chunks
class ChunkedFile : public IFile
{
public:
	int32  m_chunkSize; // On begin read : size of the data of the current chunk
	uint32 m_chunkName; // On begin read : name of current chunk
	uint32 m_crc;       // On end read : CRC of the current chunk. On write : CRC being computed
public:

	///////////////////////////////////////////////////////////////////////
	// IFile implementation
	virtual bool  SetPosition(int64 offset, Whence eWhence = posBegin);
	virtual int64 GetPosition() const;
	virtual int64 GetSize();
	virtual int   Read(void* pBuffer, int size);
	virtual int   Write(const void* pBuffer, int size);
	virtual ByteOrder  GetByteOrder() const;
	virtual void  SetByteOrder(ByteOrder byteOrder);
	virtual void Close();
	///////////////////////////////////////////////////////////////////////

	// - READ MODE -
	// Starts reading a new chunk. Updates m_nSizeof and m_nChunk
	// Returns true if the SIZEOF and NAME could be read (8 bytes), false otherwise (no more data)
	// Usually called after the file header has been read
	bool BeginChunkRead();
	
	// Finalizes the reading of a chunk, ie jumping to next chunk if not enough data were read.
	// Resets m_nSizeof and m_nChunk to 0
	// It expects to find the CRC (4 bytes)
	// Returns true if the CRC could be read, false otherwise (too much data read or no more data)
	bool EndChunkRead();
	
	// Gets the number of usefull data bytes remaining in the chunk
	// "usefull data bytes" means the chunk bytes excluding the SIZEOF, NAME and CRC
	int32 GetRemainingLength() const;

	// - WRITE MODE -
	// Note: the CChunkData object does the job of writting the SIZEOF, NAME and CRC	
	
	// Tells to the object that the incoming data to be written will be included in
	// a new chunk
	bool BeginChunkWrite(uint32 chunkName);
	
	// Tells to the object that the current chunk is finished
	// Returns false if data could not be completely written in stream
	bool EndChunkWrite();

	// Computes a PNG like CRC from a buffer
	static void InitCrc(uint32& crc);
	static void UpdateCrc(uint32& crc, const void* pBuffer, int32 bufferSize);
	static void FinalizeCrc(uint32& crc);

	// Receives the real file here
	ChunkedFile(IFile& file);
	virtual ~ChunkedFile();

private:
	// Read/Write mode attribute
	int64  m_startAt;     // Position of the current reading/writing chunk
	
	// Write mode attributes
	bool   m_insideChunk; // Set to true by BeginChunkWrite, set to false by EndChunkWrite
	
	// Attached real file
	IFile* m_pFile;
};

} // namespace chustd

#endif // ndef CHUSTD_CHUNKEDFILE_H
