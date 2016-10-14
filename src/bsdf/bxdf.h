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

#ifndef	SORT_BXDF
#define	SORT_BXDF

// include the header
#include "spectrum/spectrum.h"
#include "utility/enum.h"
#include "math/vector3.h"

class BsdfSample;

/////////////////////////////////////////////////////////////////////////
// definition of bxdf
class	Bxdf
{
// public method
public:
	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const = 0;

	// sample a direction randomly
	// para 'wo'  : out going direction
	// para 'wi'  : in direction generated randomly
	// para 'bs'  : bsdf sample variable
	// para 'pdf' : property density function value of the specific 'wi'
	// result     : brdf value for the 'wo' and 'wi'
	virtual Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const;

	// get the pdf of the sampled direction
	// para 'wo' : out going direction
	// para 'wi' : coming in direction from light
	// result    : the pdf for the sample
	virtual float Pdf( const Vector& wo , const Vector& wi ) const;

	// whether the flag is matched
	bool	MatchFlag( BXDF_TYPE type ) const
	{return (type & m_type)==m_type;}

	// get bxdf type
	BXDF_TYPE GetType() const { return m_type; }

	// weight for the bxdf
	Spectrum	m_weight;

// protected field
protected:
	// the type for the bxdf
	BXDF_TYPE m_type = BXDF_NONE;
};

#endif
