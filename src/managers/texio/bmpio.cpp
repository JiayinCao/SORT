/*
 * filename :	bmpio.cpp
 *
 * programmer : Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "bmpio.h"
#include "spectrum/spectrum.h"
#include <fstream>
#include "managers/logmanager.h"
#include "managers/texmanager.h"
#include "texture/texture.h"

//-------------------------------------------------
// define some useful structure
// bitmap info header
typedef struct
{
	unsigned long   bfSize;
	unsigned short  bfReserved1;
	unsigned short  bfReserved2;
	unsigned long   bfOffBits;
} BitMapFileHeader;
// bitmap info header
typedef struct
{
	unsigned long  biSize;
	long           biWidth;
	long           biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long  biCompression;
	unsigned long  biSizeImage;
	long           biXPelsPerMeter;
	long           biYPelsPerMeter;
	unsigned long  biClrUsed;
	unsigned long  biClrImportant;
} BitMapInfoHeader;

// output the bmp file
bool BmpIO::Write( const string& str , const Texture* tex )
{
	// check if 'str' and 'tex' are valid
	if( str.empty() || tex == 0 )
	{
		if( tex == 0 )
			LOG_WARNING<<"No texture , writing image failed."<<ENDL;
		return false;
	}

	if( tex->CanOutput() == false )
		return false;

	// get the size of the image
	int w = tex->GetWidth();
	int h = tex->GetHeight();

	// if either of the length of the edge is zero , return
	if( w == 0 || h == 0 )
	{
		LOG_WARNING<<"There is no length in one dimension of the image, writing image failed"<<ENDL;
		return false;
	}
	
	// the size for the image
	int bitcount = 3;
	int pitch = ( w * bitcount + 3 ) / 4 * 4;
	int bytes = pitch * h;

	// allocate the memory
	char* data = new char[bytes];
	memset( data , 0 , bytes * sizeof( char ) );
	for( int i = 0 ; i < h ; i++ )
		for( int j = 0 ; j < w ; j++ )
		{
			unsigned offset = ( h - i - 1 ) * pitch + j * bitcount;
			data[offset+0] = (unsigned char)( tex->GetColor( (int)j , (int)i ).GetB() * 255 );
			data[offset+1] = (unsigned char)( tex->GetColor( (int)j , (int)i ).GetG() * 255 );
			data[offset+2] = (unsigned char)( tex->GetColor( (int)j , (int)i ).GetR() * 255 );
		}

	// the type for the image
	unsigned short type;
	*((char*)&type) = 'B';
	*(((char*)&type) + 1 ) = 'M';

	// set the bitmap header
	BitMapInfoHeader header;
	memset( &header , 0 , sizeof( header ) );
	header.biWidth = w;
	header.biHeight = h;
	header.biSize = sizeof( header );
	header.biPlanes = 1;
	header.biBitCount = 24;
	header.biCompression = 0L;

	// file header
	BitMapFileHeader bmfh;
	memset( &bmfh , 0 , sizeof( bmfh ) );
	bmfh.bfOffBits = sizeof( bmfh ) + sizeof( header ) + sizeof( type ) ;
	bmfh.bfSize = bmfh.bfOffBits + bytes;

	// open the file
	ofstream file;
	file.open( str.c_str() , ios::binary );

	// if the file could not be opened , just return an LOG_ERROR
	if( file.is_open() == false )
		return false;

	// output the information to file
	file.write( (const char*)&type , sizeof( type ) );
	file.write( (const char*)&bmfh , sizeof( bmfh ) );
	file.write( (const char*)&header , sizeof( header ) );
	file.write( (const char*)data , bytes * sizeof( char ) );

	// close it
	file.close();

	delete[] data;

	// output log
	LOG<<"Write image file : \""<<str<<"\""<<ENDL;

	return true;
}

// read data from file
bool BmpIO::Read( const string& str , ImgMemory* mem )
{
	// check if 'str' and 'tex' are valid
	if( str.empty() || mem == 0 )
	{
		if( mem == 0 )
			LOG_WARNING<<"No memory , reading image failed."<<ENDL;
		return false;
	}

	// open the file
	ifstream file;
	file.open( str.c_str() , ios::binary );

	// if the file could not be opened , just return an LOG_ERROR
	if( file.is_open() == false )
		return false;

	// the neccessary information
	unsigned short type;
	BitMapInfoHeader header;
	BitMapFileHeader bmfh;

	// output the information to file
	file.read( (char*)&type , sizeof( type ) );
	file.read( (char*)&bmfh , sizeof( bmfh ) );
	file.read( (char*)&header , sizeof( header ) );

	// get the size
	int w = header.biWidth;
	int h = header.biHeight;
	
	// the bit count for the image
	int bitcount = header.biBitCount / 8;

	// if either of the length of the edge is zero , return
	if( w == 0 || h == 0 )
	{
		LOG_WARNING<<"There is no length in one dimension of the image, reading image failed."<<ENDL;
		file.close();
		return false;
	}
	
	// the size for the image
	int pitch = ( w * bitcount + 3 ) / 4 * 4;
	int bytes = pitch * h;

	// allocate the memory
	char* data = new char[bytes];
	mem->m_ImgMem = new Spectrum[w*h];
	// read the data
	file.read( (char*)data , bytes * sizeof( char ) );
	for( int i = 0 ; i < h ; i++ )
		for( int j = 0 ; j < w ; j++ )
		{
			unsigned offset = ( h - i - 1 ) * pitch + j * bitcount;
			unsigned* color = (unsigned*)&data[offset];
			unsigned c = (*color) & 0x00ffffff;
			mem->m_ImgMem[i * w + j].SetColor( c );
		}

	// set the width and heigth
	mem->m_iWidth = w;
	mem->m_iHeight = h;

	// close file
	file.close();

	// delete the data
	delete[] data;

	// output log
	LOG<<"Read image file : \""<<str<<"\""<<ENDL;

	return true;
}
