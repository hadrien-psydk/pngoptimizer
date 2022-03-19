/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
/////////////////////////////////////////////////////////////////////////////////////
#ifndef POENG_PALETTETRANSLATOR_H
#define POENG_PALETTETRANSLATOR_H

using namespace chustd;

/////////////////////////////////////////////////////////////////////////////////////////
// When a palette is reorganized, this helps to update the pixels to match the new palette
struct PaletteTranslator
{
	uint8 conv[256];

	PaletteTranslator() { BuildIdentity(); }

	void BuildIdentity();
	void Translate(uint8* pPixels, int pixelCount) const;
	void TranslateAll(PngDumpData& dd) const;
	void BuildSortAlpha(Palette& palette, uint32 pColCounts[256]);
	void BuildUnusedColors(Palette& palette, uint32* pCounts);
	void BuildSortPopulation(Palette& palette, uint32* pColCounts);
	void BuildSortLuminance(Palette& palette, uint32* pCounts);
	bool MergePalette(Palette& pal1, const Palette& pal2);
	void BuildDuplicatedColors(Palette& pal, uint32 pColCounts[256]);
	void BuildGreyscale(const Palette& pal);

	static PaletteTranslator Combine(const PaletteTranslator& pt0, const PaletteTranslator& pt1);

private:
	// Update color count according to this translator
	void UpdateCounts(uint32* pCounts) const;
};

#endif
