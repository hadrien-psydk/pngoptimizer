/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PaletteTranslator.h"

using namespace chustd;

void PaletteTranslator::BuildIdentity()
{
	// Init with identity
	for(int i = 0; i < 256; ++i)
	{
		conv[i] = uint8(i);
	}
}

// Translates one pixel buffer
void PaletteTranslator::Translate(uint8* pPixels, int pixelCount) const
{
	const uint8* pConvPtr = conv;
	for(int i = 0; i < pixelCount; ++i)
	{
		uint8 oldValue = pPixels[i];
		uint8 newValue = pConvPtr[oldValue];
		pPixels[i] = newValue;
	}
}

// Translate the default image and all animation frames
void PaletteTranslator::TranslateAll(PngDumpData& dd) const
{
	if( dd.pixelFormat != PF_8bppIndexed
     && dd.pixelFormat != PF_8bppGrayScale )
	{
		// Case not managed
		return;
	}

	const int frameCount = dd.frames.GetSize();

	if( dd.hasDefaultImage || frameCount == 0 )
	{
		Translate(dd.pixels.GetWritePtr(), dd.width * dd.height);
	}

	for(int iFrame = 0; iFrame < frameCount; ++iFrame)
	{
		ApngFrame* pFrame = dd.frames[iFrame];

		const int width = pFrame->GetWidth();
		const int height = pFrame->GetHeight();
		Translate(pFrame->m_pixels.GetWritePtr(), width * height);
	}
}

void PaletteTranslator::BuildSortAlpha(Palette& palette, uint32 pColCounts[256])
{
	// Buyild the elements to sort (indexes in the palette)
	// Get back the sort attributes (the alpha of each color)
	uint32 aAlphaSortedIndexes[256];
	uint8 aAlphas[256];
	for(int32 i = 0; i < 256 ;++i)
	{
		aAlphaSortedIndexes[i] = i;
		aAlphas[i] = palette.m_colors[i].GetAlpha();
	}
	
	uint32* paElements = aAlphaSortedIndexes;
	uint8* paSortAttributs = aAlphas;
	int colCount = palette.m_count;

	chustd::Sort::ByteSortAtt(paElements, paSortAttributs, colCount);
	
	Palette palNewNew; // New sorted palette
	palNewNew.m_count = colCount;
	
	for(int32 i = 0; i < colCount; ++i)
	{
		palNewNew.m_colors[i] = palette.m_colors[ aAlphaSortedIndexes[i]];
	}
	
	// Fill the translation table
	for(int32 i = 0; i < 256; ++i)
	{
		int32 n = aAlphaSortedIndexes[i];
		conv[n] = uint8(i);
	}
	// Overwrite the old palette
	palette = palNewNew;

	if( pColCounts != nullptr)
	{
		// Update color counts
		UpdateCounts(pColCounts);
	}
}

