/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_BMPCDSETTINGS_H
#define PO_BMPCDSETTINGS_H

// BmpClipboardDumper class settings
struct BmpcdSettings
{
	bool   useDefaultDir;
	String customDir;
	bool   askForFileName;
	bool   maximizeCompression;

	BmpcdSettings()
	{
		useDefaultDir = true;

		// Note: only fires BmpClipboardDumper::DumpStateChanged, no dialog involved
		askForFileName = false;
		// Users feedback reveals that they expect this parameter to be true
		// (maximize compression when creating a screenshot)
		maximizeCompression = true;
	}
};

#endif
