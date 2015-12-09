/*
   FileName:      tgaio.cpp

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
#include "tgaio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "thirdparty/tga/targa.h"

// read data from file
bool TgaIO::Read( const string& name , ImgMemory* mem )
{
	tga_image img;
    tga_result result;
    if ((result = tga_read(&img, name.c_str())) != TGA_NOERR) {
        return false;
    }

    if (tga_is_right_to_left(&img))
        tga_flip_horiz(&img);
    if (!tga_is_top_to_bottom(&img))
        tga_flip_vert(&img);
    if (tga_is_colormapped(&img))
        tga_color_unmap(&img);

	mem->m_iWidth  = img.width;
	mem->m_iHeight = img.height;

    // TGA pixels are in BGRA format.
    mem->m_ImgMem = new Spectrum[ img.width * img.height ];

	for( int i = 0 ; i < img.height ; i++ )
		for( int j = 0 ; j < img.width ; j++ )
		{
			uint8_t *src = tga_find_pixel(&img, j, i);
			if (tga_is_mono(&img)){
				mem->m_ImgMem[i * img.width + j] = Spectrum( *src / 255.f );
			}
            else {
                float c[3];
                c[2] = src[0] / 255.f;
                c[1] = src[1] / 255.f;
                c[0] = src[2] / 255.f;
                mem->m_ImgMem[i * img.width + j] = Spectrum( c[0] , c[1], c[2] );
            }
		}

    tga_free_buffers(&img);

	return true;
}

// output the texture into bmp file
bool TgaIO::Write( const string& name , const Texture* tex )
{
    int width = tex->GetWidth();
    int height = tex->GetHeight();
    if( width == 0 || height == 0 )
        return false;
    
    // Reformat to BGR layout.
    uint8_t *outBuf = new uint8_t[3 * width * height];
    uint8_t *dst = outBuf;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Spectrum c = tex->GetColor(x, y);
            c = c.Clamp(); // no tone mapping yet
            dst[0] = ( unsigned char ) (c.GetB() * 255);
            dst[1] = ( unsigned char ) (c.GetG() * 255);
            dst[2] = ( unsigned char ) (c.GetR() * 255);
            dst += 3;
        }
    }
    
    tga_result result;
    result = tga_write_bgr(name.c_str(), outBuf, width, height, 24);
    
    delete[] outBuf;

	return result == TGA_NOERR;
}