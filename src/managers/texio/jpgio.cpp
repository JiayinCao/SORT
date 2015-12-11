/*
   FileName:      jpgio.cpp

   Created Time:  2015-12-10

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "jpgio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "thirdparty/libjpg/jpgd.h"
#include "thirdparty/libjpg/jpge.h"

// read data from file
bool JpgIO::Read( const string& name , ImgMemory* mem )
{
	const int channel = 3;
	int width = 0;
	int height = 0;
	int numcolors = 0;
	unsigned char* data = jpgd::decompress_jpeg_image_from_file(name.c_str(), &width, &height, &numcolors, channel);

	mem->m_iWidth = width;
	mem->m_iHeight = height;

	// TGA pixels are in BGRA format.
	mem->m_ImgMem = new Spectrum[width * height];

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			float b = data[channel * (i * width + j)] / 255.0f;
			float g = data[channel * (i * width + j) + 1] / 255.0f;
			float r = data[channel * (i * width + j) + 2] / 255.0f;

			mem->m_ImgMem[i * width + j] = Spectrum( r , g , b );
		}

	// delete allocated data
	delete[] data;

	return true;
}

// output the texture into bmp file
bool JpgIO::Write( const string& name , const Texture* tex )
{
	const int channel = 3;
	int width = tex->GetWidth();
	int height = tex->GetHeight();
	unsigned char* data = new unsigned char[width * height * 3];

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			Spectrum c = tex->GetColor(j, i).Clamp();
			data[channel * (i * width + j)] = (unsigned char)(c.GetR() * 255);
			data[channel * (i * width + j) + 1] = (unsigned char)(c.GetG() * 255);
			data[channel * (i * width + j) + 2] = (unsigned char)(c.GetB() * 255);
		}

	jpge::compress_image_to_jpeg_file(name.c_str(), tex->GetWidth(), tex->GetHeight(), 3, data );

	delete data;

	return false;
}