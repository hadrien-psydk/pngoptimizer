///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_DEFLATEUNCOMPRESSOR_H
#define CHUSTD_DEFLATEUNCOMPRESSOR_H

#include "DeflateStream.h"

namespace chustd {

class DeflateUncompressor : public DeflateStream
{
public:
	DeflateRet Init();
	DeflateRet Init2(int windowBits);

	DeflateRet Uncompress(DeflateFlush flush);
	DeflateRet End();

	DeflateRet SetDictionary(const uint8* pDictionary, uint32 dictLength);
	DeflateRet Sync();
	DeflateRet Reset();
	DeflateRet SyncPoint();

	int GetTotalOut() const;
};

} // namespace chustd

#endif // ndef CHUSTD_DEFLATEUNCOMPRESSOR_H
