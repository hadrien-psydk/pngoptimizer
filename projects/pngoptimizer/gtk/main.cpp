// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POApplication.h"

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	POApplication app;
	if( !app.Init() )
	{
		return 1;
	}
	return app.Run(argc, argv);
}
