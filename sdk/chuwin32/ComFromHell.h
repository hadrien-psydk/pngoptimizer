///////////////////////////////////////////////////////////////////////////////
// This file is part of the chuwin32 library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chuwin32.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUWIN32_COMFROMHELL_H
#define CHUWIN32_COMFROMHELL_H

namespace chuwin32 {

class ComFromHell  
{
public:
	static bool Equals(const IID& iid1, const IID& iid2)
	{
		return chustd::Memory::Equals(&iid1, &iid2, sizeof(IID));
	}
};

} // namespace chuwin32

#endif // ndef CHUWIN32_COMFROMHELL_H
