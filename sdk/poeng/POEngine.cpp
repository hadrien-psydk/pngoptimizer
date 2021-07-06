///////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "POEngine.h"
#include "PaletteTranslator.h"

///////////////////////////////////////////////////////////////////////////////
const char k_szCannotStartWorkerThreads[] = "Cannot start worker threads";
const char k_szCannotDumpTry[]            = "Cannot dump internal try, check available memory";
const char k_szCannotWriteOnDevice[]      = "Cannot write on device";
const char k_szCannotWriteUncomplete[]    = "Write uncomplete, device may be full";
const char k_szNotEnoughMemoryForInternalConversion[] = "Not enough memory for internal conversion";
const char k_szUnsupportedPixelFormat[]   = "Unsupported pixel format";

const char k_szCannotLoadFile[]             = "Cannot load file";
const char k_szFileTooLarge[]               = "File is too large";
const char k_szUnsupportedFileFormat[]      = "Unsupported file format";
const char k_szNotEnoughMemoryToConvertTo24Bits[] = "Not enough memory to convert to 24 bits";

const char k_szPathDoesNotExist[]        = "Path does not exist";
const char k_szFileIsReadOnly[]          = "File is read-only";
const char k_szCannotPerformBackup[]     = "Cannot perform backup, previous backup deletion failed";
const char k_szCannotPerformBackupRenameFailed[] = "Cannot perform backup, rename failed";
const char k_szNotEnoughMemoryToKeepOriginalFile[] = "Not enough memory to keep original file";
const char k_szCorruptedChunkStructure[] = "Corrupted chunk structure";

const char k_szPathIsNotAFile[] = "Not a file";
const char k_szUnsupportedFileType[] = "Unsupported file type";
const char k_szInvalidArgument[] = "Invalid argument";
const char k_szCannotSetFilePosition[] = "Cannot set file position";
const char k_szInternalError[] = "Internal error";

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Gets a buffer to use for next optimization. We always keep the best result
// (smallest file weight) and return the largest to be overwritten by a new
// optimization case.
DynamicMemoryFile& POEngine::ResultManager::GetCandidate()
{
	// Get the largest, except if 0
	int64 size0 = m_dmf0.GetPosition();
	int64 size1 = m_dmf1.GetPosition();

	if( size0 == 0 )
	{
		return m_dmf0;
	}

	if( size1 == 0 )
	{
		return m_dmf1;
	}

	// Favor slot 1 in case we have the same sizes.
	// The idea is to keep the original clean copy unmodified in slot 0
	// until we can do better
	DynamicMemoryFile& dmf = (size1 >= size0) ? m_dmf1 : m_dmf0;

	// Open the dynamic memory file with allocation performed

	// The -64 is to be gentle with the memory allocator that needs room for its headers
	const int32 firstAlloc = 512 * 1024 - 64;
	dmf.Open(firstAlloc);
	dmf.SetPosition(0);
	return dmf;
}

///////////////////////////////////////////////////////////////////////////////
// Gets the best result, i.e. the smallest result.
DynamicMemoryFile& POEngine::ResultManager::GetSmallest()
{
	// Get the smallest, except if 0
	int64 size0 = m_dmf0.GetPosition();
	int64 size1 = m_dmf1.GetPosition();

	if( size0 == 0 )
	{
		return m_dmf1;
	}

	if( size1 == 0 )
	{
		return m_dmf0;
	}
	// Favor slot 0 in case we get the same sizes.
	// The idea is to keep the original clean copy unmodified in slot 0
	// until we can do better
	DynamicMemoryFile& dmf = (size0 <= size1) ? m_dmf0 : m_dmf1;
	return dmf;
}

void POEngine::ResultManager::Reset()
{
	m_dmf0.SetPosition(0);
	m_dmf1.SetPosition(0);
}

///////////////////////////////////////////////////////////////////////////////
// Sets a value to 0 if negative, max_u16 if greater than max_u16
static uint16 ClampUInt16(int value)
{
	if( value < 0 )
		return 0;
	if( value > 65535 )
		return 65535;
	return static_cast<uint16>(value);
}

