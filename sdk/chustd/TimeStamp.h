///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_TIMESTAMP_H
#define CHUSTD_TIMESTAMP_H

namespace chustd
{
	void SplitChuTimeStamp(int64 stamp, uint16& year, uint8& month, uint8& day, 
	                       uint8& hour, uint8& minute, uint8& second, uint16& millisecond);

	int64 MakeChuTimeStamp(uint16 year, uint8 month, uint8 day, 
	                       uint8 hour, uint8 minute, uint8 second, uint16 millisecond);
}

#endif // CHUSTD_TIMESTAMP_H
