/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2017 by Cao Jiayin - All rights reserved.
 
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
#include "jpgio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "thirdparty/libjpg/jpgd.h"
#include "thirdparty/libjpg/jpge.h"

// read data from file
bool JpgIO::Read( const string& name , std::shared_ptr<ImgMemory>& mem )
{
	const int channel = 3;
	int width = 0;
	int height = 0;
	int numcolors = 0;
	unsigned char* data = jpgd::decompress_jpeg_image_from_file(name.c_str(), &width, &height, &numcolors, channel);

	mem->m_iWidth = width;
	mem->m_iHeight = height;

	// TGA pixels are in BGRA format.
	mem->m_ImgMem = std::unique_ptr<Spectrum[]>( new Spectrum[width * height] );

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

	delete[] data;

	return false;
}
