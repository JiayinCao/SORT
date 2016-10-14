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

#ifndef	SORT_NORMALTEXTURE
#define	SORT_NORMALTEXTURE

// include the header
#include "texture.h"

////////////////////////////////////////////////////////////////////////////
// definition of normal texture
class	NormalTexture : public Texture
{
// public method
public:
	DEFINE_CREATOR( NormalTexture , "normal" );

	// default constructor
	NormalTexture() {}
	// destructor
	~NormalTexture() {}

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const;

	// whether the texture support outputing
	// result : 'true' if the texture supports texture output
	virtual	bool CanOutput() const ;
	
	// get the texture value
	// para 'intersect' : the intersection
	// result :	the spectrum value
	virtual Spectrum Evaluate( const Intersection* intersect ) const;
};

#endif
