/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef SORT_BMPIO
#define SORT_BMPIO

// include the header file
#include "texio.h"

////////////////////////////////////////////////////////////////////////////
// definition of bmpio
// save the image as an bmp file into the file system
// load the bmp file from file system into texture
class BmpIO : public TexIO
{
// public method
public:
	// default constructor
	BmpIO(){m_TexType=TT_BMP;}
	
	// output the texture into bmp file
	// para 'str' : the name of the outputed bmp file
	// para 'tex' :	the texture for outputing
	// result     : 'true' if saving is successful
	// note		  : all of the image we out put is 24-bits
	virtual bool Write( const string& str , const Texture* tex );

	// read data from file
	// para 'str' : the name of the input entity
	// para 'mem' : the memory for the image
	// result     :	'true' if the input file is parsed successfully
	// note       : only 24-bits or 32-bits image are supported!!
	virtual bool Read( const string& str , ImgMemory* mem );
};

#endif
