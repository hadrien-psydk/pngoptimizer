/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the poeng library, part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// This library is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
// See License.txt for the full license.
/////////////////////////////////////////////////////////////////////////////////////
#ifndef POENG_POENGINE_H
#define POENG_POENGINE_H

using namespace chustd;

#include "POEngineSettings.h"
#include "POWorkerThread.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PNG optimizing engine class
class POEngine
{
public:
	// Color used when notifying progress
	enum TextType
	{
		TT_FilePath,      // black
		TT_RegularInfo,   // gray
		TT_ActionVerb,    // gray :  Creating, Converting, Optimizing...
		TT_SizeInfo,      // gray :  xxx bytes -> yyy bytes
		TT_SizeInfoNum,   // gray :  xxx in "xxx bytes"
		TT_FileEnlarged,  // amber : (103% of the original size)
		TT_FileSizeSame,  // gray :  (100% of the original size)
		TT_FileReduced,   // green : (80% of the original size)
		TT_ActionOk,      // green : (OK)
		TT_ActionFail,    // red :   (KO)
		TT_ErrorMsg,      // red :   could not load image
		TT_Animated,      // blue :  [Animated GIF : ...]
		TT_BatchDoneOk,   // green: more than two files optimization done
		TT_BatchDoneFail, // red:   more than two files optimization done with errors
		TT_Last
	};

	struct ProgressingArg
	{
		chustd::String  text;
		TextType        textType;
		ProgressingArg() : textType(TT_Last) {}
	};

	chustd::Event1<const ProgressingArg&> Progressing; // Fired during the optimization process

public:
	POEngineSettings m_settings;

public:
	bool OptimizeFiles(const chustd::StringArray& filePaths, const chustd::String& joker = "");
	bool OptimizeExternalBuffer(const chustd::PngDumpData& ds, const chustd::String& filePath);
	bool OptimizeFileNoBackup(const chustd::String& filePath, const chustd::String& newFilePath);

	// Gets error explanation when an optimization function fails
	chustd::String GetLastErrorString() const;
	void ClearLastError();

	struct SingleOptiInfo
	{
		uint32 sizeBefore;
		uint32 sizeAfter;

		SingleOptiInfo() { Clear(); }
		void Clear()
		{
			sizeBefore = 0;
			sizeAfter = 0;
		}
	};

	// Optimizes or converts one single file.
	bool OptimizeSingleFile(const chustd::String& filePath, const chustd::String& displayDir, SingleOptiInfo& singleOptiInfo);

	// Optimizes or converts one single file loaded in memory.
	bool OptimizeSingleFileMem(const uint8* imgBuf, int imgSize, uint8* dst, int dstCapacity, int* pDstSize);

	// Optimizes or converts a file coming from stdin and write result to stdout
	bool OptimizeStdio();

	static chustd::Color ColorFromTextType(TextType tt);

	// Initializes some structures. To be called before any optimization function to avoid making the first call
	// slower than other calls (first call will do the warmup if it is not done).
	bool WarmUp();

	void EnableUnicodeArrow();

	POEngine();
	virtual ~POEngine();

private:
	// This class allows the storing of the different tries done by POEngine
	// When dumping, we keep the best result of all
	class ResultManager
	{
	public:
		DynamicMemoryFile& GetCandidate(); // Gets the slot to test a new compression flavour
		DynamicMemoryFile& GetSmallest();  // Gets the best slot of all
		
		void Reset();

	private:
		DynamicMemoryFile m_dmf0;
		DynamicMemoryFile m_dmf1;
	};
	
	ResultManager m_resultmgr;
	
	// Last errors
	StringArray m_astrErrors;
	DateTime m_originalFileWriteTime;

	POWorkerThread m_workerThreads[4];

	bool m_unicodeArrowEnabled; // To have a nice arrow for ->

	// Holds target information: stdout, a file path or a memory buffer
	struct OptiTarget
	{
		enum class Type { Stdout, File, Memory };
		Type   type;
		String filePath;
		void*  buf;
		int    bufCapacity;
		int    size;

		OptiTarget() : type(Type::Stdout)
		{
			buf = nullptr;
			bufCapacity = 0;
			size = 0;
		}

		OptiTarget(const String& filePathArg) 
			: type(Type::File), filePath(filePathArg)
		{
			buf = nullptr;
			bufCapacity = 0;
			size = 0;
		}

		OptiTarget(void* bufArg, int bufCapacityArg) 
			: type(Type::Memory), buf(bufArg), bufCapacity(bufCapacityArg), size(0) {}
	};

private:
	struct OptiInfo
	{
		int optiCount;
		int errorCount;
		int64 sizeBefore;
		int64 sizeAfter;

		OptiInfo()
		{
			optiCount = 0;
			errorCount = 0;
			sizeBefore = 0;
			sizeAfter = 0;
		}
	};
	void OptimizeFilesInternal(const String& baseDir, const StringArray& filePaths, const String& displayDir, 
	                   const String& joker, OptiInfo& optiInfo);

	bool Optimize(PngDumpData& dd, OptiTarget& target);
	bool OptimizeSingleFileNoBackup(IFile& fileImage, OptiTarget& target);

	// Those functions fill the DynamicMemoryFiles of m_resultmgr
	bool OptimizePaletteMode(PngDumpData& dd);
	bool Optimize24BitsMode(PngDumpData& dd);
	bool Optimize32BitsMode(PngDumpData& dd);
	bool OptimizeGrayScale(PngDumpData& dd);
	bool OptimizeGrayScaleAlpha(PngDumpData& dd);

	int  CanSimplifyGreyAlpha(const PngDumpData& dd) const;
	bool IsBlackAndWhite(const Palette& pal, bool& bShouldSwap);
	bool IsGreyPalette(const Palette& pal);
	bool TryToConvertIndexedToBlackAndWhite(PngDumpData& dd);
	bool TryToConvertIndexedToGreyscale(PngDumpData& dd);
	bool FindUnusedColor(const Buffer& aRgb, uint8& nRed, uint8& nGreen, uint8& nBlue);
	bool FindUnusedColorHardcoreMethod(const uint8* pRgba, int32 nPixelCount, uint8& nRed, uint8& nGreen, uint8& nBlue);
	bool DumpBestResultToFile(OptiTarget& target);

	bool InsertCleanOriginalPngAsResult(IFile& file);

	void AddError(const String& str);

	void PrintSizeChange(int64 sizeBefore, int64 sizeAfter);
	void PrintText(const String& text, TextType textType);
	
	static void BgrToRgb(PngDumpData& dd);
	static void BgraToRgba(PngDumpData& dd);
	static bool Rgb16ToRgb24(PngDumpData& dd);

public:
	// public for unit testing
	bool OptimizeAnimated(const ImageFormat& img, PngDumpData& dd, const String& filePath);
private:
	bool OptimizeAnimated(const ImageFormat& img, PngDumpData& dd, OptiTarget& target);
	bool PerformDumpTries(PngDumpData& ds);

	static void UnpackPixelFrames(PngDumpData& dd);
	static void PackPixelFrames(PngDumpData& dd);
	static bool IsFileExtensionSupported(const String& ext, const String& joker="");
		
	friend class POEngine_IsFileExtensionSupported_Test;
};

#endif
