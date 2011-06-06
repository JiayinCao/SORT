/*
 * filename :	bmpsaver.cpp
 *
 * programmer : Cao Jiayin
 */

// include the header file
#include "bmpsaver.h"
#include <fstream>

//-------------------------------------------------
// define some useful structure
// bitmap info header
typedef struct
{
	int 	bfType;
	long	bfSize;
	int	bfReserved1;
	int	bfReserved2;
	long	bfOffBits;
} BITMAPFILEHEADER;
// bitmap info header
typedef struct
{
	long	biSize;
	long	biWidth;
	long	biHeight;
	int	biPlanes;
	int	biBitCount;
	long	biCompression;
	long	biSizeImage;
	long	biXPelsPerMeter;
	long	biYPelsPerMeter;
	long	biClrUsed;
	long	biClrImportant;
} BITMAPINFOHEADER;

// output the bmp file
bool BmpSaver::Output( const Texture* tex , const string& str )
{
	// get the size of the image
	int w = 800;
	int h = 800;
	
	// the size for the image
	int bytes = w * h * 4; // temp

	char* data = new char[bytes];
	memset( data , 0 , bytes * sizeof( char ) );

	// set the bitmap header
	BITMAPINFOHEADER header;
	memset( &header , 0 , sizeof( header ) );
	header.biWidth = w;
	header.biHeight = h;
	header.biSize = sizeof( BITMAPINFOHEADER );
	header.biPlanes = 1;
	header.biBitCount = 32;
	header.biCompression = 0L;

	BITMAPFILEHEADER bmfh;
	memset( &bmfh , 0 , sizeof( bmfh ) );
	*((char*)&bmfh.bfType) = 'B';
	*(((char*)&bmfh.bfType) + 1 ) = 'M';
	*(((char*)&bmfh.bfType) + 2 ) = 'P';
	bmfh.bfOffBits = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );
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

	file.write( (const char*)&bmfh , sizeof( bmfh ) );
	file.write( (const char*)&header , sizeof( header ) );
	file.write( data , bytes );

	// close it
	file.close();

	delete[] data;

	return true;
}
