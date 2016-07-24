///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeflateStream.h"
#include "Memory.h"

#include "File.h"
#include "StringBuilder.h"
#include "Console.h"

// inflate / deflate (PNG)
#include "zlib/zlib.h"

namespace chustd {

//////////////////////////////////////////////////////////////////////

class DeflateStreamImpl
{
public:
	z_stream m_ZLibStream;
};
}

static void* chustd_zalloc(voidpf /*opaque*/, uInt items, uInt size)
{
	return chustd::Memory::Alloc(items * size);
}

static void chustd_zfree(voidpf /*opaque*/, voidpf address)
{
	chustd::Memory::Free(address);
}

//////////////////////////////////////////////////////////////////////
using namespace chustd;
//////////////////////////////////////////////////////////////////////

DeflateStream::DeflateStream()
{
	m_pImpl = new DeflateStreamImpl;
	Memory::Zero(&m_pImpl->m_ZLibStream, sizeof(m_pImpl->m_ZLibStream));
	m_pImpl->m_ZLibStream.zalloc = &chustd_zalloc;
	m_pImpl->m_ZLibStream.zfree = &chustd_zfree;
}

DeflateStream::~DeflateStream()
{
	delete m_pImpl;
}

void DeflateStream::SetBuffers(const uint8* pInNext, uint32 nInAvailable, uint8* pOutNext, uint32 outAvailable)
{
	m_pImpl->m_ZLibStream.next_in = (unsigned char*) pInNext;
	m_pImpl->m_ZLibStream.avail_in = nInAvailable;
	
	m_pImpl->m_ZLibStream.next_out = (unsigned char*) pOutNext;
	m_pImpl->m_ZLibStream.avail_out = outAvailable;
}

uint32 DeflateStream::GetOutAvailable() const
{
	return m_pImpl->m_ZLibStream.avail_out;
}

uint32 DeflateStream::GetOutTotalRead() const
{
	return m_pImpl->m_ZLibStream.total_out;
}

const char* DeflateStream::GetLastError() const
{
	if( m_pImpl == null )
	{
		return "";
	}
	
	const char* psz = m_pImpl->m_ZLibStream.msg;
	if( psz == null )
	{
		return "";
	}
	return psz;
}

