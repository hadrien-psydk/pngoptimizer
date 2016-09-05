////////////////////////////////////////////////////////////////////////////////
//
// chustd.h : Main header of the chustd library
//
// Version : See "CHUSTD_VERSION" below
//  
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005/2016 ChuTeam
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the author be held liable for any damages arising from 
// the use of this software. Any use of this Software is at your own risk.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// ChuTeam credits :
// - Hadrien Nilsson ; hadrien.nilsson [at] psydk.org
// - Vincent Robert ; vincent.robert [at] genezys.net
// - Sylvain Audi ; audisylvain [at] hotmail.com
//
//

#ifndef CHUSTD_CHUSTD_H
#define CHUSTD_CHUSTD_H

#define CHUSTD_VERSION 232

#include "CppExtension.h"
#include "String.h"
#include "StringBuilder.h"
#include "CodePoint.h"
#include "FormatType.h"
#include "ScanType.h"
#include "XmlDocument.h"
#include "Random.h"
#include "Sort.h"
#include "DeflateCompressor.h"
#include "DeflateUncompressor.h"
#include "File.h"
#include "DynamicMemoryFile.h"
#include "StaticMemoryFile.h"
#include "StdFile.h"
#include "Console.h"
#include "TextEncoding.h"
#include "Directory.h"

#include "Png.h"
#include "PngDumper.h"
#include "Jpeg.h"
#include "Gif.h"
#include "Bmp.h"
#include "Tga.h"

#include "ImageLoader.h"

#include "Math.h"
#include "DateTime.h"

#include "Process.h"
#include "Thread.h"

#include "BitBuffer.h"
#include "CriticalSection.h"
#include "MemIniFile.h"
#include "System.h"
#include "events.h"
#include "Buffer.h"
#include "ArgvParser.h"

#include "Semaphore.h"

#endif // ndef CHUSTD_CHUSTD_H
