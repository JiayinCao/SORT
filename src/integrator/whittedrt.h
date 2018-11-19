/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

// include the header file
#include "integrator.h"

/////////////////////////////////////////////////////////////////////////////
// definition of whittedrt
// note : Whitted ray tracer only takes direct light into consideration,
//		  there are also specular reflection and refraction. While indirect 
//		  light, like color bleeding , is not supported.
class	WhittedRT : public Integrator
{
public:
	DEFINE_CREATOR( WhittedRT , Integrator , "whitted" );

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;

private:
    SORT_STATS_ENABLE( "Whitted Ray Tracing" )
};
