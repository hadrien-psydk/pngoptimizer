///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Array.h"
#include "Sort.h"

using namespace chustd;

ByteArrayBox::ByteArrayBox(uint8* pExternalBuffer, int size)
{
	m_paTs = pExternalBuffer;
	m_count = size;
	m_capacity = size;
}

ByteArrayBox::~ByteArrayBox()
{

}

const ByteArray& ByteArrayBox::ToByteArray() const
{
	return (const ByteArray&) *this;
}

void ByteArrayBox::Set(uint8* pExternalBuffer, int size)
{
	m_paTs = pExternalBuffer;
	m_count = size;
	m_capacity = size;
}

StringArray StringArray::Sort() const
{
	return chustd::Sort::SortStrings(*this);
}
