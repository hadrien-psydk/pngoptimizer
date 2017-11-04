///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "File.h"
#include "StringBuilder.h"
#include "TextEncoding.h"

namespace chustd {\

///////////////////////////////////////////////////////////////////////////////
File::File()
{
	m_openMode = 0;
}

///////////////////////////////////////////////////////////////////////////////
File::~File()
{
	Close();
}

///////////////////////////////////////////////////////////////////////////////
bool File::Open(const String& filePath, uint32 mode/*=modeRead*/)
{
	if( filePath.IsEmpty() )
		return false;

	bool append = (mode & modeAppend) != 0;
	if( append )
	{
		// In append mode, the file is implicitely open in write mode
		mode |= modeWrite;
	}
	bool openRead = (mode & modeRead) != 0;
	bool openWrite = (mode & modeWrite) != 0;
	m_openMode = mode;

#if defined(_WIN32)
	int access = 0;
	int disposition = 0;
	int shareMode = FILE_SHARE_READ;

	if( !append )
	{
		if( openRead && !openWrite )
		{
			// Read only on an existing file
			access = GENERIC_READ;
			disposition = OPEN_EXISTING;
		}
		else if( !openRead && openWrite )
		{
			// Write only on a unexisting or existing file
			access = GENERIC_WRITE;
			disposition = CREATE_ALWAYS;
		}
		else if( openRead && openWrite )
		{
			// If the file exists, open it and set at the start of it
			// If the file does not exist, create it
			access = GENERIC_READ | GENERIC_WRITE;
			disposition = OPEN_ALWAYS;
		}
		else
		{
			// Bad open mode
			return false;
		}
	}
	else
	{
		// Note : the append mode in chustd has a different behaviour in comparison with the Libc:
		// A call to SetPosition followed by a call to Write, writes data at the position specified by SetPosition.
		// The Libc seeks to the end of the file before writing.
		access = GENERIC_WRITE;

		if( openRead )
		{
			access |= GENERIC_READ;
		}
		disposition = OPEN_ALWAYS;
	}

	HANDLE handle = CreateFileW(filePath.GetBuffer(),
		access, shareMode, nullptr, disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
	if( handle == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	if( append )
	{
		DWORD ret = SetFilePointer(handle, 0, 0, FILE_END);
		ret;
	}
	m_impl.handle = handle;

#elif defined(__linux__)
	int flags = 0;
	mode_t linuxMode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

	if( !append )
	{
		if( openRead && !openWrite )
		{
			// Read only on an existing file
			flags = O_RDONLY;
		}
		else if( !openRead && openWrite )
		{
			// Write only on a unexisting or existing file
			flags = O_WRONLY|O_CREAT|O_TRUNC;
		}
		else if( openRead && openWrite )
		{
			// If the file exists, open it and set at the start of it
			// If the file does not exist, create it
			flags = O_RDWR|O_CREAT;
		}
		else
		{
			// Bad open mode
			return false;
		}
	}
	else
	{
		// Write only on a unexisting or existing file
		// Do not set the size to 0 (omit O_TRUNC)
		if( openRead )
		{
			flags = O_RDWR|O_CREAT;
		}
		else
		{
			flags = O_WRONLY|O_CREAT;
		}
	}

	char filePath8[260];
	if( !filePath.ToUtf8Z(filePath8) )
	{
		return false;
	}

	int fd = open(filePath8, flags, linuxMode);
	if( fd < 0 )
	{
		return false;
	}
	if( append )
	{
		lseek(fd, 0, SEEK_END);
	}
	m_impl.fd = fd;

#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////
void File::Close()
{
	if( !m_impl.IsValid() )
		return;

#if defined(_WIN32)
	CloseHandle(m_impl.handle);
#elif defined(__linux__)
	close(m_impl.fd);
#endif

	m_impl.Clear();
}

///////////////////////////////////////////////////////////////////////////////
ByteOrder File::GetByteOrder() const
{
	if( !m_impl.IsValid() )
	{
		return boBigEndian;
	}

	if( (m_openMode & modeLittleEndian) != 0 )
	{
		return boLittleEndian;
	}
	return boBigEndian;
}

///////////////////////////////////////////////////////////////////////////////
// Sets the byte order when reading values greater than 8bits to automatically
// swap bytes.
void File::SetByteOrder(ByteOrder byteOrder)
{
	if( !m_impl.IsValid() )
		return;

	if( byteOrder == boLittleEndian )
	{
		m_openMode |= modeLittleEndian;
	}
	else if( byteOrder == boBigEndian )
	{
		m_openMode &= (~modeLittleEndian);
	}
	else
	{
		ASSERT(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
int File::Read(void* pBuffer, int size)
{
	if( !m_impl.IsValid() )
		return -1;

	if( size < 0 )
		return -2;

	if( size == 0 )
		return 0; // Not an error

#if defined(_WIN32)
	DWORD read = 0;
	BOOL bRet = ReadFile(m_impl.handle, pBuffer, size, &read, nullptr);
	if( !bRet )
	{
		return -3;
	}

#elif defined(__linux__)
	int read = ::read(m_impl.fd, pBuffer, size);
#endif
	return int(read);
}

///////////////////////////////////////////////////////////////////////////////
int File::Write(const void* pBuffer, int size)
{
	if( !m_impl.IsValid() )
		return -1;

	if( size < 0 )
		return -2;

	if( size == 0 )
		return 0; // Not an error

#if defined(_WIN32)
	DWORD written = 0;
	BOOL bRet = WriteFile(m_impl.handle, pBuffer, size, &written, nullptr);
	if( !bRet )
	{
		return -3;
	}

#elif defined(__linux__)
	int written = ::write(m_impl.fd, pBuffer, size);
#endif
	return int(written);
}

///////////////////////////////////////////////////////////////////////////////
bool File::SetPosition(int64 offset, Whence eWhence)
{
	if( !m_impl.IsValid() )
		return false;

#if defined(_WIN32)
	int32 nCFrom = 0;
	switch(eWhence)
	{
	case posBegin:
		nCFrom = FILE_BEGIN;
		break;
	case posCurrent:
		nCFrom = FILE_CURRENT;
		break;
	case posEnd:
		nCFrom = FILE_END;
		break;
	default:
		ASSERT(0); // Bad value for seek
		break;
	}

	LONG high = LONG(offset >> 32);
	DWORD low = DWORD(offset & 0x00000000ffffffff);
	DWORD ret = SetFilePointer(m_impl.handle, low, &high, nCFrom);

	DWORD error = 0;
	if( ret == INVALID_SET_FILE_POINTER && (error = GetLastError()) != NO_ERROR )
	{
		return false;
	}
	return true;

#elif defined(__linux__)
	int whence = 0;
	switch(eWhence)
	{
	case posBegin:   whence = SEEK_SET; break;
	case posCurrent: whence = SEEK_CUR; break;
	case posEnd:     whence = SEEK_END; break;
	default:
		ASSERT(0); // Bad value for seek
		break;
	}

	const int ret = lseek(m_impl.fd, offset, whence);
	return (ret >= 0);
#endif
}

///////////////////////////////////////////////////////////////////////////////
int64 File::GetPosition() const
{
	if( !m_impl.IsValid() )
		return 0;

#if defined(_WIN32)
	LONG high = 0;
	DWORD ret = SetFilePointer(m_impl.handle, 0, &high, FILE_CURRENT);

	DWORD error = 0;
	if( ret == INVALID_SET_FILE_POINTER && (error = GetLastError()) != NO_ERROR )
	{
		return 0;
	}

	int64 position = high;
	position <<= 32;
	position |= ret;

	return position;

#elif defined(__linux__)
	return lseek(m_impl.fd, 0, SEEK_CUR);
#endif
}

///////////////////////////////////////////////////////////////////////////////
int64 File::GetSize()
{
	if( !m_impl.IsValid() )
		return 0;

#if defined(_WIN32)
	DWORD sizeHigh = 0;
	DWORD sizeLow = GetFileSize(m_impl.handle, &sizeHigh);

	int64 length = sizeHigh;
	length <<= 32;
	length |= sizeLow;

	return length;

#elif defined(__linux__)
	struct stat stat;
	if( fstat(m_impl.fd, &stat) != 0 )
	{
		return 0;
	}
	return stat.st_size;
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool File::Rename(const String& oldName, const String& newName)
{
#if defined(_WIN32)
	BOOL bRet = MoveFileW(oldName.GetBuffer(), newName.GetBuffer());
	return bRet != FALSE;

#elif defined(__linux__)
	char oldName8[260];
	if( !oldName.ToUtf8Z(oldName8) )
	{
		return false;
	}
	char newName8[260];
	if( !newName.ToUtf8Z(newName8) )
	{
		return false;
	}
	return rename(oldName8, newName8) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
String File::GetDrive(const String& filePath)
{
	const int32 length = filePath.GetLength();
	if( length == 0 )
	{
		return String();
	}

	uint16 c = 0;

	int32 iSeparator = 0;
	for(; iSeparator < length; ++iSeparator)
	{
		c = filePath.GetAt(iSeparator);
		if( c == '\\' || c == '/' || c == ':' )
		{
			break;
		}
	}

	if( iSeparator < 0 )
	{
		// No separator found
		return String();
	}

	if( c == ':' )
	{
		// Drive found
		return filePath.Left(iSeparator);
	}

	return String();
}

///////////////////////////////////////////////////////////////////////////////
// Cuts string to parent's directory
// Does not leave the final separator
///////////////////////////////////////////////////////////////////////////////
String File::CutToParent(const String& filePath)
{
	int32 length = filePath.GetLength();

	const wchar* pszPath = filePath.GetBuffer();

	// Is last character a separator ?
	wchar c = pszPath[length - 1];
	if( c == L':' )
		return filePath; // A partition, do not modify the string

	if( c == L'\\' || c == L'/' )
		length--; // Yep, do not count the separator

	int32 i = length - 1;
	for(;i >= 0; i--)
	{
		if( pszPath[i] == L':' || pszPath[i] == L'\\' || pszPath[i] == L'/' )
			break;
	}

	return filePath.Left(i + 1);
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if a path name is absolute
//
// plop/onk.txt returns false
// ../plop/onk.txt returns false
//
// file://plop/onk.txt returns true
// d:/plop/onk.txt returns true
// d:onk.txt returns true
// /plop/onk.txt returns true
///////////////////////////////////////////////////////////////////////////////
bool File::IsAbsolutePath(const String& filePath)
{
	const int32 length = filePath.GetLength();
	if( length == 0 )
		return false;

	const wchar* pszPath = filePath.GetBuffer();

	bool bSeparatorFound = false;
	int32 i = 0;
	for(; i < length; i++)
	{
		if( pszPath[i] == '/'
			|| pszPath[i] == '\\'
			|| pszPath[i] == ':')
		{
			bSeparatorFound = true;
			break;
		}
	}

	if( !bSeparatorFound )
		return false;

	if( pszPath[i] == ':' )
		return true;

	if( pszPath[0] == '/' || pszPath[0] == '\\' )
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Non static
// 2 Go Max
ByteArray File::GetContent()
{
	const int64 currentPosition = GetPosition();
	int64 fileLength = GetSize();

	if( fileLength > 0x000000007fffffff )
	{
		fileLength = 0x000000007fffffff;
	}

	ByteArray content;
	content.SetSize( int(fileLength));

	Read(content.GetPtr(), int(fileLength));

	SetPosition(currentPosition);
	return content;
}

///////////////////////////////////////////////////////////////////////////////
// Static
ByteArray File::GetContent(const String& filePath)
{
	File file;
	if( !file.Open(filePath, File::modeRead) )
	{
		return ByteArray();
	}

	ByteArray content = file.GetContent();
	return content;
}

///////////////////////////////////////////////////////////////////////////////
// Static
bool File::SetContent(const String& filePath, const ByteArray& content)
{
	File file;
	if( !file.Open(filePath, File::modeWrite) )
	{
		return false;
	}
	return content.GetSize() == file.Write(content.GetPtr(), content.GetSize());
}

///////////////////////////////////////////////////////////////////////////////
// Non static
String File::GetTextContent(const TextEncoding& te)
{
	ByteArray aContent = GetContent();
	return String::FromBytes(aContent, te);
}

///////////////////////////////////////////////////////////////////////////////
// Static
String File::GetTextContent(const String& filePath, const TextEncoding& te)
{
	ByteArray aContent = GetContent(filePath);
	return String::FromBytes(aContent, te);
}

///////////////////////////////////////////////////////////////////////////////
// Non static
StringArray File::GetLines(const TextEncoding& te)
{
	String content = GetTextContent(te);
	StringArray lines = content.SplitByEndlines();
	return lines;
}

///////////////////////////////////////////////////////////////////////////////
// Static
StringArray File::GetLines(const String& filePath, const TextEncoding& te)
{
	File file;
	if( !file.Open(filePath, File::modeRead) )
	{
		return StringArray();
	}

	StringArray lines = file.GetLines(te);
	return lines;
}

///////////////////////////////////////////////////////////////////////////////
bool File::Exists(const String& filePath)
{
	bool bDirectory, bReadOnly;
	return GetFileAttributes(filePath, bDirectory, bReadOnly) && !bDirectory;
}

///////////////////////////////////////////////////////////////////////////////
int64 File::GetSize(const String& filePath)
{
	File file;
	if( !file.Open(filePath) )
		return 0;

	return file.GetSize();
}

///////////////////////////////////////////////////////////////////////////////
DateTime File::GetLastWriteTime()
{
	if( !m_impl.IsValid() )
		return DateTime();

#if defined(_WIN32)
	uint64 lastWriteTime = 0;
	FILETIME timeLastWrite = {0};
	if( ::GetFileTime(m_impl.handle, nullptr, nullptr, &timeLastWrite) )
	{
		lastWriteTime = *((uint64*)&timeLastWrite);
	}
	lastWriteTime /= 10000;
	return DateTime::FromChuTimeStamp(lastWriteTime);

#elif defined(__linux__)
	struct stat stat;
	if( fstat(m_impl.fd, &stat) != 0 )
	{
		return DateTime();
	}
	return DateTime::FromUnixTimeStamp(stat.st_mtim.tv_sec, stat.st_mtim.tv_nsec);
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool File::SetLastWriteTime(const DateTime& dt)
{
	if( !m_impl.IsValid() )
		return false;

#if defined(_WIN32)
	uint64 ts = dt.GetChuTimeStamp() * 10000;
	FILETIME timeLastWrite = *((FILETIME*)&ts);
	return ::SetFileTime(m_impl.handle, nullptr, nullptr, &timeLastWrite) != FALSE;

#elif defined(__linux__)
	int nsec = 0;
	time_t t = dt.ToUnixTimeStamp(&nsec);
	struct timespec ts[2]; // 0: last access, 1: last modification
	ts[0].tv_sec = t;
	ts[0].tv_nsec = nsec;
	ts[1] = ts[0];
	return futimens(m_impl.fd, ts) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
String File::GetAbsolutePath(const String& strCurrentDir, const String& strRelativePath, wchar cModel)
{
	wchar szModel[2] = {cModel, 0};

	ASSERT(strCurrentDir.StartsWith(szModel));

	String filePath;

	if( strRelativePath.StartsWith(szModel) )
	{
		// Starts with the root dir, no need to cat with strCurrentDir
		filePath = strRelativePath;
	}
	else
	{
		filePath = FilePath::AddSeparator(strCurrentDir);
		filePath = filePath + strRelativePath;
	}

	StringArray astrFragments = filePath.Split(cModel);
	StringArray astrStack;

	foreach(astrFragments, i)
	{
		String& strFrag = astrFragments[i];
		if( strFrag == ".." )
		{
			// Before : plop/abc/..
			// After:   plop/
			astrStack.RemoveLast();
		}
		else if( strFrag == "." )
		{
			// Before : plop/abc/.
			// After:   plop/abc/
		}
		else
		{
			if( !strFrag.IsEmpty() )
			{
				astrStack.Add(strFrag);
			}
		}
	}

	if( astrStack.GetSize() == 0 )
		return String(szModel, 1);

	StringBuilder sbResult;
	foreach(astrStack, i)
	{
		sbResult += cModel;
		sbResult += astrStack[i];
	}
	return sbResult.ToString();
}

///////////////////////////////////////////////////////////////////////////////
String File::GetAbsolutePath(const String& filePath)
{
#if defined(_WIN32)
	const int length = ::GetFullPathName(filePath.GetBuffer(), 0, nullptr, nullptr);

	wchar* pBuffer = new wchar[length];
	wchar* pFilePart;
	::GetFullPathName(filePath.GetBuffer(), length, pBuffer, &pFilePart);

	String strFullPathName(pBuffer);

	delete[] pBuffer;

	return strFullPathName;

#else
	return filePath;
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool File::Copy(const String& srcFilePath, const String& dstFilePath)
{
#if defined(_WIN32)
	return ::CopyFileW(srcFilePath.GetBuffer(),
	                   dstFilePath.GetBuffer(), FALSE) != 0;

#else
	char tmp1[400];
	if( !srcFilePath.ToUtf8Z(tmp1) )
	{
		return false;
	}

	char tmp2[400];
	if( !dstFilePath.ToUtf8Z(tmp2) )
	{
		return false;
	}

	int source = open(tmp1, O_RDONLY, 0);
	if( source < 0 )
	{
		return false;
	}
	int dest = open(tmp2, O_WRONLY | O_CREAT, 0644);
	if( dest < 0 )
	{
		return false;
	}

	struct stat stat;
	fstat(source, &stat);

	sendfile(dest, source, nullptr, stat.st_size);

	close(source);
	close(dest);
	return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool File::GetFileAttributes(const String& filePath, bool& isDirectory, bool& readOnly)
{
#if defined(_WIN32)
	DWORD dwAttributes = ::GetFileAttributesW(filePath.GetBuffer());
	if( dwAttributes == INVALID_FILE_ATTRIBUTES )
		return false;

	isDirectory = (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	readOnly = (dwAttributes & FILE_ATTRIBUTE_READONLY) != 0;

	return true;

#elif defined(__linux__)
	char path8[400];
	if( !filePath.ToUtf8Z(path8) )
	{
		return false;
	}
	struct stat st;
	if( stat(path8, &st) != 0 )
	{
		return false;
	}
	isDirectory = S_ISDIR(st.st_mode);
	readOnly = !bool(st.st_mode & S_IWUSR);
	return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool File::Delete(const String& filePath)
{
#if defined(_WIN32)
	return ::DeleteFileW(filePath.GetBuffer()) != FALSE;

#elif defined(__linux__)
	char path8[400];
	if( !filePath.ToUtf8Z(path8) )
	{
		return false;
	}
	return unlink(path8) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
bool File::WriteTextUtf8(const String& filePath, const String& content)
{
	File file;
	if( !file.Open(filePath, modeWrite) )
	{
		return false;
	}
	static const uint8 bom[] = { 0xEF, 0xBB, 0xBF };
	if( file.Write(bom, sizeof(bom)) != sizeof(bom) )
	{
		return false;
	}
	if( !file.WriteString(content, TextEncoding::Utf8()) )
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool File::SetReadOnly(const String& filePath, bool readOnly)
{
#if defined(_WIN32)
	DWORD dwAttributes = ::GetFileAttributesW(filePath.GetBuffer());
	if( dwAttributes == INVALID_FILE_ATTRIBUTES )
		return false;

	if( readOnly )
	{
		dwAttributes |= FILE_ATTRIBUTES_READONLY;
	}
	else
	{
		dwAttributes &= ~FILE_ATTRIBUTES_READONLY;
	}
	return ::SetFileAttributesW(filePath.GetBuffer(), dwAttributes) != FALSE;

#elif defined(__linux__)
	char path8[400];
	if( !filePath.ToUtf8Z(path8) )
	{
		return false;
	}
	struct stat st;
	if( stat(path8, &st) != 0 )
	{
		return false;
	}

	if( readOnly )
	{
		st.st_mode &= ~S_IWUSR;
	}
	else
	{
		st.st_mode |= S_IWUSR;
	}

	return chmod(path8, st.st_mode) == 0;
#endif

}

///////////////////////////////////////////////////////////////////////////////
}

