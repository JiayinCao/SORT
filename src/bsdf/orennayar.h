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

#ifndef	SORT_ORENNAYAR
#define	SORT_ORENNAYAR

// include header file
#include "bxdf.h"

////////////////////////////////////////////////////////////////////////
//	A geometric-optics microfacet-based brdf model that is proposed by
//	Oren and Nayar (1994)
class OrenNayar : public Bxdf
{
// public method
public:
	// constructor
	OrenNayar( const Spectrum& reflectence , float roughness);
	
	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const;
	
// private field
private:
	// spectrum for scaling the brdf
	Spectrum R;
	// the factors
	float	A;
	float	B;
};

#endif
