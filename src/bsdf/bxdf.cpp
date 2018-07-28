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

#include "bxdf.h"
#include "utility/samplemethod.h"
#include "sampler/sample.h"

// sample a direction randomly
Spectrum Bxdf::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
	wi = CosSampleHemisphere( bs.u , bs.v );
	if( pdf ) *pdf = Pdf( wo , wi );
	return f( wo , wi );
}

// the pdf for the sampled direction
float Bxdf::Pdf( const Vector& wo , const Vector& wi ) const{
    return SameHemisphere( wo , wi ) ? CosHemispherePdf( wi ) : 0.0f;
}
