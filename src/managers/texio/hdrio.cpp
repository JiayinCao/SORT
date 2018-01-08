/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include the header
#include "hdrio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "thirdparty/hdrlib/hdrloader.h"

// read data from file
bool HdrIO::Read( const string& name , std::shared_ptr<ImgMemory>& mem )
{
	HDRLoaderResult result;
	bool ret = HDRLoader::load(name.c_str(), result);
	if (!ret)
		return false;

	mem->m_iWidth = result.width;
	mem->m_iHeight = result.height;

	int totalsize = result.width * result.height;
    mem->m_ImgMem = std::unique_ptr<Spectrum[]>( new Spectrum[ totalsize ] );

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
