/*
 * filename :	bmpio.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_BMPIO
#define SORT_BMPIO

// include the header file
#include "texio.h"

////////////////////////////////////////////////////////////////////////////
// defination of bmpio
// save the image as an bmp file into the file system
// load the bmp file from file system into texture
class BmpIO : public TexIO
{
// public method
public:
	// default constructor
	BmpIO(){tot_type=TOT_BMP;}
	
	// output the texture into bmp file
	// para 'str' : the name of the outputed bmp file
	// para 'tex' :	the texture for outputing
	// result     : 'true' if saving is successful
	virtual bool Write( const string& str , const Texture* tex );
};

#endif
