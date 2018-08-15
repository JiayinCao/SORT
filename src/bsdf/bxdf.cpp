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

Bxdf::Bxdf(const Spectrum& w, BXDF_TYPE type, Vector n , bool doubleSided) : m_weight(w), m_type(type), gnormal( DIR_UP ), doubleSided(doubleSided){
    if( n == DIR_UP ) return;
    
    normal_map_applied = true;
    nn = Normalize(n);
    tn = Normalize(Cross( nn , Vector( 1.0f , 0.0f , 0.0f ) ));
    sn = Normalize(Cross( tn , nn ));
}

// sample a direction randomly
Spectrum Bxdf::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const
{
	wi = CosSampleHemisphere( bs.u , bs.v );
	if( pPdf ) *pPdf = pdf( wo , wi );
	return f( wo , wi );
}

// the pdf for the sampled direction
float Bxdf::pdf( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    return CosHemispherePdf( wi );
}

bool Bxdf::PointingUp( const Vector& v ) const {
    return Dot( v , gnormal ) > 0.0f;
}
