///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_DEFLATECOMPRESSOR_H
#define CHUSTD_DEFLATECOMPRESSOR_H

#include "DeflateStream.h"

namespace chustd {

// Compression method
enum DeflateMethod
{
	DF_METHOD_DEFLATED = 8
};

// Compression strategy
enum DeflateStrategy
{
	DF_STRATEGY_DEFAULT      = 0,
	DF_STRATEGY_FILTERED     = 1,
	DF_STRATEGY_HUFFMAN_ONLY = 2,
	DF_STRATEGY_RLE          = 3,
	DF_STRATEGY_FIXED        = 4
};

class DeflateCompressor : public DeflateStream
{
public:
	DeflateRet Init(int level); // Default compression is for level = 6
	DeflateRet Init2(int level, DeflateMethod method, int windowBits, int memLevel, DeflateStrategy strategy);

	DeflateRet Compress(DeflateFlush flush);
	DeflateRet Params(int newLevel, int strategy);
	DeflateRet End();

	DeflateRet SetDictionary(const uint8* pDictionary, uint32 dictLength);
	DeflateRet Reset();
	
	static uint32 Bound(uint32 sourceLength);

	// Compresses the source buffer into the destination buffer. The level
	// parameter has the same meaning as in deflateInit.  sourceLen is the byte
	// length of the source buffer. Upon entry, destLen is the total size of the
	// destination buffer, which must be at least 0.1% larger than sourceLen plus
	// 12 bytes. Upon exit, destLen is the actual size of the compressed buffer.

	// compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
	// memory, Z_BUF_ERROR if there was not enough room in the output buffer,
	// Z_STREAM_ERROR if the level parameter is invalid.
	static DeflateRet Compress(uint8* pDest, uint32* pDestLen, const uint8* pSource, uint32 sourceLen, int level);
};

} // namespace chustd

#endif // ndef CHUSTD_DEFLATECOMPRESSOR_H
