///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_CODEPOINT_H
#define CHUSTD_CODEPOINT_H

namespace chustd {

class CodePoint
{
public:
	static bool IsWhitespace(int codePoint);
	static bool IsBaseChar(int codePoint);
	static bool IsIdeographic(int codePoint);
	static bool IsLetter(int codePoint) { return IsBaseChar(codePoint) || IsIdeographic(codePoint); }

	static bool IsDigit(int codePoint);
	static bool IsCombiningChar(int codePoint);
	static bool IsExtenderChar(int codePoint);
};

} // namespace chustd

#endif
