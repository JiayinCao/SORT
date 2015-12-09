/*
   FileName:      pngio.cpp

   Created Time:  2015-12-9

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "pngio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "thirdparty/png/lodepng.h"

// read data from file
bool PngIO::Read( const string& name , ImgMemory* mem )
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
    mem->m_ImgMem = new Spectrum[ width * height ];

    for( int i = 0 ; i < height ; i++ )
		for( int j = 0 ; j < width ; j++ )
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
    for( int i = 0 ; i < tex->GetHeight() ; i++ )
        for( int j = 0 ; j < tex->GetWidth() ; j++ )
        {
            Spectrum c = tex->GetColor( j , i );
            c = c.Clamp(); // no tone mapping yet
            image[4 * width * i + 4 * j + 0] = ( unsigned char ) (c.GetR() * 255.0f);
            image[4 * width * i + 4 * j + 1] = ( unsigned char ) (c.GetG() * 255.0f);
            image[4 * width * i + 4 * j + 2] = ( unsigned char ) (c.GetB() * 255.0f);
            image[4 * width * i + 4 * j + 3] = 255; // no alpha
        }

    unsigned error = lodepng::encode(name, image, width, height);
	return error = 0;
}