///////////////////////////////////////////////////////////////////////////////
POEngine::POEngine()
{
	// Default to false because of older versions of Windows
	// that cannot display some unicode symbols
	m_unicodeArrowEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////
POEngine::~POEngine()
{
}

///////////////////////////////////////////////////////////////////////////////
void POEngine::EnableUnicodeArrow()
{
	m_unicodeArrowEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////
// Checks if a palette is black&white
// [in]   pal         Palette to check
// [out]  shouldSwap  true if the palette colors 0 and 1 should be swapped
bool POEngine::IsBlackAndWhite(const Palette& pal, bool& shouldSwap)
{
	if( pal.m_count != 2 )
	{
		return false;
	}
	Color col0 = pal.m_colors[0];
	Color col1 = pal.m_colors[1];

	shouldSwap = false;

	if( col0.IsEqualRgb(Color::Black) && col1.IsEqualRgb(Color::White) )
	{
		return true;
	}
	if( col0.IsEqualRgb(Color::White) && col1.IsEqualRgb(Color::Black) )
	{
		shouldSwap = true;
		return true;
	}

	if( col0.GetAlpha() == 0 && col1.IsEqualRgb(Color::White) )
	{
		return true;
	}
	if( col0.GetAlpha() == 0 && col1.IsEqualRgb(Color::Black) )
	{
		shouldSwap = true;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Checks if a palette is made only of shades of grey
bool POEngine::IsGreyPalette(const Palette& pal)
{
	for(int32 i = 0; i < pal.m_count; ++i)
	{
		uint8 r, g, b;
		pal.m_colors[i].ToRgb(r, g, b);

		if( r != g || r != b || g != b )
		{
			return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Creates resources to avoid too much burden on first file
// To be called before any optimization function to avoid making the first call
// slower than other calls (first call will do the warmup if it is not done).
/////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::WarmUp()
{
	const int count = ARRAY_SIZE(m_workerThreads);
	for(int i = 0; i < count; ++i)
	{
		if( !m_workerThreads[i].Create() )
		{
			return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Performs several dumps of buffer in memory.
//
// [in] dd     Dump data
//
// Returns true upon sucess
/////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::PerformDumpTries(PngDumpData& dd)
{
	// Force a background color if necessary.
	dd.useBackgroundColor = false;
	if( dd.pixelFormat == PF_32bppRgba )
	{
		// Force or keep background if requested
		if( m_settings.bkgdOption == POChunk_Keep || m_settings.bkgdOption == POChunk_Force )
		{
			dd.useBackgroundColor = true;
		}
	}

	// Force frame control delay values if necessary
	if( m_settings.fctlOption == POChunk_Force )
	{
		// Clamp to be safe
		uint16 num = ClampUInt16(m_settings.fctlDelayNum);
		uint16 den = ClampUInt16(m_settings.fctlDelayDen);

		const int frameCount = dd.frames.GetSize();
		for(int iFrame = 0; iFrame < frameCount; ++iFrame)
		{
			ApngFrame* pFrame = dd.frames[iFrame];
			pFrame->m_fctl.delayFracNumerator = num;
			pFrame->m_fctl.delayFracDenominator = den;
		}
	}

	// We perform the dumps asynchronously with 4 threads

	const int beginCount = 4;
	int waitCount = beginCount;
	for(int i = 0; i < beginCount; ++i)
	{
		if( !m_workerThreads[i].Begin(i, &dd) )
		{
			waitCount = i;
			break;
		}
	}
	for(int i = 0; i < waitCount; ++i)
	{
		m_workerThreads[i].Wait();
	}
	// Check begin error
	if( waitCount != beginCount )
	{
		AddError(k_szCannotStartWorkerThreads);
		return false;
	}
	// Check job error
	for(int i = 0; i < waitCount; ++i)
	{
		if( !m_workerThreads[i].Succeeded() )
		{
			AddError(k_szCannotDumpTry);
			return false;
		}
	}
	// Find smallest result
	int64 smallest = MAX_INT64;
	int smallestIndex = -1;
	for(int i = 0; i < waitCount; ++i)
	{
		int64 resultSize = m_workerThreads[i].GetResult().GetPosition();

		// If resultSize is 0, it means the job type gave no result
		// Example: a job for 8 bpp on a 24 bpp image
		if( 0 < resultSize && resultSize < smallest )
		{
			smallest = resultSize;
			smallestIndex = i;
		}
	}

	if( smallestIndex >= 0 )
	{
		// Move result to result manager in case we come back to this function
		m_resultmgr.GetCandidate() = m_workerThreads[smallestIndex].GetResult();
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Changes and pack pixels if the original 8bpp pixel buffer can be converted to 1bpp black and white
// The palette must be sorted by alpha [0 to 255]
bool POEngine::TryToConvertIndexedToBlackAndWhite(PngDumpData& dd)
{
	if( dd.pixelFormat != PF_8bppIndexed )
	{
		// Case not managed
		return false;
	}

	// Maybe it's a black&white picture, in that case there is no need for a palette
	bool shouldSwap;
	if( !IsBlackAndWhite(dd.palette, shouldSwap) )
	{
		return false;
	}

	uint8 nAlpha0 = dd.palette.m_colors[0].GetAlpha();
	uint8 nAlpha1 = dd.palette.m_colors[1].GetAlpha();

	if( (nAlpha0 != 255 || nAlpha1 != 255) && m_settings.avoidGreyWithSimpleTransparency )
	{
		// A black&white picture with transparency, we stay in palette mode
		return false;
	}

	if( shouldSwap )
	{
		Color colTmp = dd.palette.m_colors[0];
		dd.palette.m_colors[0] = dd.palette.m_colors[1];
		dd.palette.m_colors[1] = colTmp;

		// Inversion : 0 => 1 & 1 => 0
		PaletteTranslator inverterTranslator;
		inverterTranslator.conv[0] = 1;
		inverterTranslator.conv[1] = 0;
		inverterTranslator.TranslateAll(dd);
	}

	// Check if there was transparency in this paletted image, and if it is
	// compatible with a pure black&white image with no palette

	// In grayscale, only one index can define an alpha of 0
	uint8 a0 = dd.palette.m_colors[0].GetAlpha();
	uint8 a1 = dd.palette.m_colors[1].GetAlpha();
	if( a0 == 255 && a1 == 255 )
	{
		// No transparency at all
		dd.useTransparentColor = false;
		dd.pixelFormat = PF_1bppGrayScale;
	}
	else if( a0 == 0 && a1 == 255 )
	{
		// Transparency for index 0
		dd.useTransparentColor = true;
		dd.tRNS.grey = 0;
		dd.pixelFormat = PF_1bppGrayScale;
	}
	else if( a0 == 255 && a1 == 0 )
	{
		// Transparency for index 1
		dd.useTransparentColor = true;
		dd.tRNS.grey = 1;
		dd.pixelFormat = PF_1bppGrayScale;
	}
	else
	{
		// Another configuration of transparency, we cannot keep the image in black&white
		// We stay in palette mode...
		return false;
	}

	// Pack the pixels according to the new pixel format (1bpp grey)
	const int frameCount = dd.frames.GetSize();

	// Pack the default image if any
	if( dd.hasDefaultImage || frameCount == 0 )
	{
		ImageFormat::PackPixels(dd.pixels, dd.width, dd.height, dd.pixelFormat);
	}

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		ApngFrame* pFrame = dd.frames[iFrame];

		const int width = pFrame->GetWidth();
		const int height = pFrame->GetHeight();
		ImageFormat::PackPixels(pFrame->m_pixels, width, height, dd.pixelFormat);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Finds an unused gray in a grayscale palette
// Returns -1 if all 256 levels are used
static int FindUnusedGray(const Palette& pal)
{
	// In this array, put a non-zero value when the grayscale value is used
	// When alpha is 0, we can consider that the level is unused
	uint8 used[256];
	memset(used, 0, sizeof(used));
	for(int i = 0; i < pal.m_count; ++i)
	{
		used[pal.m_colors[i].r] |= pal.m_colors[i].a;
	}

	// Select the first unused level
	int level = 0;
	for(; level < 256; ++level)
	{
		if( !used[level] )
		{
			break;
		}
	}
	if( level >= 256 )
	{
		return -1;
	}
	return level;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if the image can be converted to greyscale. Converts if possible.
// The palette must be sorted by alpha [0 to 255]
bool POEngine::TryToConvertIndexedToGreyscale(PngDumpData& dd)
{
	if( dd.pixelFormat != PF_8bppIndexed )
	{
		// Case not managed
		return false;
	}

	if( !IsGreyPalette(dd.palette) )
	{
		return false;
	}

	// The colors are ok, now check the alphas. We need no transparency at all
	// or only one fully transparent color
	uint8 alpha0 = dd.palette.m_colors[0].GetAlpha();
	uint8 alpha1 = dd.palette.m_colors[1].GetAlpha();

	if( alpha0 == 255 )
	{
		// Full opaque image
		dd.useTransparentColor = false;
	}
	else if( alpha0 == 0 && alpha1 == 255 )
	{
		// One single fully transparent color
		if( m_settings.avoidGreyWithSimpleTransparency )
		{
			// Keep palette per settings
			return false;
		}
		dd.useTransparentColor = true;

		// We may have the same exact gray in the palette, but with different
		// alphas. So we should not reuse the first palette color blindly.
		// Let's find a unused gray. Because there can only be 256 entries in
		// the palette, it means we have the guaranty to find at least one
		// unused gray.
		dd.tRNS.grey = static_cast<uint8>(FindUnusedGray(dd.palette));

		// Apply the special gray level for the first color in order to
		// get the correct result when converting indices at the
		// end of this function
		dd.palette.m_colors[0].SetRgb(dd.tRNS.grey, dd.tRNS.grey, dd.tRNS.grey);
	}
	else
	{
		// Multiple transparencies, must keep the palette
		return false;
	}
	dd.pixelFormat = PF_8bppGrayScale;

	// Now change the pixel so they represent a grey intensity instead of palette indexes
	PaletteTranslator greyTranslator;
	greyTranslator.BuildGreyscale(dd.palette);
	greyTranslator.TranslateAll(dd);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Computes the signature of a PNG. This function is called for the result of the engine, and
// thus any error in the PNG decoding is actually a programming error.
static bool ComputePngSignature(IFile& file, POEngine::PngSignature& sign)
{
	sign.Clear();

	if( !Png::IsPng(file) )
	{
		// Not a PNG
		ASSERT(0); // Programming error
		return false;
	}

	ChunkedFile chfIn(file);

	bool bIENDFound = false;

	while( !bIENDFound)
	{
		bool chunkPresent = chfIn.BeginChunkRead();
		if( !chunkPresent )
		{
			break;
		}

		switch( chfIn.m_chunkName )
		{
		case PngChunk_IEND::Name:
			bIENDFound = true;
			break;
		default:
			break;
		}

		if( !chfIn.EndChunkRead() )
		{
			// Unexpected end of stream, CRC not present or chunk mishandled (read too much)
			return false;
		}
		sign.Add(chfIn.m_crc);
	}

	if( !bIENDFound )
	{
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Dumps either to file or to memory the best optmization result.
//
// [in]  target    Structure containing the file or the memory buffer information to use for the dump
// [out] optiInfo  Receives the size of the dumped file
//
// Returns true upon success
/////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::DumpBestResultToFile(const OptiTarget& target, OptiInfo& optiInfo)
{
	DynamicMemoryFile& dmf = m_resultmgr.GetSmallest();
	int sizeToDump = static_cast<int>(dmf.GetPosition());

	if( target.type == OptiTarget::Type::Stdout )
	{
		StdFile file(StdFileType::Stdout);
		int32 written = file.Write(dmf.GetContent().GetReadPtr(), sizeToDump);
		if( written != sizeToDump )
		{
			AddError(k_szCannotWriteUncomplete);
			return false;
		}
		optiInfo.sizeAfter = sizeToDump;
	}
	else if( target.type == OptiTarget::Type::File )
	{
		// Target is a file
		ASSERT(!target.filePath.IsEmpty());

		// Check if overwritting is necessary. This is the case only for PNG files,
		// as a clean version was inserted and its signature was computed.
		//
		// We cannot just check the size because a clean version of the PNG file may
		// add data depending on the engine settings (for example, forcing a specifc chunk).
		//
		// Thus we need to compare the content, which is done by looking at the signatures.

		if( target.filePath == target.srcInfo.filePath
			&& target.srcInfo.fileSize == sizeToDump
		 	&& !optiInfo.srcSignature.IsEmpty() )
		{
			// Theses are the same path, and the source file is a PNG
			// So now verify the contents
			dmf.SetPosition(0);
			PngSignature newSignature;
			if( !ComputePngSignature(dmf, newSignature) )
			{
				// If it happens, it means there is a bug in the engine
				AddError(k_szInternalError);
				return false;
			}
			dmf.SetPosition(sizeToDump);

			if( newSignature == optiInfo.srcSignature )
			{
				optiInfo.sizeAfter = sizeToDump;
				optiInfo.sameContent = true;
				return true;
			}
		}

		File file;
		if( !file.Open(target.filePath, File::modeWrite) )
		{
			AddError(k_szCannotWriteOnDevice);
			return false;
		}

		int32 written = file.Write(dmf.GetContent().GetReadPtr(), sizeToDump);
		if( written != sizeToDump )
		{
			AddError(k_szCannotWriteUncomplete);
			return false;
		}
		optiInfo.sizeAfter = sizeToDump;

		// Write the same modification date than the original file.
		// This should be done just before closing, because Write()
		// will update the modification date too
		if( m_settings.keepFileDate )
		{
			file.SetLastWriteTime(m_originalFileWriteTime);
		}
		file.Close();
	}
	else
	{
		ASSERT(target.type == OptiTarget::Type::Memory);
		// Target is memory buffer
		if( target.buf == nullptr )
		{
			AddError(k_szInvalidArgument);
			return false;
		}
		if( sizeToDump > target.bufCapacity )
		{
			AddError(k_szCannotWriteUncomplete);
			return false;
		}
		memcpy(target.buf, dmf.GetContent().GetReadPtr(), sizeToDump);
		optiInfo.sizeAfter = sizeToDump;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Counts the colors of a 8bpp animation frame and adds it to an existing array of counts.
// [in]      pFrame   Animation frame which buffer is to be used to count colors
// [in,out]  pCounts  Array of color counts
static void AddFrameColorCounts(const ApngFrame* pFrame, uint32* pCounts)
{
	const int width = pFrame->GetWidth();
	const int height = pFrame->GetHeight();
	const int pixelCount = width * height;
	const uint8* pPixels = pFrame->m_pixels.GetReadPtr();
	for(int i = 0; i < pixelCount; ++i)
	{
		pCounts[ pPixels[i] ]++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Counts the total number of colors of a PNG, including animation frames.
// [in]   dd       PNG information
// [out]  pCounts  Colors counts. Must be an array of 256 values.
static void CountColors(const PngDumpData& dd, uint32* pCounts)
{
	ASSERT( dd.pixelFormat == PF_8bppIndexed );

	Memory::Zero32(pCounts, 256);

	const int frameCount = dd.frames.GetSize();

	if( dd.hasDefaultImage || frameCount == 0 )
	{
		const int pixelCount = dd.width * dd.height;
		const uint8* pPixels = dd.pixels.GetReadPtr();
		for(int i = 0; i < pixelCount; ++i)
		{
			pCounts[ pPixels[i] ]++;
		}
	}

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		const ApngFrame* pFrame = dd.frames[iFrame];
		AddFrameColorCounts(pFrame, pCounts);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes an image which pixel format is indexed (palette mode)
//
// [in,out]  dd  Image information, may be changed to a better format.
//
// Returns true upon success
/////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizePaletteMode(PngDumpData& dd)
{
	ASSERT( PF_1bppIndexed <= dd.pixelFormat && dd.pixelFormat <= PF_8bppIndexed);

	UnpackPixelFrames(dd);

	// Count the number of times a color appears
	uint32 colCounts[256];
	CountColors(dd, colCounts);

	// Remove holes in the palette and create the conversion table old index -> new index
	PaletteTranslator noHolesTranslator;
	noHolesTranslator.BuildUnusedColors(dd.palette, colCounts);

	// Remove duplicated colors
	PaletteTranslator dupTranslator;
	dupTranslator.BuildDuplicatedColors(dd.palette, colCounts);

	// Sort the palette according to an increasing alpha
	PaletteTranslator sortedAlphaTranslator;
	sortedAlphaTranslator.BuildSortAlpha(dd.palette, colCounts);

	// Change the pixel buffer so every index points on the new palette
	PaletteTranslator tmpTranslator = PaletteTranslator::Combine(noHolesTranslator, dupTranslator);
	PaletteTranslator finalTranslator = PaletteTranslator::Combine(tmpTranslator, sortedAlphaTranslator);
	finalTranslator.TranslateAll(dd);

	// Check if we can convert to Black&White
	if( TryToConvertIndexedToBlackAndWhite(dd) )
	{
		return PerformDumpTries(dd);
	}

	// Check if we can convert to greyscale
	if( TryToConvertIndexedToGreyscale(dd) )
	{
		return PerformDumpTries(dd);
	}

	// Some tries with different orders
	// To get a stable optimization, sort by luminance before sorting by population

	//////////////////////////
	PaletteTranslator luminanceTranslator;
	luminanceTranslator.BuildSortLuminance(dd.palette, colCounts);
	luminanceTranslator.TranslateAll(dd);
	PackPixelFrames(dd);
	if( !PerformDumpTries(dd) )
	{
		return false;
	}

	// Save CPU or Memory ? :-p
	UnpackPixelFrames(dd);

	//////////////////////////
	// Sometimes we get some best result by sorting twice
	PaletteTranslator populationTranslator;
	populationTranslator.BuildSortPopulation(dd.palette, colCounts);

	PaletteTranslator populationTranslator2;
	populationTranslator2.BuildSortPopulation(dd.palette, colCounts);

	PaletteTranslator finalPopulationTranslator = PaletteTranslator::Combine(populationTranslator, populationTranslator2);
	finalPopulationTranslator.TranslateAll(dd);

	PackPixelFrames(dd);
	if( !PerformDumpTries(dd) )
	{
		return false;
	}

	// We're done
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Find an unused color among the pixels
// Needs memory allocation, thus can fail
bool POEngine::FindUnusedColorHardcoreMethod(const uint8* pRgba, int32 pixelCount,
                                             uint8& nRed, uint8& nGreen, uint8& nBlue)
{
	ByteArray aSorted;
	if( !aSorted.SetSize(pixelCount * 4) )
	{
		return false;
	}

	ByteArray aSorted2;
	if( !aSorted2.SetSize(pixelCount * 4) )
	{
		return false;
	}

	uint32* pRgba32 = (uint32*) pRgba;

	// ByteSort function that works the same on both endian mode
	chustd::Sort::ByteSortLittleEndian( pRgba32, (uint32*)aSorted.GetPtr(), (uint32*)aSorted2.GetPtr(), pixelCount);

	// This first entry is pure black with alpha set to 0,
	// If we are in this function it's because it is not suitable so we start at 1

	if( k_ePlatformByteOrder == boBigEndian )
	{
		// Convert uint32 to little endian
		IFile::Swap32( (uint32*) aSorted.GetPtr(), pixelCount);
	}

	uint32* pSorted32 = (uint32*) aSorted.GetPtr();

	uint32 nA = 0x00000000; // ABGR in register
	for(int32 i = 1; i < pixelCount; ++i)
	{
		uint32 nB = (0x00ffffff & pSorted32[i]);
		uint32 nDif = nB - nA;
		if( nDif > 1 )
		{
			// Found a useable range of colors to be used as transparent
			break;
		}
		nA = nB; // Next entry
	}

	if( nA == 0x00ffffff )
	{
		// This can only occurs if all 16777216 colors are present in the picture
		// Like a 4096*4096 picture
		return false;
	}

	uint32 nColor = nA + 1;

	nRed = uint8(nColor & 0x000000ff);
	nGreen = uint8((nColor & 0x0000ff00) >> 8);
	nBlue = uint8((nColor & 0x00ff0000) >> 16);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Finds an unused color among the pixels
//
// [in]  rbRgb                Pixel buffer
// [out] nRed, nGreen, nBlue  Color not present in the pixel buffer
//
// Returns true upon success
/////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::FindUnusedColor(const Buffer& aRgb, uint8& nRed, uint8& nGreen, uint8& nBlue)
{
	// Check with some candidates
	FixArray<Color, 7> aCandidates;
	aCandidates[0].SetRgb(1, 0, 0);
	aCandidates[1].SetRgb(1, 1, 0);
	aCandidates[2].SetRgb(1, 1, 1);
	aCandidates[3].SetRgb(0, 1, 1);
	aCandidates[4].SetRgb(0, 0, 1);
	aCandidates[5].SetRgb(1, 0, 1);
	aCandidates[6].SetRgb(0, 1, 0);

	const int32 pixelCount = aRgb.GetSize() / 3;

	foreach(aCandidates, iCandidate)
	{
		bool bGoodCandidate = true;

		uint8 cr, cg, cb;
		aCandidates[iCandidate].ToRgb(cr, cg, cb);

		const uint8* pPixelsRgb = aRgb.GetReadPtr();
		for(int32 i = 0; i < pixelCount; ++i)
		{
			uint8 r = pPixelsRgb[0];
			uint8 g = pPixelsRgb[1];
			uint8 b = pPixelsRgb[2];

			if( r == cr && g == cg && b == cb )
			{
				// Found, not a good candidate
				bGoodCandidate = false;
				break;
			}

			pPixelsRgb += 3;
		}

		if( bGoodCandidate )
		{
			nRed = cr;
			nGreen = cg;
			nBlue = cb;
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// When a 32 bpp image actually uses a simple yes/no transparency, we want to check what color can
// be used as the transparent color in order to convert the image into 24 bpp. Black would be nice
// but we must ensure that it never appears in the image.
///////////////////////////////////////////////////////////////////////////////////////////////////
static bool CanBlackBeUsedAsTransparentColor32BppRGBX(const PngDumpData& dd)
{
	const int pixelCount = dd.width * dd.height;
	const uint8* pPixelsRgba = dd.pixels.GetReadPtr();

	for(int i = 0; i < pixelCount; ++i)
	{
		uint8 r = pPixelsRgba[0];
		uint8 g = pPixelsRgba[1];
		uint8 b = pPixelsRgba[2];
		uint8 a = pPixelsRgba[3];
		if( a == 255 )
		{
			if( r == 0 && g == 0 && b == 0 )
			{
				// Black is a used color, we cannot use it as a the transparent color
				return false;
			}
		}
		pPixelsRgba += 4;
	}
	// Black never appears as a visible color, we can use it
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::Optimize32BitsMode(PngDumpData& dd)
{
	ASSERT(dd.pixelFormat == PF_32bppRgba);

	const int32 width = dd.width;
	const int32 height = dd.height;
	const int32 pixelCount = width * height;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// First step : set to 0 every color which alpha is 0

	// It will allow a potential optimisation 32 bits --> palette mode
	// We set the fully transparent color to 0 as 4 bytes to 0 are nicely compressed

	uint8* pPixels8 = dd.pixels.GetWritePtr();

	for(int32 iPixel = 0; iPixel < pixelCount; ++iPixel)
	{
		// In memory : RGBA, RGBA, RGBA
		//             0123  0123  0123
		uint8 alpha = pPixels8[3];
		if( alpha == 0 )
		{
			// Alpha set to 0, set everything to 0
			uint32* pPixels32 = (uint32*) pPixels8;
			pPixels32[0] = 0x00000000;
		}
		pPixels8 += 4;
	}

	// Test 0
	// We save the result as-is, sometimes a small 32 bits image can be smaller than the same one in 24 or 8 bits
	if( pixelCount <= (64 * 64) )
	{
		// Our definition of "small" is 64*64
		if( !PerformDumpTries(dd) )
		{
			AddError(k_szCannotDumpTry);
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Verify the alpha channel is really necessary
	const uint8* pBuffer = dd.pixels.GetReadPtr();
	const uint8* pSrcBuffer = pBuffer;

	Buffer rbNewRgb; // 24 bits version of the image
	if( !rbNewRgb.SetSize(pixelCount * 3) )
	{
		AddError(k_szNotEnoughMemoryToConvertTo24Bits);
		return false;
	}
	uint8* pNewBuffer = rbNewRgb.GetWritePtr();

	int32 alphaCounts[256]; // To count how many times the same alpha appears in the image
	Memory::Zero32(alphaCounts, 256);

	for(int32 i = 0; i < pixelCount; ++i)
	{
		uint8 r = pSrcBuffer[0];
		uint8 g = pSrcBuffer[1];
		uint8 b = pSrcBuffer[2];
		uint8 alpha = pSrcBuffer[3];

		pNewBuffer[0] = r;
		pNewBuffer[1] = g;
		pNewBuffer[2] = b;

		alphaCounts[alpha] += 1;

		pSrcBuffer += 4;
		pNewBuffer += 3;
	}

	// All pixels are opaque, no need for an alpha channel, use the 24 bits optimizer
	if( alphaCounts[255] == pixelCount )
	{
		dd.pixels = rbNewRgb;
		dd.pixelFormat = PF_24bppRgb;
		return Optimize24BitsMode(dd);
	}

	// Ok, maybe we can keep the 24 bits buffer if every alpha is set to 255 except for one color

	if( (alphaCounts[0] + alphaCounts[255]) == pixelCount )
	{
		// Yes we can, but the color to be used to mean a pixel is totaly transparent must not appear
		// elsewhere the picture as being an opaque pixel.

		// It means we must choose a color not used in the picture, that's a bit tricky

		////////////////////////////////////////////////////
		// First, check if black is a used color

		bool bBlackAsTransparentColor = CanBlackBeUsedAsTransparentColor32BppRGBX(dd);
		////////////////////////////////////////////////////

		uint8 nTransRed = 0;
		uint8 nTransGreen = 0;
		uint8 nTransBlue = 0;

		bool bContinueIn24Bits = bBlackAsTransparentColor;

		if( !bBlackAsTransparentColor )
		{
			// Find an alternative transparent color
			if(  FindUnusedColor(rbNewRgb, nTransRed, nTransGreen, nTransBlue)
			  || FindUnusedColorHardcoreMethod(dd.pixels.GetReadPtr(), pixelCount, nTransRed, nTransGreen, nTransBlue) )
			{
				bContinueIn24Bits = true;

				// Ok, we have our color to be use for transparency :)
				// Convert every pixel to that color

				uint8* pPixelsRgb = rbNewRgb.GetWritePtr();
				const uint8* pPixelsRgba = dd.pixels.GetReadPtr();

				for(int32 i = 0; i < pixelCount; ++i)
				{
					uint8 nAlpha = pPixelsRgba[3];
					if( nAlpha == 0 )
					{
						// Put the transparent color
						pPixelsRgb[0] = nTransRed;
						pPixelsRgb[1] = nTransGreen;
						pPixelsRgb[2] = nTransBlue;
					}

					pPixelsRgba += 4;
					pPixelsRgb += 3;
				}
			}
		}

		if( bContinueIn24Bits )
		{
			// Now we have our 24 bits image + one color for transparency, continue with 24 bits optimization...
			dd.pixels = rbNewRgb;
			dd.pixelFormat = PF_24bppRgb;
			dd.useTransparentColor = true;
			dd.tRNS.red = nTransRed;
			dd.tRNS.green = nTransGreen;
			dd.tRNS.blue = nTransBlue;
			return Optimize24BitsMode(dd);
		}
	}

	///////////////////////////////////////////////////////////////////
	// Dump to memory

	//dd.pBuffer = pBuffer;
	dd.pixelFormat = PF_32bppRgba;
	return PerformDumpTries(dd);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes a 24 bits image
bool POEngine::Optimize24BitsMode(PngDumpData& dd)
{
	ASSERT(dd.pixelFormat == PF_24bppRgb);

	const uint8* pBuffer = dd.pixels.GetReadPtr();
	const int32 width = dd.width;
	const int32 height = dd.height;

	////////////////////////////////////////////////////////////////
	// Some test to check if we can switch to palette mode
	Palette palTest;
	palTest.m_count = 0;

	const uint8* pSrcBuffer = pBuffer;
	const int32 pixelCount = width * height;

	Buffer rbNew;
	if( !rbNew.SetSize(pixelCount) )
	{
		return false;
	}
	uint8* pNewBuffer = rbNew.GetWritePtr();

	bool bTooMuchColors = false;
	for(int32 i = 0; i < pixelCount; ++i)
	{
		uint8 r = pSrcBuffer[0];
		uint8 g = pSrcBuffer[1];
		uint8 b = pSrcBuffer[2];

		// Find the color in the palette
		int32 iCol = 0;
		for(; iCol < palTest.m_count; ++iCol)
		{
			const Color& col = palTest.m_colors[iCol];
			uint8 rp, gp, bp;
			col.ToRgb(rp, gp, bp);

			if( r == rp && g == gp && b == bp )
			{
				// Found !
				break;
			}
		}

		pNewBuffer[0] = uint8(iCol); // Fill the new buffer

		if( iCol == palTest.m_count )
		{
			// The 24 bit color was not in the palette

			// Add a new entry in the palette
			if( palTest.m_count == 256 )
			{
				// The palette is full
				bTooMuchColors = true;
				break;
			}

			// New entry in the palette
			palTest.m_colors[ palTest.m_count].SetRgb(r, g, b);
			palTest.m_count++;
		}

		pSrcBuffer += 3;
		pNewBuffer += 1;
	}

	// If the picture is very small, we may enlarge it if we switch to palette mode
	const int32 nMaxSizeOrigin = pixelCount * 3;
	const int32 nMaxSizeNew = pixelCount + palTest.m_count * 3 + 12; // 12 = min size chunk
	if( nMaxSizeOrigin < nMaxSizeNew )
	{
		bTooMuchColors = true;
	}

	///////////////////////////////////////////////////////////////////
	//dd.pBuffer = pBuffer;
	dd.pixelFormat = PF_24bppRgb;
	if( !PerformDumpTries(dd) )
	{
		return false;
	}

	///////////////////////////////////////////////////////////////////

	if( bTooMuchColors )
	{
		// Cannot convert to palette mode, we stop here
		return true;
	}

	// Add transparency to the palette
	if( dd.useTransparentColor )
	{
		// Find the transparent color
		int32 iCol = 0;
		for(; iCol < palTest.m_count; ++iCol)
		{
			const Color& col = palTest.m_colors[iCol];
			uint8 rp, gp, bp;
			col.ToRgb(rp, gp, bp);

			if( dd.tRNS.red == rp && dd.tRNS.green == gp && dd.tRNS.blue == bp )
			{
				// Aha ! :)
				palTest.m_colors[iCol].SetAlpha(0);
				break;
			}
		}
	}

	// Now we have a 8 bits indexed image, we can continue with the palette mode optimizer
	dd.pixels = rbNew;
	dd.palette = palTest;
	dd.pixelFormat = PF_8bppIndexed;
	return OptimizePaletteMode(dd);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeGrayScale(PngDumpData& dd)
{
	ASSERT( PF_1bppGrayScale <= dd.pixelFormat && dd.pixelFormat <= PF_16bppGrayScale );
	return PerformDumpTries(dd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the intensity value of the grey to use for transparency, -1 if no simple transparency
// is possible.
int POEngine::CanSimplifyGreyAlpha(const PngDumpData& dd) const
{
	ASSERT( PF_16bppGrayScaleAlpha == dd.pixelFormat );

	// Check if we can omit the alpha channel. Conditions:
	// alpha always opaque
	// alpha not opaque always fully transparent AND one greyscale value is never used or its alpha is always 0
	uint32 counts[256];
	Memory::Zero32(counts, 256);

	const int frameCount = dd.frames.GetSize();
	if( dd.hasDefaultImage || frameCount == 0 )
	{
		const int pixelCount = dd.width * dd.height;
		const uint8* pPixels = (uint8*)dd.pixels.GetReadPtr();
		for(int i = 0; i < pixelCount; ++i)
		{
			uint8 grey = pPixels[2*i];
			uint8 alpha = pPixels[2*i+1];
			if( !(alpha == 0 || alpha == 255) )
			{
				// Nop
				return -1;
			}
			if( alpha == 255 )
			{
				counts[ grey ]++;
			}
		}
	}

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		const ApngFrame* pFrame = dd.frames[iFrame];
		const int width = pFrame->GetWidth();
		const int height = pFrame->GetHeight();
		const int pixelCount = width * height;
		const uint8* pPixels = (uint8*)pFrame->m_pixels.GetReadPtr();
		for(int i = 0; i < pixelCount; ++i)
		{
			uint8 grey = pPixels[2*i];
			uint8 alpha = pPixels[2*i+1];
			if( !(alpha == 0 || alpha == 255) )
			{
				// Nop
				return -1;
			}
			if( alpha == 255 )
			{
				counts[ grey ]++;
			}
		}
	}

	// Verify that there is an unused color
	for(int i = 0; i < 256; ++i)
	{
		if( counts[i] == 0 )
		{
			// Found!
			return i;
		}
	}
	// All shades are used
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeGrayScaleAlpha(PngDumpData& dd)
{
	ASSERT( PF_16bppGrayScaleAlpha <= dd.pixelFormat && dd.pixelFormat <= PF_32bppGrayScaleAlpha );
	if( dd.pixelFormat == PF_32bppGrayScaleAlpha )
	{
		// Ignore this flavour for now
		return PerformDumpTries(dd);
	}

	int transIndex = CanSimplifyGreyAlpha(dd);
	if( transIndex < 0 )
	{
		return PerformDumpTries(dd);
	}
	/////////////////////////////
	// Change image data
	dd.pixelFormat = PF_8bppGrayScale;
	dd.useTransparentColor = true;
	dd.tRNS.grey = uint16(transIndex);

	/////////////////////////////
	// Modify pixel buffers
	const int frameCount = dd.frames.GetSize();
	if( dd.hasDefaultImage || frameCount == 0 )
	{
		const int pixelCount = dd.width * dd.height;
		uint8* pPixels = dd.pixels.GetWritePtr();
		for(int i = 0; i < pixelCount; ++i)
		{
			uint8 grey = pPixels[2*i];
			uint8 alpha = pPixels[2*i+1];
			if( alpha == 0 )
			{
				// Use new index
				pPixels[i] = uint8(transIndex);
			}
			else
			{
				pPixels[i] = grey;
			}
		}
		dd.pixels.SetSize(pixelCount); // Shrink buffer
	}

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		ApngFrame* pFrame = dd.frames[iFrame];
		const int width = pFrame->GetWidth();
		const int height = pFrame->GetHeight();
		const int pixelCount = width * height;
		uint8* pPixels = pFrame->m_pixels.GetWritePtr();
		for(int i = 0; i < pixelCount; ++i)
		{
			uint8 grey = pPixels[2*i];
			uint8 alpha = pPixels[2*i+1];
			if( alpha == 0 )
			{
				// Use new index
				pPixels[i] = uint8(transIndex);
			}
			else
			{
				pPixels[i] = grey;
			}
		}
		pFrame->m_pixels.SetSize(pixelCount); // Shrink buffer
	}
	/////////////////////////////

	return PerformDumpTries(dd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::BgraToRgba(PngDumpData& dd)
{
	uint8* pBuffer = dd.pixels.GetWritePtr();
	int32 pixelCount = dd.width * dd.height;

	for(int32 i = 0; i < pixelCount; ++i)
	{
		uint8 b = pBuffer[0];
		uint8 r = pBuffer[2];
		pBuffer[0] = r;
		pBuffer[2] = b;

		pBuffer += 4;
	}
	dd.pixelFormat = PF_32bppRgba;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::BgrToRgb(PngDumpData& dd)
{
	uint8* pBuffer = dd.pixels.GetWritePtr();
	int32 pixelCount = dd.width * dd.height;

	for(int32 i = 0; i < pixelCount; ++i)
	{
		uint8 b = pBuffer[0];
		uint8 r = pBuffer[2];
		pBuffer[0] = r;
		pBuffer[2] = b;
		pBuffer += 3;
	}
	dd.pixelFormat = PF_24bppRgb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::Rgb16ToRgb24(PngDumpData& dd)
{
	const uint8* pSrc = dd.pixels.GetReadPtr();
	int32 pixelCount = dd.width * dd.height;

	Buffer newBuffer;
	if( !newBuffer.SetSize(pixelCount * 3) )
	{
		return false;
	}

	uint8* pDst = newBuffer.GetWritePtr();

	if( dd.pixelFormat == PF_16bppRgb555 )
	{
		for(int32 i = 0; i < pixelCount; i++)
		{
			uint16 nPixel = uint16( (pSrc[0] << 8) | pSrc[1]);
			pSrc += 2;

			uint8 r = uint8((nPixel & 0x7c00) >> 10);
			uint8 g = uint8((nPixel & 0x03e0) >> 5);
			uint8 b = uint8((nPixel & 0x001f));

			r = uint8( (r << 3) + ((r >> 2) & 0x07));
			g = uint8( (g << 3) + ((g >> 2) & 0x07));
			b = uint8( (b << 3) + ((b >> 2) & 0x07));

			pDst[0] = uint8(r);
			pDst[1] = uint8(g);
			pDst[2] = uint8(b);
			pDst += 3;
		}
	}
	else if( dd.pixelFormat == PF_16bppRgb565 )
	{
		for(int32 i = 0; i < pixelCount; i++)
		{
			uint16 nPixel = uint16( (pSrc[0] << 8) | pSrc[1]);
			pSrc += 2;

			uint8 r = uint8( (nPixel & 0xf800) >> 11 );
			uint8 g = uint8( (nPixel & 0x07e0) >> 5 );
			uint8 b = uint8( (nPixel & 0x001f) );

			r = uint8( (r << 3) + ((r >> 2) & 0x07));
			g = uint8( (g << 2) + ((g >> 1) & 0x03));
			b = uint8( (b << 3) + ((b >> 2) & 0x07));

			pDst[0] = uint8(r);
			pDst[1] = uint8(g);
			pDst[2] = uint8(b);
			pDst += 3;
		}
	}
	else
	{
		return false;
	}
	dd.pixelFormat = PF_24bppRgb;
	dd.pixels = newBuffer;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes a buffer in memory and outputs the result as a png file. (public)
// This function ignores chunk options of m_settings
//
// [in] ds        Source buffer description
// [in] filePath  File path of the result
//
// Returns true upon success
/////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeExternalBuffer(const PngDumpData& dd, const String& newFileName)
{
	m_astrErrors.SetSize(0);
	m_resultmgr.Reset();
	m_originalFileWriteTime = DateTime();
	PngDumpData dd2 = dd; // Create modifiable version
	OptiTarget target(newFileName);
	OptiInfo optiInfo;
	return Optimize(dd2, target, optiInfo);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::Optimize(PngDumpData& dd, const OptiTarget& target, OptiInfo& optiInfo)
{
	if( dd.pixelFormat == PF_32bppBgra )
	{
		// Yuk ! Change this
		BgraToRgba(dd);
	}
	else if( dd.pixelFormat == PF_24bppBgr )
	{
		// Yuk ! Change this aswell
		BgrToRgb(dd);
	}
	else if( dd.pixelFormat == PF_16bppRgb555 || dd.pixelFormat == PF_16bppRgb565 )
	{
		// Change this, but harder to do this time because we need to reallocate
		if( !Rgb16ToRgb24(dd) )
		{
			AddError(k_szNotEnoughMemoryForInternalConversion);
			return false;
		}
	}

	PixelFormat pf = dd.pixelFormat;

	bool bOptimizeOk = false;

	if( PF_1bppIndexed <= pf && pf <= PF_8bppIndexed )
	{
		bOptimizeOk = OptimizePaletteMode(dd);
	}
	else if( pf == PF_32bppRgba )
	{
		bOptimizeOk = Optimize32BitsMode(dd);
	}
	else if( pf == PF_24bppRgb )
	{
		bOptimizeOk = Optimize24BitsMode(dd);
	}
	else if( PF_1bppGrayScale <= pf && pf <= PF_16bppGrayScale )
	{
		bOptimizeOk = OptimizeGrayScale(dd);
	}
	else if( pf == PF_16bppGrayScaleAlpha || pf == PF_32bppGrayScaleAlpha )
	{
		bOptimizeOk = OptimizeGrayScaleAlpha(dd);
	}
	else if( pf == PF_48bppRgb || pf == PF_64bppRgba )
	{
		// Other uncommon pixel formats
		bOptimizeOk = PerformDumpTries(dd);
	}
	else
	{
		AddError(k_szUnsupportedPixelFormat);
		return false;
	}

	if( !bOptimizeOk )
	{
		return false;
	}

	return DumpBestResultToFile(target, optiInfo);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::PrintText(const String& text, TextType tt)
{
	ProgressingArg arg;
	arg.text = text;
	arg.textType = tt;
	Progressing.Fire(arg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Builds a new PNG from another one (given by file) excluding unwanted chunks.
//
// [in]  file     Original file
// [out] oriSign  Signature of the original file. This is helpful to know if overwriting
//                the original file is really needed.
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::InsertCleanOriginalPngAsResult(IFile& file, PngSignature& oriSign)
{
	if( !Png::IsPng(file) )
	{
		// Not a PNG
		ASSERT(0); // Programming error
		return false;
	}
	oriSign.Clear();

	const int64 fileSize = file.GetSize();
	const int32 fileSize32 = int32(fileSize);

	DynamicMemoryFile& dmf = m_resultmgr.GetCandidate();
	dmf.SetByteOrder(boBigEndian);

	// Ensure capacity
	if( !dmf.EnsureCapacity(fileSize32) )
	{
		AddError(k_szNotEnoughMemoryToKeepOriginalFile);
		return false;
	}
	if( !dmf.SetPosition(0) )
	{
		AddError(k_szCannotSetFilePosition);
		return false;
	}

	if( !PngDumper::WriteSignature(dmf) )
	{
		return false;
	}

	ChunkedFile chfIn(file);

	bool bOkHandled = true;
	bool bIENDFound = false;
	bool forcedChunksWritten = false;
	PngChunk_IHDR pngIHDR; // We need to get some information here

	while( !bIENDFound)
	{
		bool chunkPresent = chfIn.BeginChunkRead();
		if( !chunkPresent )
		{
			break;
		}

		if( chfIn.m_chunkName == PngChunk_IHDR::Name )
		{
			// Read this one to get some info
			int64 posBefore = file.GetPosition();
			if( Png::Read_IHDR(file, chfIn.m_chunkSize, pngIHDR) != 0 )
			{
				AddError(k_szCannotLoadFile);
				return false;
			}
			file.SetPosition(posBefore);
		}

		// If we remove or force a chunk, we remove the original one(s)
		bool keepChunk = false;
		if( chfIn.m_chunkName == PngChunk_bKGD::Name )
		{
			keepChunk = (m_settings.bkgdOption == POChunk_Keep);
		}
		else if( chfIn.m_chunkName == PngChunk_pHYs::Name )
		{
			keepChunk = (m_settings.physOption == POChunk_Keep);
		}
		else if( chfIn.m_chunkName == PngChunk_tEXt::Name )
		{
			keepChunk = (m_settings.textOption == POChunk_Keep);
		}
		else
		{
			switch( chfIn.m_chunkName )
			{
			case PngChunk_IEND::Name:
				bIENDFound = true;
				// fallthrough
			case PngChunk_IHDR::Name:
			case PngChunk_PLTE::Name:
			case PngChunk_IDAT::Name:
			case PngChunk_tRNS::Name:
			case PngChunk_acTL::Name:
			case PngChunk_fcTL::Name:
			case PngChunk_fdAT::Name:
				keepChunk = true;
				break;

			default:
				break;
			}
		}

		// Write forced chunks if any, before acTL if found, or IDAT
		if( !forcedChunksWritten && (chfIn.m_chunkName == PngChunk_acTL::Name || chfIn.m_chunkName == PngChunk_IDAT::Name) )
		{
			if( m_settings.bkgdOption == POChunk_Force )
			{
				PixelFormat pf = Png::GetPixelFormat(pngIHDR);
				if( pf == PF_32bppRgba )
				{
					// Only for this mode
					PngChunk_bkGD bkGD;
					bkGD.red = m_settings.bkgdColor.r;
					bkGD.green = m_settings.bkgdColor.g;
					bkGD.blue = m_settings.bkgdColor.b;

					ChunkedFile cf(dmf);
					if( !PngDumper::WriteChunk_bkGD(cf, pngIHDR.colorType, bkGD) )
					{
						return false;
					}
				}
			}

			if( m_settings.physOption == POChunk_Force )
			{
				PngChunk_pHYs phys;
				phys.pixelsPerUnitX = m_settings.physPpmX;
				phys.pixelsPerUnitY = m_settings.physPpmY;
				phys.unit = 1; // 1: meter

				ChunkedFile cf(dmf);
				if( !PngDumper::WriteChunk_pHYs(cf, phys) )
				{
					return false;
				}
			}

			if( m_settings.textOption == POChunk_Force )
			{
				PngChunk_tEXt text;
				text.keyword = m_settings.textKeyword;
				text.data = m_settings.textData;

				ChunkedFile cf(dmf);
				if( !PngDumper::WriteChunk_tEXt(cf, text) )
				{
					return false;
				}
			}

			forcedChunksWritten = true;
		}

		if( keepChunk )
		{
			// We keep that chunk
			dmf.Write32(chfIn.m_chunkSize);
			dmf.Write32(chfIn.m_chunkName);
			if( dmf.WriteFromFile(chfIn, chfIn.m_chunkSize) != chfIn.m_chunkSize )
			{
				// The source file is invalid
				bOkHandled = false;
				break;
			}
		}

		if( !chfIn.EndChunkRead() )
		{
			// The source file is invalid
			bOkHandled = false;
			break;
		}

		if( keepChunk )
		{
			// We bypassed the CRC computation, so we just write the one we read
			// chfIn.m_crc is valid only after a call to EndChunkRead()
			dmf.Write32(chfIn.m_crc);
		}
		oriSign.Add(chfIn.m_crc);
	}

	if( !(bOkHandled && bIENDFound) )
	{
		AddError(k_szCorruptedChunkStructure);
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes/Converts a file from disk and outputs the result as a png file.
// This function ignores m_settings.backupOldPngFiles.
//
// [in]  filePath      File path of the file to optimize
// [in]  newFilePath   File path of the optimized/converted file
// [out] optiInfo
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeFileDiskNoBackup(const String& filePath, const String& newFilePath, OptiInfo& optiInfo)
{
	m_astrErrors.SetSize(0);
	m_resultmgr.Reset();
	m_originalFileWriteTime = DateTime();

	optiInfo.Clear();

	File fileImage;
	if( !fileImage.Open(filePath, File::modeRead) )
	{
		AddError(k_szCannotLoadFile);
		return false;
	}
	int64 srcFileSize = fileImage.GetSize();
	if( srcFileSize > MAX_INT32 )
	{
		AddError(k_szFileTooLarge);
		return false;
	}

	if( m_settings.keepFileDate )
	{
		m_originalFileWriteTime = fileImage.GetLastWriteTime();
	}

	OptiTarget target(newFilePath);

	// The source data is a file, set the source information field of the target struct
	target.srcInfo.filePath = filePath;
	target.srcInfo.fileSize = static_cast<int>(srcFileSize);

	if( !OptimizeFileStreamNoBackup(fileImage, target, optiInfo) )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes/Converts a file and output the result to a memory buffer.
//
// [in]  imgBuf        Buffer containing the image file to optimize
// [in]  imgSize       Size of imgBuf buffer
// [out] dst           Destination buffer receiving the optimized/converted PNG file
// [in]  dstCapacity   Capacity in bytes of the destination buffer
// [out] pDstSize      File size of the optimized/converted PNG file
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeFileMem(const uint8* imgBuf, int imgSize, uint8* dst, int dstCapacity, int* pDstSize)
{
	m_astrErrors.SetSize(0);
	m_resultmgr.Reset();
	m_originalFileWriteTime = DateTime();

	if( imgBuf == nullptr || imgSize <= 0 || dst == nullptr || dstCapacity <= 0 || pDstSize == nullptr )
	{
		AddError(k_szInvalidArgument);
		return false;
	}

	StaticMemoryFile fileImage;
	if( !fileImage.OpenRead(imgBuf, imgSize) )
	{
		return false;
	}
	OptiTarget target(dst, dstCapacity);
	OptiInfo optiInfo;
	bool ret = OptimizeFileStreamNoBackup(fileImage, target, optiInfo);
	*pDstSize = optiInfo.sizeAfter;
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes/Converts a file from stdin and output the result to stdout.
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeFileStdio()
{
	m_astrErrors.SetSize(0);
	m_resultmgr.Reset();
	m_originalFileWriteTime = DateTime();

	StdFile fileImage(StdFileType::Stdin);
	OptiTarget target; // No argument means stdout
	OptiInfo optiInfo;
	bool ret = OptimizeFileStreamNoBackup(fileImage, target, optiInfo);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Loads a file or stdin content to a memory buffer
// Closes fileImage.
static bool LoadFileToMem(IFile& fileImage, DynamicMemoryFile& dmfAsIs)
{
	const int64 fileSize = fileImage.GetSize();
	if( fileSize > MAX_INT32 )
	{
		return false;
	}
	// -1 is accepted, means stdin
	int initialCapacity = 0;
	if( fileSize >= 0 )
	{
		initialCapacity = static_cast<int>(fileSize);
	}

	if( !dmfAsIs.Open(initialCapacity) )
	{
		return false;
	}
	int ret = dmfAsIs.WriteFromFile(fileImage, int(fileSize));
	// Regular file has a known file size
	if( fileSize > 0 && ret != fileSize )
	{
		return false;
	}
	// File size unknown, just check if the number of bytes read is enough
	if( fileSize < 0 && ret <= 0 )
	{
		return false;
	}
	if( !dmfAsIs.SetPosition(0) )
	{
		return false;
	}
	fileImage.Close(); // No need for the file anymore
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes a file stored in a representation of a file, which can be on disk, in memory or from
// stdio.
//
// [in,out] fileImage   File stream
// [in]     target      Kind of wanted destination
// [out]    singleOpti  Result size written in sizeAfter
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeFileStreamNoBackup(IFile& fileImage, const OptiTarget& target, OptiInfo& optiInfo)
{
	m_astrErrors.Clear();

	/////////////////////////////////////////////
	// Load as-is in memory
	DynamicMemoryFile dmfAsIs;
	if( !LoadFileToMem(fileImage, dmfAsIs) )
	{
		AddError(k_szCannotLoadFile);
		return false;
	}

	// Needed for display
	optiInfo.sizeBefore = static_cast<int>(dmfAsIs.GetSize());

	/////////////////////////////////////////////
	ImageLoader imgloader;
	if( !imgloader.InstanciateLosslessFormat(dmfAsIs) )
	{
		AddError(k_szUnsupportedFileFormat);
		return false;
	}

	ImageFormat& img = *imgloader.m_pImageType;
	bool loadOk = false;

	if( imgloader.m_type == ImageLoader::Type_Png )
	{
		loadOk = img.LoadFromFile(dmfAsIs);

		// Insert a clean version of the source PNG
		// "clean" means the same PNG expect some unwanted chunks (like the gamma chunk)

		// As we insert a copy of the source file, the source file becomes a candidate for the best result,
		// thus if we cannot achieve a better compression than the original file, we just dump the original file
		dmfAsIs.SetPosition(0);
		if( !InsertCleanOriginalPngAsResult(dmfAsIs, optiInfo.srcSignature) )
		{
			return false;
		}
	}
	else
	{
		loadOk = img.LoadFromFile(dmfAsIs);
	}

	if( !loadOk )
	{
		String strImgErr = img.GetLastErrorString();

		String strErr = "Cannot load image: ";
		strErr = strErr + strImgErr;

		AddError(strErr);
		return false;
	}

	const int32 width = img.GetWidth();
	const int32 height = img.GetHeight();
	const Buffer& pixels = img.GetPixels();
	const Palette& palette = img.GetPalette();
	PixelFormat pf = img.GetPixelFormat();

	//////////////////////////////////////////////////////////////
	PngDumpData dd; // Final dump structure
	dd.width = width;
	dd.height = height;
	dd.pixels = pixels;
	dd.palette = palette;
	dd.pixelFormat = pf;

	if( imgloader.m_type == ImageLoader::Type_Png )
	{
		const Png& png = (const Png&) img;
		dd.useTransparentColor = png.HasSimpleTransparency();
		dd.tRNS.grey = png.GetGreyTransIndex();
		png.GetTransIndexes(dd.tRNS.red, dd.tRNS.green, dd.tRNS.blue);

		if( m_settings.keepInterlacing )
		{
			if( png.IsInterlaced() )
			{
				dd.interlaced = true;
			}
		}

		if( m_settings.bkgdOption == POChunk_Keep )
		{
			// We only keep the background color for non-palette images
			// Why ?
			// 1. Tracking the correct background index through a palette reorganization is very tricky
			// 2. Keeping background color is used so 32 bits images are not blended with gray
			// in Internet Explorer 6
			//
			// So let's wait before a real need comes

			if( PF_1bppIndexed <= pf && pf <= PF_8bppIndexed )
			{
				// Give up
			}
			else
			{
				dd.useBackgroundColor = png.HasBackgroundColor();
				dd.bkGD.grey = png.GetBkGrey();
				dd.bkGD.red = png.GetBkRed();
				dd.bkGD.green = png.GetBkGreen();
				dd.bkGD.blue = png.GetBkBlue();
			}
		}

		if( m_settings.physOption == POChunk_Keep )
		{
			dd.usePhys = png.HasPhysicalPixelDimensions();
			dd.pHYs.pixelsPerUnitX = png.GetPhysPpuX();
			dd.pHYs.pixelsPerUnitY = png.GetPhysPpuY();
			dd.pHYs.unit = png.GetPhysUnit();
		}

		if( m_settings.textOption == POChunk_Keep )
		{
			dd.textInfos = png.GetTexts();
		}

		if( png.IsAnimated() )
		{
			PrintText("[APNG] ", TT_Animated);
		}
	}
	else if( imgloader.m_type == ImageLoader::Type_Gif )
	{
		const Gif& gif = (const Gif&) img;
		if( gif.IsAnimated() )
		{
			if( m_settings.ignoreAnimatedGifs )
			{
				PrintText("[Animated GIF: ignored]\n", TT_Animated);
				return false;
			}
			else
			{
				PrintText("[Animated GIF: converting to APNG] ", TT_Animated);
			}
		}
	}

	if( m_settings.bkgdOption == POChunk_Force )
	{
		dd.useBackgroundColor = true;
		dd.bkGD.Clear();
		dd.bkGD.red = m_settings.bkgdColor.r;
		dd.bkGD.green = m_settings.bkgdColor.g;
		dd.bkGD.blue = m_settings.bkgdColor.b;
	}
	if( m_settings.physOption == POChunk_Force )
	{
		dd.usePhys = true;
		dd.pHYs.Clear();
		dd.pHYs.pixelsPerUnitX = m_settings.physPpmX;
		dd.pHYs.pixelsPerUnitY = m_settings.physPpmY;
		dd.pHYs.unit = 1; // 1: meter
	}
	if( m_settings.textOption == POChunk_Force )
	{
		PngChunk_tEXt text;
		text.keyword = m_settings.textKeyword;
		text.data = m_settings.textData;
		dd.textInfos.SetSize(1);
		dd.textInfos[0] = text;
	}

	if (m_settings.keepPixels)
	{
		return DumpBestResultToFile(target, optiInfo);
	}
	else
	{
		if (img.IsAnimated())
		{
			return OptimizeAnimated(img, dd, target, optiInfo);
		}
		return Optimize(dd, target, optiInfo);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::PrintSizeChange(int64 sizeBefore, int64 sizeAfter, bool sameContent)
{
	if( !(sizeBefore > 0 && sizeAfter > 0) )
	{
		PrintText("\n", TT_FileSizeSame);
		return;
	}
	int64 ratio = sizeAfter;
	ratio *= 100;
	ratio /= sizeBefore;

	String arrow;
	if( m_unicodeArrowEnabled )
	{
		static const wchar arrow2 = 0x2192; // utf-8: { 0xE2, 0x86, 0x92 }
		arrow = String(&arrow2, 1);
	}
	else
	{
		arrow = "->";
	}
	StringBuilder sb;
	PrintText(String::FromInt64(sizeBefore), TT_SizeInfoNum);
	PrintText(" bytes "+arrow+" ", TT_SizeInfo);
	PrintText(String::FromInt64(sizeAfter), TT_SizeInfoNum);
	PrintText(" bytes", TT_SizeInfo);

	sb.Empty();

	sb += " (";
	if( !sameContent )
	{
		sb += String::FromInt(uint32(ratio));
		sb += "% of the original size";
	}
	else
	{
		sb += "unmodified";
	}
	sb += ")\n"; // End of line here

	if( sizeAfter > sizeBefore )
	{
		PrintText(sb.ToString(), TT_FileEnlarged);
	}
	else if( sizeAfter == sizeBefore )
	{
		PrintText(sb.ToString(), TT_FileSizeSame);
	}
	else
	{
		PrintText(sb.ToString(), TT_FileReduced);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes one single file
//
// [in]   filePath
// [in]   displayDir
// [out]  optiInfo
//
// Returns true upon success
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeFileDisk(const String& filePath, const String& displayDir, OptiInfo& optiInfo)
{
	optiInfo.Clear();
	m_astrErrors.Clear();

	bool srcIsDir = false;
	bool srcIsReadOnly = false;
	if( !File::GetFileAttributes(filePath, srcIsDir, srcIsReadOnly) )
	{
		// We cannot even read file attributes, so we just stop there for this file
		AddError(k_szPathDoesNotExist);
		return false;
	}
	if( srcIsDir )
	{
		AddError(k_szPathIsNotAFile);
		return false;
	}

	// We need the name alone
	String strDirOnly, strNameOnly;
	FilePath::Split(filePath, strDirOnly, strNameOnly);

	// Filter by extension
	String fileExt = FilePath::GetExtension(filePath).ToLowerCase();
	if( !IsFileExtensionSupported(fileExt) )
	{
		// Nothing to do
		AddError(k_szUnsupportedFileType);
		return false;
	}

	///////////////////////////////////////////////////////////////////////
	// Write the waiting message during the optimization
	if( fileExt == "png" || fileExt == "apng" )
	{
		PrintText("Optimizing ", TT_ActionVerb);
	}
	else
	{
		PrintText("Converting ", TT_ActionVerb);
	}
	///////////////////////////////////////////////////////////////////////

	if( !displayDir.IsEmpty() )
	{
		// Display the sub-directory
		PrintText(displayDir + "/", TT_FilePath);
	}
	PrintText(strNameOnly, TT_FilePath);
	PrintText(" ", TT_RegularInfo);

	strDirOnly = FilePath::AddSeparator(strDirOnly);

	String oldFilePath;
	String newFilePath;

	// If the source file is a PNG, we rename that source file with a "_" at the beginning of its name
	if( fileExt == "png" || fileExt == "apng" )
	{
		if( srcIsReadOnly )
		{
			// File is read only, we won't be able to modify it
			AddError(k_szFileIsReadOnly);
			return false;
		}

		if( m_settings.backupOldPngFiles )
		{
			String backupFilePath = strDirOnly + "_" + strNameOnly;

			// Delete a possible previous backup file
			if( File::Exists(backupFilePath) )
			{
				if( !File::Delete(backupFilePath) )
				{
					// We cannot delete the old backup file, we stop here
					AddError(k_szCannotPerformBackup);
					return false;
				}
			}

			if( File::Rename(filePath, backupFilePath) )
			{
				// Rename possible
				oldFilePath = backupFilePath;
				newFilePath = filePath;
			}
			else
			{
				// We cannot rename for unknown reason
				AddError(k_szCannotPerformBackupRenameFailed);
				return false;
			}
		}
		else
		{
			oldFilePath = filePath;
			newFilePath = filePath;
		}
	}
	else
	{
		// Not a PNG file
		oldFilePath = filePath;
		newFilePath = strDirOnly + FilePath::RemoveExtension(strNameOnly) + ".png";
	}

	// TMP DEBUG : to compare size before and after, uncomment the line above
	//newFilePath = File::AddFileNamePrefix(newFilePath, "optimized-");

	if( !OptimizeFileDiskNoBackup(oldFilePath, newFilePath, optiInfo) )
	{
		return false;
	}
	PrintText(" (OK) ", TT_ActionOk);
	PrintSizeChange(optiInfo.sizeBefore, optiInfo.sizeAfter, optiInfo.sameContent);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Optimizes multiple files and directories (private)
//
// [in] baseDir            Directory name containing the files
// [in] fileNames          File path relative to base dir
// [in] displayDir         Directory name to display
// [in] joker              Type of files managed
// [in,out] multiOptiInfo  Optimization information
///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::OptimizeFilesInternal(const String& baseDir, const StringArray& fileNames, const String& displayDir,
                                     const String& joker, MultiOptiInfo& multiOptiInfo)
{
	const int32 fileCount = fileNames.GetSize();
	for(int32 iFile = 0; iFile < fileCount; ++iFile)
	{
		String filePath = FilePath::Combine(baseDir, fileNames[iFile]);

		bool srcIsDir = false;
		bool srcIsReadOnly = false;
		if( !File::GetFileAttributes(filePath, srcIsDir, srcIsReadOnly) )
		{
			// We cannot even read file attributes, so we just stop there for this file
			AddError(k_szPathDoesNotExist);
		}
		else
		{
			// We need the name alone
			String strDirOnly, strNameOnly;
			FilePath::Split(filePath, strDirOnly, strNameOnly);
			if( srcIsDir )
			{
				// A directory, we optimize it before we continue
				StringArray astrSubFileNames = Directory::GetFileNames(filePath, "*");

				// Set the directory to display
				String newDisplayDir;
				if( !displayDir.IsEmpty() )
				{
					newDisplayDir = displayDir + "/";
				}
				newDisplayDir = newDisplayDir + strNameOnly;
				OptimizeFilesInternal(filePath, astrSubFileNames, newDisplayDir, joker, multiOptiInfo);
			}
			else
			{
				// A file. Filter by extension.
				if( IsFileExtensionSupported(FilePath::GetExtension(filePath), joker) )
				{
					// Call the single file optimization function
					OptiInfo soi;
					multiOptiInfo.optiCount++;
					m_astrErrors.Clear();
					if( !OptimizeFileDisk(filePath, displayDir, soi) )
					{
						multiOptiInfo.errorCount++;
						String strLastError = GetLastErrorString();
						PrintText(" (KO) ", TT_ActionFail);
						PrintText(strLastError + "\n", TT_ErrorMsg);
					}
					else
					{
						multiOptiInfo.sizeBefore += soi.sizeBefore;
						multiOptiInfo.sizeAfter += soi.sizeAfter;
					}
					m_astrErrors.Clear();
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Optimize several files or directories on disk from their paths (public)
//
// [in] filePaths  Absolute file paths of files or directories to optimize
// [in] joker      File types handled. ex: "*.png" or "*.gif|*.bmp"
//
// Returns true if all filtered files could be optimized or converted,
//         false if at least one error was raised during the call.
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeMultiFilesDisk(const StringArray& filePaths, const String& joker)
{
	uint32 startTime = System::GetTime();
	MultiOptiInfo multiOptiInfo;
	OptimizeFilesInternal("", filePaths, "", joker, multiOptiInfo);

	bool success = (multiOptiInfo.errorCount == 0);
	if( multiOptiInfo.optiCount > 1 )
	{
		// Several files, write a summary of the batch optimization
		String doneMsg;
		TextType tt = success ? TT_BatchDoneOk : TT_BatchDoneFail;

		uint32 stopTime = System::GetTime();

		PrintText("-- Done --  " + String::FromInt(stopTime - startTime) + " ms ", tt); // \x2014
		PrintSizeChange(multiOptiInfo.sizeBefore, multiOptiInfo.sizeAfter, false);
	}
	else
	{
		// OptimizeFilesInternal will silently filter out files that are not supported.
		// However, for a public function, when no file at all is optimized, this is
		// considered as an error.
		if( multiOptiInfo.optiCount == 0 && multiOptiInfo.errorCount == 0
		 && filePaths.GetSize() == 1 && File::Exists(filePaths[0]) )
		{
			PrintText(String(k_szUnsupportedFileType) + ": " + FilePath::GetName(filePaths[0]) + "\n", TT_ErrorMsg);
			success = false;
		}
	}
	return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::ClearLastError()
{
	m_astrErrors.Clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::AddError(const String& str)
{
	m_astrErrors.Add(str);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Gets error explanation when an optimization function fails
String POEngine::GetLastErrorString() const
{
	const int32 errCount = m_astrErrors.GetSize();
	if( errCount == 0 )
	{
		return String();
	}

	String str = m_astrErrors[0];
	for(int32 i = 1; i < errCount; ++i)
	{
		str = str + "\n" + m_astrErrors[i];
	}
	return str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Blits a block of pixels to a specific position.
///////////////////////////////////////////////////////////////////////////////////////////////////
void BasicBlit(int bytesPerPixel, const uint8* pSrc, int srcWidth, int srcHeight,
                                        uint8* pDst, int dstWidth, int /*dstHeight*/, int posX, int posY)
{
	// Blit at the right place
	pDst += ((posY * dstWidth) + posX) * bytesPerPixel;

	for(int y = 0; y < srcHeight; ++y)
	{
		for(int i = 0; i < srcWidth * bytesPerPixel; ++i)
		{
			pDst[i] = pSrc[i];
		}
		pSrc += srcWidth * bytesPerPixel;
		pDst += dstWidth * bytesPerPixel;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Unpacks the pixels of each animation frame so one pixel is one byte.
///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::UnpackPixelFrames(PngDumpData& dd)
{
	if( !ImageFormat::IsIndexed(dd.pixelFormat ) )
	{
		// Case not managed
		return;
	}
	if( dd.pixelFormat == PF_8bppIndexed )
	{
		// Already ok
		return;
	}

	const int frameCount = dd.frames.GetSize();

	// Unpack the default image if any
	if( dd.hasDefaultImage || frameCount == 0 )
	{
		ImageFormat::UnpackPixels(dd.pixels, dd.width, dd.height, dd.pixelFormat);
	}

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		ApngFrame* pFrame = dd.frames[iFrame];

		const int width = pFrame->GetWidth();
		const int height = pFrame->GetHeight();
		ImageFormat::UnpackPixels(pFrame->m_pixels, width, height, dd.pixelFormat);
	}
	dd.pixelFormat = PF_8bppIndexed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Packs the pixels of each animation frame according to the pixel format.
// Example: if the pixel format is 8 bits but the palette contains 16 entries, the pixels can be
// repacked to 4 bits.
///////////////////////////////////////////////////////////////////////////////////////////////////
void POEngine::PackPixelFrames(PngDumpData& dd)
{
	if( dd.pixelFormat != PF_8bppIndexed )
	{
		// Case not managed
		ASSERT(0);
		return;
	}
	if( dd.palette.m_count > 16 )
	{
		// Cannot pack
		return;
	}
	if( dd.palette.m_count > 4 )
	{
		dd.pixelFormat = PF_4bppIndexed;
	}
	else if( dd.palette.m_count > 2 )
	{
		dd.pixelFormat = PF_2bppIndexed;
	}
	else
	{
		dd.pixelFormat = PF_1bppIndexed;
	}

	const int frameCount = dd.frames.GetSize();

	// Pack the default image if any
	if( dd.hasDefaultImage || frameCount == 0 )
	{
		ImageFormat::PackPixels(dd.pixels, dd.width, dd.height, dd.pixelFormat);
	}

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		ApngFrame* pFrame = dd.frames[iFrame];

		const int width = pFrame->GetWidth();
		const int height = pFrame->GetHeight();
		ImageFormat::PackPixels(pFrame->m_pixels, width, height, dd.pixelFormat);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fills the DumpSettings with all information from the image
void PrepareAnimatedDumpSettings(const ImageFormat& img, PngDumpData& dd)
{
	// Move image information to dump information, including animation frames
	dd.pixels = img.GetPixels();
	dd.width  = img.GetWidth();
	dd.height = img.GetHeight();
	dd.pixelFormat = img.GetPixelFormat();
	dd.palette    = img.GetPalette();
	dd.hasDefaultImage = img.HasDefaultImage();
	dd.loopCount   = img.GetLoopCount();

	dd.frames.Clear();

	const int frameCount = img.GetFrameCount();
	for(int i = 0; i < frameCount; ++i)
	{
		const AnimFrame* pSrc = img.GetAnimFrame(i);

		ApngFrame* pFrame = new ApngFrame(nullptr);

		pFrame->m_fctl.width = pSrc->GetWidth();
		pFrame->m_fctl.height = pSrc->GetHeight();
		pFrame->m_fctl.offsetX = pSrc->GetOffsetX();
		pFrame->m_fctl.offsetY = pSrc->GetOffsetY();
		pFrame->m_fctl.blending = pSrc->GetBlending();
		pFrame->m_fctl.delayFracDenominator = (uint16) pSrc->GetDelayFracDenominator();
		pFrame->m_fctl.delayFracNumerator = (uint16) pSrc->GetDelayFracNumerator();
		pFrame->m_fctl.disposal = pSrc->GetDisposal();
		if( i == 0 )
		{
			pFrame->m_fctl.blending = AnimFrame::BlendSource;
		}
		else
		{
			pFrame->m_fctl.blending = pSrc->GetBlending();
		}
		pFrame->m_pixels = pSrc->GetPixels();
		dd.frames.Add(pFrame);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
struct FrameNewPal
{
	Palette           palette;
	PaletteTranslator translator;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Merges all palettes in one single one, converts to true color if the merged palette exceeds 256 colors
void MergePalettes(const ImageFormat& img, PngDumpData& dd)
{
	if( dd.pixelFormat != PF_8bppIndexed )
	{
		// Case not managed
		return;
	}

	Array<const Palette*> apPalettes;
	const int frameCount = img.GetFrameCount();
	for(int i = 0; i < frameCount; ++i)
	{
		const AnimFrame* pSrc = img.GetAnimFrame(i);
		if( img.IsIndexed() )
		{
			const Palette* pPal = &(pSrc->GetPalette());
			if( pPal->m_count > 0 )
			{
				apPalettes.AddUnique( pPal );
			}
		}
	}

	Palette superPalette; // In case a merging of local palettes is needed (GIF)

	bool switchToTrueColors = false;
	bool switchToRgba = false;

	// Check if we we should expand the first frame to be used as the default image
	bool expandFirstFrameAndMakeItTheDefaultImage = false;
	if( !dd.hasDefaultImage )
	{
		bool bOk1 = dd.width == dd.frames[0]->m_fctl.width;
		bool bOk2 = dd.height == dd.frames[0]->m_fctl.height;
		bool bOk3 = dd.frames[0]->m_fctl.offsetX == 0;
		bool bOk4 = dd.frames[0]->m_fctl.offsetY == 0;
		expandFirstFrameAndMakeItTheDefaultImage = !(bOk1 && bOk2 && bOk3 && bOk4);
	}
	else
	{
		// A default image is already provided, keep it
	}

	if( !expandFirstFrameAndMakeItTheDefaultImage && apPalettes.GetSize() <= 1 )
	{
		// No trick needed with the default image, one single palette, can stop here
		return;
	}


	// Build new local palettes and the super palette
	bool hasTransparency = false; // Switches to true if one of the color is transparent
	Array<FrameNewPal> frameNewPals;
	frameNewPals.SetSize(frameCount);

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		ApngFrame* pFrame = dd.frames[iFrame];

		uint32 colCounts[256];
		Memory::Zero32(colCounts, 256);
		AddFrameColorCounts(pFrame, colCounts);

		// Create the no-hole palette
		FrameNewPal fnp;
		fnp.palette = img.GetAnimFrame(iFrame)->GetPalette();
		fnp.translator.BuildUnusedColors(fnp.palette, colCounts);

		if( !hasTransparency && fnp.palette.HasNonOpaqueColor() )
		{
			hasTransparency = true;
		}

		if( !switchToTrueColors )
		{
			PaletteTranslator superTranslator;
			if( !superTranslator.MergePalette(superPalette, fnp.palette) )
			{
				// Too many colors, have to switch to Color mode
				// Continue however to compute no holes palettes
				switchToTrueColors = true;
			}
			fnp.translator = PaletteTranslator::Combine(fnp.translator, superTranslator);
		}
		frameNewPals[iFrame] = fnp;
	}

	////////////////////////////////////////////////////////////////////
	// Choose between Color and RGBA when switching to true colors
	if( switchToTrueColors && hasTransparency )
	{
		// Alpha is needed
		switchToRgba = true;
	}
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	if( expandFirstFrameAndMakeItTheDefaultImage )
	{
		// Frame0 expansion is needed, we need to find or create a color to fill the blank.
		// This color has to be transparent.
		if( !switchToTrueColors )
		{
			// Palette mode, look in the super palette
			if( !hasTransparency )
			{
				// No transparent color to be used to create the default image.
				// Try to add a new entry in the palette
				if( superPalette.m_count < 255 )
				{
					superPalette.m_colors[superPalette.m_count].SetRgba(0, 0, 0, 0);
					superPalette.m_count++;
				}
				else
				{
					// The palette is full, we have to switch to true colors with alpha
					switchToTrueColors = true;
					switchToRgba = true;
				}
			}
		}
		else
		{
			// Alpha is needed for transparency
			switchToRgba = true;
		}
	}
	////////////////////////////////////////////////////////////////////

	if( !switchToTrueColors )
	{
		// Use the super palette when saving
		dd.palette = superPalette;

		// Convert all frame indexes to match the super palette
		for(int iFrame = 0; iFrame < frameCount; ++iFrame)
		{
			ApngFrame* pFrame = dd.frames[iFrame];

			const FrameNewPal& fnp = frameNewPals[iFrame];

			int pixelCount = pFrame->m_fctl.width * pFrame->m_fctl.height;
			uint8* pPixels = pFrame->m_pixels.GetWritePtr();
			ASSERT(pFrame->m_pixels.GetSize() == pixelCount);

			fnp.translator.Translate(pPixels, pixelCount);
		}
	}
	else
	{
		// Convert to true-colors
		for(int iFrame = 0; iFrame < frameCount; ++iFrame)
		{
			const AnimFrame* pSrc = img.GetAnimFrame(iFrame);
			ApngFrame* pDst = dd.frames[iFrame];

			int dstPixelSize = switchToRgba ? 4 : 3;
			const int32 nPixCount = pDst->m_fctl.width * pDst->m_fctl.height;
			pDst->m_pixels.SetSize(nPixCount * dstPixelSize);

			const uint8* pSrcPixels = pSrc->GetPixels().GetReadPtr();
			uint8* pDstPixels = pDst->m_pixels.GetWritePtr();
			const Palette* pPalette = &(pSrc->GetPalette());

			if( switchToRgba )
			{
				// Palette to RGBA
				dd.pixelFormat = PF_32bppRgba;
				for(int iPix = 0; iPix < nPixCount; ++iPix)
				{
					uint8 r, g, b, a;
					pPalette->m_colors[ pSrcPixels[iPix] ].ToRgba(r, g, b, a);

					pDstPixels[0] = r;
					pDstPixels[1] = g;
					pDstPixels[2] = b;
					pDstPixels[3] = a;
					pDstPixels += dstPixelSize;
				}
			}
			else
			{
				// Palette to Color
				dd.pixelFormat = PF_24bppRgb;
				for(int iPix = 0; iPix < nPixCount; ++iPix)
				{
					uint8 r, g, b, a;
					pPalette->m_colors[ pSrcPixels[iPix] ].ToRgba(r, g, b, a);

					pDstPixels[0] = r;
					pDstPixels[1] = g;
					pDstPixels[2] = b;
					pDstPixels += dstPixelSize;
				}
			}
		}
	}
	if( expandFirstFrameAndMakeItTheDefaultImage )
	{
		Buffer oldPixels = dd.frames[0]->m_pixels;
		dd.frames[0]->m_pixels.Clear();
		uint32 oldWidth = dd.frames[0]->m_fctl.width;
		uint32 oldHeight = dd.frames[0]->m_fctl.height;
		uint32 oldOffsetX = dd.frames[0]->m_fctl.offsetX;
		uint32 oldOffsetY = dd.frames[0]->m_fctl.offsetY;

		dd.frames[0]->m_fctl.width = dd.width;
		dd.frames[0]->m_fctl.height = dd.height;
		dd.frames[0]->m_fctl.offsetX = 0;
		dd.frames[0]->m_fctl.offsetY = 0;

		dd.frames[0]->m_pixels.SetSize(dd.height * ImageFormat::ComputeByteWidth(dd.pixelFormat, dd.width));

		const uint8* pSrc = oldPixels.GetReadPtr();
		uint8* pDst = dd.frames[0]->m_pixels.GetWritePtr();

		int bytesPerPixel = ImageFormat::SizeofPixelInBits(dd.pixelFormat) / 8;

		// Fill with transparent color
		if( bytesPerPixel == 1 )
		{
			// Use the transparent index
			int transparentColorIndex = superPalette.GetFirstFullyTransparentColor();
			ASSERT(transparentColorIndex >= 0 );
			dd.frames[0]->m_pixels.Fill(uint8(transparentColorIndex));
		}
		else
		{
			// Use transparent black in true colors
			dd.frames[0]->m_pixels.Fill(0);
		}

		BasicBlit(bytesPerPixel, pSrc, oldWidth, oldHeight, pDst, dd.width, dd.height, oldOffsetX, oldOffsetY);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Converts a loaded animated GIF into an APNG file or convert an existing APNG.
// As GIF has no concept of default image, the IDAT is included in the animation.
//
// [in]     img       Loaded image
// [out]    dd        PNG data ready to be dumped
// [in]     filePath  Target file path
//
// Returns true upon success.
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeAnimated(const ImageFormat& img, PngDumpData& dd, const String& filePath,
                                OptiInfo& optiInfo)
{
	OptiTarget target(filePath);
	return OptimizeAnimated(img, dd, target, optiInfo);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Converts a loaded animated GIF into an APNG file or convert an existing APNG.
// As GIF has no concept of default image, the IDAT is included in the animation.
//
// [in]  img      Loaded image
// [out] dd       PNG data ready to be dumped
// [in]  target   Target information: file path or destination buffer
// [out] optiInfo To get the result size
//
// Returns true upon success.
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::OptimizeAnimated(const ImageFormat& img, PngDumpData& dd, const OptiTarget& target,
                                OptiInfo& optiInfo)
{
	if( !img.IsAnimated() )
	{
		return false;
	}

	// Fills the dump settings
	PrepareAnimatedDumpSettings(img, dd);

	// Unpack pixels to 8 bits per pixel if indexed to make treatment easier
	UnpackPixelFrames(dd);

	// Merge palettes if frame local palettes are found (GIF)
	// The pixel format can switch to Color or RGBA
	MergePalettes(img, dd);

	dd.interlaced = false; // Always to false with animation

	PngDumpSettings ds;
	ds.filtering = 0;
	ds.zlibCompressionLevel = 9;

	// If there is one frame only, use it as the main image and discard any animation information
	if( dd.pixels.IsEmpty() && dd.frames.GetSize() == 1 )
	{
		ApngFrame* pFrame = dd.frames[0];
		dd.width = pFrame->GetWidth();
		dd.height = pFrame->GetHeight();
		dd.pixels = pFrame->m_pixels;
		dd.frames.Clear();
	}

	bool optiOk = false;
	if( dd.pixelFormat == PF_8bppIndexed )
	{
		// Continue with specialized optimizer
		optiOk = OptimizePaletteMode(dd);
	}
	else
	{
		// Other pixel formats
		optiOk = PerformDumpTries(dd);
	}

	bool dumpOk = false;
	if( optiOk )
	{
		dumpOk = DumpBestResultToFile(target, optiInfo);
	}
	m_resultmgr.Reset();
	return optiOk && dumpOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Checks a file extension against a joker string.
//!
//! [in]  ext    File extension
//! [in]  joker  Joker string, ex:"*.bmp|*.tga". Empty to check against all supported types.
//!
//! Returns true if the file extension is supported as declared by the joker string
///////////////////////////////////////////////////////////////////////////////////////////////////
bool POEngine::IsFileExtensionSupported(const String& ext, const String& joker)
{
	String extLow = ext.ToLowerCase();
	static const char* const supportedExts[] = {
		"png",
		"apng",
		"gif",
		"bmp",
		"tga"
	};
	bool supported = false;
	for(int i = 0; i < ARRAY_SIZE(supportedExts); ++i)
	{
		if( extLow == supportedExts[i] )
		{
			supported = true;
			break;
		}
	}
	if( !supported )
	{
		return false;
	}
	if( joker.IsEmpty() )
	{
		return true;
	}
	return joker.ToLowerCase().Find(extLow, 0) >= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Gets a color related to a text type.
Color POEngine::ColorFromTextType(TextType tt, bool darkTheme)
{
	// Light theme / Dark theme
	static const Color colors[][2] = {
		{ {  0,   0,   0}, {230, 230, 230} }, // FilePath
		{ {100, 100, 100}, {120, 120, 120} }, // RegularInfo
		{ {100, 100, 100}, {120, 120, 120} }, // ActionVerb (Creating, Converting, Optimizing...)
		{ {100, 100, 100}, {120, 120, 120} }, // SizeInfo (xxx bytes -> yyy bytes)
		{ {100, 100, 100}, {120, 120, 120} }, // SizeInfoNum (xxx bytes -> yyy bytes)
		{ {200, 100,   0}, {220, 120,  20} }, // FileEnlarged (103% of the original size)
		{ {100, 100, 100}, {120, 120, 120} }, // FileSizeSame (100% of the original size)
		{ {  0, 120,   0}, { 20, 200,  20} }, // FileReduced (80% of the original size)
		{ {  0, 100,   0}, { 20, 170,  20} }, // ActionOk
		{ {200,   0,   0}, {200,  20,  20} }, // ActionFail
		{ {255,   0,   0}, {255,  20,  20} }, // ErrorMsg
		{ {  0,  50, 200}, { 50, 100, 230} }, // Animated [APNG] [Animated GIF : converting to APNG]
		{ {  0, 100,   0}, { 20, 170,  20} }, // BatchDoneOk
		{ {200,   0,   0}, {200,  20,  20} }, // BatchDoneFail
		{ {  0,   0,   0}, {  0,   0,   0} }, // Last
	};
	static const int colorCount = sizeof(colors) / (sizeof(Color) * 2);
	static_assert(colorCount == (TT_Last + 1), "Color count mismatch");
	const int index0 = static_cast<int>(tt);
	if( !(0 <= index0 && index0 < colorCount) )
	{
		return Color::Red;
	}
	const int index1 = static_cast<int>(darkTheme);
	if( !(0 <= index1 && index1 < 2) )
	{
		return Color::Red;
	}
	return colors[index0][index1];
}
