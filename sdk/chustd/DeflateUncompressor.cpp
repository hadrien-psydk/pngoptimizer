///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeflateUncompressor.h"
#include "Memory.h"

#include "File.h"
#include "StringBuilder.h"
#include "Console.h"

// inflate / deflate (PNG)
#include "zlib/zlib.h"

//////////////////////////////////////////////////////////////////////
namespace chustd {
class DeflateStreamImpl
{
public:
	z_stream m_ZLibStream;
};
}

//////////////////////////////////////////////////////////////////////
using namespace chustd;
//////////////////////////////////////////////////////////////////////
	
///////////////////////
// Uncompression
DeflateRet DeflateUncompressor::Init()
{
	return (DeflateRet)inflateInit(&m_pImpl->m_ZLibStream);
}
DeflateRet DeflateUncompressor::Init2(int windowBits)
{
	return (DeflateRet)inflateInit2(&m_pImpl->m_ZLibStream, windowBits);
}

DeflateRet DeflateUncompressor::Uncompress(DeflateFlush flush)
{
	return (DeflateRet)inflate(&m_pImpl->m_ZLibStream, flush);
}
DeflateRet DeflateUncompressor::End()
{
	return (DeflateRet)inflateEnd(&m_pImpl->m_ZLibStream);
}

DeflateRet DeflateUncompressor::SetDictionary(const uint8* pDictionary, uint32 dictLength)
{
	return (DeflateRet)inflateSetDictionary(&m_pImpl->m_ZLibStream, (const unsigned char*) pDictionary, dictLength);
}
DeflateRet DeflateUncompressor::Sync()
{
	return (DeflateRet)inflateSync(&m_pImpl->m_ZLibStream);
}
DeflateRet DeflateUncompressor::Reset()
{
	return (DeflateRet)inflateReset(&m_pImpl->m_ZLibStream);
}
DeflateRet DeflateUncompressor::SyncPoint()
{
	return (DeflateRet)inflateSyncPoint(&m_pImpl->m_ZLibStream);
}

int DeflateUncompressor::GetTotalOut() const
{
	return m_pImpl->m_ZLibStream.total_out;
}
