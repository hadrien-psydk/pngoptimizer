///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_STRINGBUILDER_H
#define CHUSTD_STRINGBUILDER_H

#include "String.h"

namespace chustd {

class StringBuilder  
{
public:
	const StringBuilder& operator+=(const String& str);
	const StringBuilder& operator+=(int codePoint);
	
	String ToString() const;
	int32 GetLength() const;
	bool IsEmpty() const;
	void Empty();

	StringBuilder();
	~StringBuilder();

private:
	wchar m_szBuffer[64];
	int32 m_nNext;
	bool m_bSzBufferDone;
	String m_strBuffer;
};

} // namespace chustd

#endif // ndef CHUSTD_STRINGBUILDER_H
