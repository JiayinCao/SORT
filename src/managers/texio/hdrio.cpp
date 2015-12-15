/*
   FileName:      hdrio.cpp

   Created Time:  2015-12-8

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "hdrio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "thirdparty/hdrlib/hdrloader.h"

// read data from file
bool HdrIO::Read( const string& name , ImgMemory* mem )
{
	HDRLoaderResult result;
	bool ret = HDRLoader::load(name.c_str(), result);
	if (!ret)
		return false;

	mem->m_iWidth = result.width;
	mem->m_iHeight = result.height;

	int totalsize = result.width * result.height;
	mem->m_ImgMem = new Spectrum[ totalsize ];

	for( int i = 0 ; i < totalsize; ++i )
		mem->m_ImgMem[i] = Spectrum( result.cols[3*i] , result.cols[3*i+1], result.cols[3*i+2] );

	// release the memory
	delete[] result.cols;

	return true;
}

// output the texture into bmp file
bool HdrIO::Write( const string& name , const Texture* tex )
{
	std::ofstream hdr(name.c_str(), std::ios::binary);

	int mResY = tex->GetHeight();
	int mResX = tex->GetWidth();

	hdr << "#?RADIANCE" << '\n';
	hdr << "# SORT" << '\n';
	hdr << "FORMAT=32-bit_rle_rgbe" << '\n' << '\n';
	hdr << "-Y " << mResY << " +X " << mResX << '\n';

	for (int y = 0; y<mResY; y++)
	{
		for (int x = 0; x<mResX; x++)
		{
			typedef unsigned char byte;
			byte rgbe[4] = { 0,0,0,0 };

			const Spectrum &rgbF = tex->GetColor(x , y);
			float v = std::max(rgbF.GetR(), std::max(rgbF.GetG(), rgbF.GetB()));

			if (v >= 1e-32f)
			{
				int e;
				v = float(frexp(v, &e) * 256.f / v);
				rgbe[0] = byte(rgbF.GetR() * v);
				rgbe[1] = byte(rgbF.GetG() * v);
				rgbe[2] = byte(rgbF.GetB() * v);
				rgbe[3] = byte(e + 128);
			}

			hdr.write((char*)&rgbe[0], 4);
		}
	}

	hdr.close();

	return true;
}