///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "String.h"
#include "File.h"

#include "ImageLoader.h"

#include "Png.h"
#include "Bmp.h"
#include "Gif.h"
#include "Jpeg.h"
#include "Tga.h"

///////////////////////////////////////////////////////////////////////////////
using namespace chustd;
///////////////////////////////////////////////////////////////////////////////

ImageLoader::ImageLoader() : m_pImageType(null), m_type(Type_None)
{

}

ImageLoader::~ImageLoader()
{
	delete m_pImageType;
}

bool ImageLoader::Load(const chustd::String& filePath)
{
	File file;
	if( !file.Open(filePath) )
		return false;

	return LoadFromFile(file);
}

bool ImageLoader::LoadFromFile(IFile& file)
{
	if( !Instanciate(file) )
		return false;

	return m_pImageType->LoadFromFile(file);
}

bool ImageLoader::Instanciate(IFile& file)
{
	if( InstanciateLosslessFormat(file) )
	{
		return true;
	}

	int64 pos = file.GetPosition();
	
	file.SetPosition(pos);
	if( Jpeg::IsJpeg(file) )
	{
		m_pImageType = new Jpeg;
		m_type = Type_Jpeg;
		file.SetPosition(pos);
		return true;
	}

	file.SetPosition(pos);
	return false;
}

// Exclude Jpeg format (optmize final executable size)
bool ImageLoader::InstanciateLosslessFormat(IFile& file)
{
	delete m_pImageType;
	m_pImageType = null;
	m_type = Type_None;

	int64 pos = file.GetPosition();
	
	if( Png::IsPng(file) )
	{
		m_pImageType = new Png;
		m_type = Type_Png;
		file.SetPosition(pos);
		return true;
	}
	
	file.SetPosition(pos);
	if( Gif::IsGif(file) )
	{
		m_pImageType = new Gif;
		m_type = Type_Gif;
		file.SetPosition(pos);
		return true;
	}

	file.SetPosition(pos);
	if( Bmp::IsBmp(file) )
	{
		m_pImageType = new Bmp;
		m_type = Type_Bmp;
		file.SetPosition(pos);
		return true;
	}

	file.SetPosition(pos);
	if( Tga::IsTga(file) )
	{
		m_pImageType = new Tga;
		m_type = Type_Tga;
		file.SetPosition(pos);
		return true;
	}

	file.SetPosition(pos);
	return false;
}
