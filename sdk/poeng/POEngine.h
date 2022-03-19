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

	// General settings, freely accessible and used during optimization/conversion
	POEngineSettings m_settings;

public:
	// A PNG file signature is just the list of all its chunk CRCs
	typedef chustd::Array<uint32> PngSignature;

	struct OptiInfo
	{
		int sizeBefore;
		int sizeAfter;

		// Computed when inserting a clean version of the source file
		PngSignature srcSignature;

		// If sizes are the same and the content too, this
		// value will be true
		bool sameContent;

		OptiInfo() { Clear(); }
		void Clear()
		{
			sizeBefore = 0;
			sizeAfter = 0;
			srcSignature.Clear();
			sameContent = false;
		}
	};

	bool OptimizeMultiFilesDisk(const chustd::StringArray& filePaths, const chustd::String& joker = "");
	bool OptimizeFileDisk(const chustd::String& filePath, const chustd::String& displayDir, OptiInfo& optiInfo);
	bool OptimizeFileDiskNoBackup(const chustd::String& filePath, const chustd::String& newFilePath, OptiInfo& optiInfo);
	bool OptimizeExternalBuffer(const chustd::PngDumpData& ds, const chustd::String& filePath);
	bool OptimizeFileMem(const uint8* imgBuf, int imgSize, uint8* dst, int dstCapacity, int* pDstSize);
	bool OptimizeFileStdio();

	chustd::String GetLastErrorString() const;
	void ClearLastError();

	bool WarmUp();
	void EnableUnicodeArrow();

	static chustd::Color ColorFromTextType(TextType tt, bool darkTheme = false);

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

	// Holds source information
	struct SrcInfo
	{
		String filePath; // Input file to be optimized
		int    fileSize; // Input file size

		SrcInfo() : fileSize(0) {}
	};

	// Holds target information: stdout, a file path or a memory buffer
	struct OptiTarget
	{
		enum class Type { Stdout, File, Memory };
		Type   type;
		String filePath;
		void*  buf;
		int    bufCapacity;

		SrcInfo srcInfo;

		OptiTarget() : type(Type::Stdout)
		{
			buf = nullptr;
			bufCapacity = 0;
		}

		OptiTarget(const String& filePathArg)
			: type(Type::File), filePath(filePathArg)
		{
			buf = nullptr;
			bufCapacity = 0;
		}

		OptiTarget(void* bufArg, int bufCapacityArg)
			: type(Type::Memory), buf(bufArg), bufCapacity(bufCapacityArg) {}
	};

private:
	struct MultiOptiInfo
	{
		int optiCount;
		int errorCount;
		int64 sizeBefore;
		int64 sizeAfter;

		MultiOptiInfo()
		{
			optiCount = 0;
			errorCount = 0;
			sizeBefore = 0;
			sizeAfter = 0;
		}
	};
	void OptimizeFilesInternal(const String& baseDir, const StringArray& filePaths, const String& displayDir,
	                   const String& joker, MultiOptiInfo& optiInfo);

	bool Optimize(PngDumpData& dd, const OptiTarget& target, OptiInfo&);
	bool OptimizeFileStreamNoBackup(IFile& fileImage, const OptiTarget& target, OptiInfo&);

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
	bool DumpBestResultToFile(const OptiTarget& target, OptiInfo&);

	bool InsertCleanOriginalPngAsResult(IFile& file, PngSignature& oriSign);

	void AddError(const String& str);

	void PrintSizeChange(int64 sizeBefore, int64 sizeAfter, bool sameContent);
	void PrintText(const String& text, TextType textType);

	static void BgrToRgb(PngDumpData& dd);
	static void BgraToRgba(PngDumpData& dd);
	static bool Rgb16ToRgb24(PngDumpData& dd);

public:
	// public for unit testing
	bool OptimizeAnimated(const ImageFormat& img, PngDumpData& dd, const String& filePath, OptiInfo&);
private:
	bool OptimizeAnimated(const ImageFormat& img, PngDumpData& dd, const OptiTarget& target, OptiInfo&);
	bool PerformDumpTries(PngDumpData& ds);

	static void UnpackPixelFrames(PngDumpData& dd);
	static void PackPixelFrames(PngDumpData& dd);
	static bool IsFileExtensionSupported(const String& ext, const String& joker="");

	friend class POEngine_IsFileExtensionSupported_Test;
};

#endif