// Removes unused colors in the palette
// palette [in,out] Palette to reorganize
// pCounts [in,out] Number of colors used for each of the 256 colors of the palette
void PaletteTranslator::BuildUnusedColors(Palette& palette, uint32* pCounts)
{
	/////////////////////////////////////////////////////////
	// Remove holes in the palette and create the conversion table old index -> new index
	int colCount = 0; // Number of colors in the palette
	int nHoleAt = -1;
	int iNoHoles = 0;
	
	Palette palNew; // New palette without holes
	
	for(int i = 0; i < 256; ++i)
	{
		if( pCounts[i] != 0 )
		{
			colCount++;
			
			if( nHoleAt >= 0 )
			{
				// One of the previous color was not used
				//bHolesInPaletteFound = true; // Commented out, this is for debugging only
			}
			
			conv[i] = (uint8) iNoHoles;
			palNew.m_colors[iNoHoles] = palette.m_colors[i];
			
			iNoHoles++;
		}
		else
		{
			if( nHoleAt < 0 )
			{
				nHoleAt = i;
			}
		}
	}
	palNew.m_count = colCount;
	palette = palNew;
	UpdateCounts(pCounts);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sorts the palette by population and update all pixels
// Starts sorting on the first opaque color (colors should be sorted first by alpha)
//
// palette    [in,out]   Palette to sort
// pColCounts [in,out]   Number of pixels using the same color for each of the 256 colors
void PaletteTranslator::BuildSortPopulation(Palette& palette, uint32* pColCounts)
{
	const int32 colCount = palette.m_count;

	//////////////////////////////////////////////////////////////////
	// Sort opaque alphas (255) according to their population
	int32 populationSortStart = 0;
	for(int i = 0; i < colCount; ++i)
	{
		if( palette.m_colors[i].GetAlpha() == 255 )
		{
			// We can start here
			populationSortStart = i;
			break;
		}
	}
	
	uint32 sortedIndexes[256];
	for(int i = 0; i < 256; ++i)
	{
		sortedIndexes[i] = i;
	}

	const int32 popColCount = colCount - populationSortStart;
	uint32* paPopElements = sortedIndexes + populationSortStart;
	uint32* paPopSortAttributs = pColCounts + populationSortStart;
	
	chustd::Sort::ByteSortAtt(paPopElements, paPopSortAttributs, popColCount);

	// Reverse the sort
	for(int i = 0; i < popColCount / 2; ++i)
	{
		const int32 offsetFar = popColCount - i - 1;
		uint32 elem = paPopElements[i];
		paPopElements[i] = paPopElements[offsetFar];
		paPopElements[offsetFar] = elem;

		uint32 att = paPopSortAttributs[i];
		paPopSortAttributs[i] = paPopSortAttributs[offsetFar];
		paPopSortAttributs[offsetFar] = att;
	}

	// Create the new palette in reverse order of their population
	// (the most popular have the lowest index)
	Palette palPopSorted;
	palPopSorted.m_count = colCount;

	for(int i = 0; i < colCount; ++i)
	{
		palPopSorted.m_colors[i] = palette.m_colors[ sortedIndexes[i] ];
	}
	
	// Create the converstion table old indexes --> new indexes
	for(int i = 0; i < 256; ++i)
	{
		int32 n = sortedIndexes[i];
		conv[n] = uint8(i);
	}
	palette = palPopSorted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sorts the palette by luminance. The first color used is the first opaque color, thus the palette
// should be first sorted by alpha.
void PaletteTranslator::BuildSortLuminance(Palette& palette, uint32* pCounts)
{
	const int32 colCount = palette.m_count;

	//////////////////////////////////////////////////////////////////
	// Sort opaque alphas (255) according to their intensity
	int32 nLuminanceSortStart = 0;
	for(int32 i = 0; i < colCount; ++i)
	{
		if( palette.m_colors[i].GetAlpha() == 255 )
		{
			// We can start here
			nLuminanceSortStart = i;
			break;
		}
	}
	
	uint32 luminances[256];
	uint32 lumSortedIndexes[256];
		
	for(int32 i = 0; i < 256; ++i)
	{
		lumSortedIndexes[i] = i;
		luminances[i] = 0;
	}

	for(int32 i = nLuminanceSortStart; i < colCount; ++i)
	{
		uint32 r, g, b;
		palette.m_colors[i].ToRgb(r, g, b);
		
		// Y = R * 0.299 + G * 0.587 + B * 0.114;
		uint32 y = r * 299 + g * 587 + b * 114;
		luminances[i] = y;
	}

	const int32 nLumColCount = colCount - nLuminanceSortStart;
	uint32* paLumElements = lumSortedIndexes + nLuminanceSortStart;
	uint32* paLumSortAttributs = luminances + nLuminanceSortStart;
	
	chustd::Sort::ByteSortAtt(paLumElements, paLumSortAttributs, nLumColCount);

	// Swap colors from the palette in order to sort them
	Palette palLumSorted;
	palLumSorted.m_count = colCount;

	for(int32 i = 0; i < colCount; ++i)
	{
		palLumSorted.m_colors[i] = palette.m_colors[ lumSortedIndexes[i] ];
	}
	
	// conv2
	for(int i = 0; i < 256; ++i)
	{
		int32 n = lumSortedIndexes[i];
		conv[n] = uint8(i);
	}
	palette = palLumSorted;
	if( pCounts != nullptr)
	{
		UpdateCounts(pCounts);
	}
}

// Merge pal2 into pal1
// If a color in pal2 already exists in pal1, the one in pal1 is used instead
bool PaletteTranslator::MergePalette(Palette& pal1, const Palette& pal2)
{
	for(int i = 0; i < pal2.m_count; ++i)
	{
		int newIndex;
		// Special case for alhpa=0, we ignore the RGB components
		if( pal2[i].a == 0 )
		{
			newIndex = pal1.GetFirstFullyTransparentColor();
		}
		else
		{
			newIndex = pal1.FindColor( pal2.m_colors[i] );
		}
		if( newIndex < 0 )
		{
			// Add an entry
			if( pal1.m_count >= 256 )
			{
				// Destination palette is full
				return false;
			}
			pal1.m_colors[ pal1.m_count ] = pal2.m_colors[i];
			newIndex = pal1.m_count;
			pal1.m_count++;
		}
		conv[i] = uint8(newIndex);
	}
	return true;
}

void PaletteTranslator::BuildDuplicatedColors(Palette& pal, uint32 pColCounts[256])
{
	Palette result;
	MergePalette(result, pal);

	// Update color counts if needed
	if( pColCounts != nullptr)
	{
		uint32 newColCounts[256];
		Memory::Zero32(newColCounts, 256);

		for(int i = 0; i < pal.m_count; ++i)
		{
			newColCounts[conv[i]] += pColCounts[i];
		}
		Memory::Copy32(pColCounts, newColCounts, result.m_count);
		Memory::Zero32(pColCounts + result.m_count, pal.m_count - result.m_count);
	}
	pal = result;
}

// Combines two translators in one, to perform multiple translations in one pass
// pt0 : First pass to apply
// pt1 : Second pass to apply
PaletteTranslator PaletteTranslator::Combine(const PaletteTranslator& pt0, const PaletteTranslator& pt1)
{
	PaletteTranslator result;
	for(int i = 0; i < 256; ++i)
	{
		result.conv[i] = pt1.conv[ pt0.conv[i] ];
	}
	return result;
}

// Update color count according to this translator
void PaletteTranslator::UpdateCounts(uint32* pCounts) const
{
	uint32 newCounts[256];
	const uint8* pConvPtr = conv;
	for(int i = 0; i < 256; ++i)
	{
		uint32 oldValue = pCounts[i];
		int newIndex = pConvPtr[i];
		newCounts[newIndex] = oldValue;
	}
	memcpy(pCounts, newCounts, sizeof(newCounts));
}

// To transform palette indexes into greyscale intensities
void PaletteTranslator::BuildGreyscale(const Palette& pal)
{
	for(int i = 0; i < 256; ++i)
	{
		conv[i] = pal[i].r;
	}
}
