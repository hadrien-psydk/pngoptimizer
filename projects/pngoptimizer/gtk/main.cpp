///////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizerGtk application
// Copyright (C) 2002/2014 Hadrien Nilsson - psydk.org
//
// PngOptimizerGtk is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Foobar is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Foobar; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POApplication.h"

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	POApplication app;
	if( !app.Init(argc, argv) )
	{
		return 1;
	}
	return app.Run();
	return 0;
}
