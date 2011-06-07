/*
 * filename :	bmpsaver.cpp
 *
 * programmer : Cao Jiayin
 */

// include the header file
#include "bmpsaver.h"
#include "../../spectrum/spectrum.h"
#include <fstream>

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
bool BmpSaver::Output( const Texture* tex , const string& str )
{
	// get the size of the image
	int w = tex->GetWidth();
	int h = tex->GetHeight();

	// if either of the length of the edge is zero , return
	if( w == 0 || h == 0 )
		return false;
	
	// the size for the image
	int bytes = w * h; 

	// allocate the memory
	unsigned* data = new unsigned[bytes];
	memset( data , 0 , bytes * sizeof( unsigned ) );
	for( int i = 0 ; i < h ; i++ )
		for( int j = 0 ; j < w ; j++ )
		{
			unsigned offset = ( h - i - 1 ) * w + j;
			data[offset] = tex->GetColor( (int)j , (int)i ).GetColor();
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
	header.biBitCount = 32;
	header.biCompression = 0L;

	// file header
	BitMapFileHeader bmfh;
	memset( &bmfh , 0 , sizeof( bmfh ) );
	bmfh.bfOffBits = sizeof( bmfh ) + sizeof( header ) + sizeof( type ) ;
	bmfh.bfSize = bmfh.bfOffBits + bytes;

	// open the file
	ofstream file;
	file.open( str.c_str() , ios::binary );

	// if the file could not be opened , just return an error
	if( file.is_open() == false )
	{
		cout<<"Can't open file "<<str<<endl;
		return false;
	}

	// output the information to file
	file.write( (const char*)&type , sizeof( type ) );
	file.write( (const char*)&bmfh , sizeof( bmfh ) );
	file.write( (const char*)&header , sizeof( header ) );
	file.write( (const char*)data , bytes * sizeof( unsigned ) );

	// close it
	file.close();

	delete[] data;

	return true;
}
