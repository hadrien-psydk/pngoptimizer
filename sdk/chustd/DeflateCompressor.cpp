///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeflateCompressor.h"
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

DeflateRet DeflateCompressor::Init(int level)
{
	return (DeflateRet)deflateInit(&m_pImpl->m_ZLibStream, level);
}
DeflateRet DeflateCompressor::Init2(int level, DeflateMethod method, int windowBits, int memLevel, DeflateStrategy strategy)
{
	return (DeflateRet)deflateInit2(&m_pImpl->m_ZLibStream, level, method, windowBits, memLevel, strategy);
}
DeflateRet DeflateCompressor::Compress(DeflateFlush flush)
{
	return (DeflateRet)deflate(&m_pImpl->m_ZLibStream, flush);
}
DeflateRet DeflateCompressor::Params(int newLevel, int strategy)
{
	return (DeflateRet)deflateParams(&m_pImpl->m_ZLibStream, newLevel, strategy);
}
DeflateRet DeflateCompressor::End()
{
	return (DeflateRet)deflateEnd(&m_pImpl->m_ZLibStream);
}

DeflateRet DeflateCompressor::SetDictionary(const uint8* pDictionary, uint32 dictLength)
{
	return (DeflateRet)deflateSetDictionary(&m_pImpl->m_ZLibStream, (const unsigned char*) pDictionary, dictLength);
}
DeflateRet DeflateCompressor::Reset()
{
	return (DeflateRet)deflateReset(&m_pImpl->m_ZLibStream);
}

uint32 DeflateCompressor::Bound(uint32 sourceLength)
{
	return deflateBound(0, sourceLength);
}
