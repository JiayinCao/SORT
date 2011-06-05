/*
 * filename :	bmpsaver.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_BMPSAVER
#define SORT_BMPSAVER

// include the header file
#include "texshower.h"

////////////////////////////////////////////////////////////////////////////
// defination of bmpsaver
// save the image as an bmp file into the file system
class BmpSaver : public TexShower
{
// public method
public:
	// default constructor
	BmpSaver(){}
	// destructor
	~BmpSaver(){}

	// output the texture into bmp file
	// para 'tex' :	the texture for outputing
	// para 'str' : the name of the outputed bmp file
	// result     : 'true' if saving is successful
	virtual bool Output( const Texture* tex , const string& str );
};

#endif
