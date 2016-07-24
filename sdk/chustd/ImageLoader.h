///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_IMAGELOADER_H
#define CHUSTD_IMAGELOADER_H

namespace chustd {

class ImageFormat;

// Loads the right imagetype
class ImageLoader
{
public:
	bool Load(const chustd::String& filePath);
	bool LoadFromFile(IFile& file);
	bool Instanciate(IFile& file);
	bool InstanciateLosslessFormat(IFile& file);

	ImageLoader();
	~ImageLoader();

public:
	ImageFormat* m_pImageType;
	enum Type
	{
		Type_None, Type_Bmp, Type_Gif, Type_Png, Type_Jpeg, Type_Tga
	};
	Type m_type;
};

} // namespace chustd

#endif // ndef CHUSTD_IMAGELOADER_H
