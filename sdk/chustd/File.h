///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_FILE_H
#define CHUSTD_FILE_H

#include "IFile.h"
#include "String.h"
#include "Array.h"
#include "FilePath.h"
#include "DateTime.h"

namespace chustd {\

class File : public IFile
{
public:
	///////////////////////////////////////////////////////////////////////
	virtual bool  SetPosition(int64 offset, Whence eWhence = posBegin);
	virtual int64 GetPosition() const;
	virtual int64 GetSize();
	virtual int Read(void* pBuffer, int size);
	virtual int Write(const void* pBuffer, int size);
	virtual ByteOrder GetByteOrder() const;
	virtual void SetByteOrder(ByteOrder byteOrder);
	virtual void Close();
	///////////////////////////////////////////////////////////////////////

	// Open a file. Default flags are :
	// Read mode + Binary mode + Big Endian mode
	bool Open(const String& filePath, uint32 mode = modeRead);

	// Copy the whole file content to a buffer
	ByteArray GetContent();

	// Copy the whole file content to a string
	String GetTextContent(const TextEncoding& te);

	// Copy all the lines of the file to a string array
	StringArray GetLines(const TextEncoding& te);

	DateTime GetLastWriteTime();
	bool     SetLastWriteTime(const DateTime& dt);

	File();
	virtual ~File();

public:
	// Does a copy the whole file content of a file to a buffer
	static ByteArray GetContent(const String& filePath);

	static bool SetContent(const String& filePath, const ByteArray& content);

	// Does a copy the whole file content of a file to a string
	static String GetTextContent(const String& filePath, const TextEncoding& te);

	// Does a copy all the lines of the into a string array
	static StringArray GetLines(const String& filePath, const TextEncoding& te);

	// Returns true if a path is absolute
	static bool IsAbsolutePath(const String& filePath);

	// Removes the last level of a path
	static String CutToParent(const String& filePath);

	// Deletes a file
	static bool Delete(const String& filePath);

	// Renames a file, returns true if the rename occured
	static bool Rename(const String& strOldName, const String& strNewName);

	// Performs a file copy, returns true if the copy occured
	static bool Copy(const String& srcFilePath, const String& dstFilePath);

	// Gets the drive part of a path (the result can be empty if no drive name is in the path)
	static String GetDrive(const String& filePath);

	// Returns the absolute file name using the current directory of the system
	static String GetAbsolutePath(const String& filePath);
	static String GetAbsolutePath(const String& strCurrentDir, const String& filePath, wchar cModel = '/');

	// Tests if a file exists
	// If the path describes a directory, this method returns false
	static bool Exists(const String& strPath);

	// Gets the size of a file in bytes
	static int64 GetSize(const String& strPath);

	// Gets a file attributes
	static bool GetFileAttributes(const String& filePath, bool& isDirectory, bool& readOnly);

	static bool WriteTextUtf8(const String& filePath, const String& content);

	static bool SetReadOnly(const String& filePath, bool readOnly = true);

private:
	union FileImpl
	{
		void* handle; int fd;
		FileImpl() : handle((void*)-1) {}
		bool IsValid() const { return handle != (void*)-1; }
		void Clear() { handle = (void*)-1; }
	};
	FileImpl m_impl;
	uint32 m_openMode;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace chustd

#endif // ndef CHUSTD_FILE_H
