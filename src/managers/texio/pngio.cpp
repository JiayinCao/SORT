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
#include "pngio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "thirdparty/png/lodepng.h"

// read data from file
bool PngIO::Read( const string& name , std::shared_ptr<ImgMemory>& mem )
{
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;
    
    //decode
    unsigned error = lodepng::decode(image, width, height, name);

    if( error )
        return false;

    // TGA pixels are in BGRA format.
    mem->m_iWidth = width;
    mem->m_iHeight = height;
    mem->m_ImgMem = std::unique_ptr<Spectrum[]>( new Spectrum[ width * height ] );

    for( unsigned i = 0 ; i < height ; i++ )
		for( unsigned j = 0 ; j < width ; j++ )
		{
            unsigned char r = image[4 * width * i + 4 * j + 0];
            unsigned char g = image[4 * width * i + 4 * j + 1];
            unsigned char b = image[4 * width * i + 4 * j + 2];
            //unsigned char a = image[4 * width * i + 4 * j + 3]; // to be supported in the future
            mem->m_ImgMem[i * width + j ] = Spectrum( r , g , b );
		}

	return true;
}

// output the texture into bmp file
bool PngIO::Write( const string& name , const Texture* tex )
{
    std::vector<unsigned char> image; //the raw pixels
    
    int width = tex->GetWidth();
    int height = tex->GetHeight();
    
    // allocate the memory
    image.resize( 4 * width * height );
    
    // load the pixels
    for( unsigned i = 0 ; i < tex->GetHeight() ; i++ )
        for( unsigned j = 0 ; j < tex->GetWidth() ; j++ )
        {
            Spectrum c = tex->GetColor( j , i );
            c = c.Clamp(); // no tone mapping yet
            image[4 * width * i + 4 * j + 0] = ( unsigned char ) (c.GetR() * 255.0f);
            image[4 * width * i + 4 * j + 1] = ( unsigned char ) (c.GetG() * 255.0f);
            image[4 * width * i + 4 * j + 2] = ( unsigned char ) (c.GetB() * 255.0f);
            image[4 * width * i + 4 * j + 3] = 255; // no alpha
        }

    unsigned error = lodepng::encode(name, image, width, height);
	return (error == 0);
}
