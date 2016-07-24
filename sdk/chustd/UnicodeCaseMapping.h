///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

namespace chustd {

class UnicodeCaseMapping
{
public:
	static const uint16* const * GetUcs2ToUpperPages();
	static const uint16* const * GetUcs2ToLowerPages();

	static const uint32* GetUcs4ToUpperPage10428();
	static const uint32* GetUcs4ToLowerPage10400();

	static const uint16* CodeUnitToLowerMulti(uint16 code);
	static const uint16* CodeUnitToUpperMulti(uint16 code);
};

} // namespace chustd
