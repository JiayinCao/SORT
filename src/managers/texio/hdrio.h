/*
   FileName:      hdrio.h

   Created Time:  2015-12-8

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_HDRIO
#define	SORT_HDRIO

// include the header file
#include "texio.h"

////////////////////////////////////////////////////////////////////////////
// definition of bmpio
// save the image as an bmp file into the file system
// load the bmp file from file system into texture
class HdrIO : public TexIO
{
// public method
public:
	// default constructor
	HdrIO(){m_TexType=TT_HDR;}
	
	// output the texture into bmp file
	// para 'str' : the name of the outputed bmp file
	// para 'tex' :	the texture for outputing
	// result     : 'true' if saving is successful
	virtual bool Write( const string& str , const Texture* tex );

	// read data from file
	// para 'str' : the name of the input entity
	// para 'mem' : the memory for the image
	// result     :	'true' if the input file is parsed successfully
	virtual bool Read( const string& str , ImgMemory* mem );
};

#endif