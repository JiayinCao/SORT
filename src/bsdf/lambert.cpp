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

#include "lambert.h"
#include "math/vector3.h"
#include "bsdf.h"
#include "utility/samplemethod.h"
#include "sampler/sample.h"

// evaluate bxdf
Spectrum Lambert::f( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    return R * INV_PI * AbsCosTheta(wi);
}

// evaluate bxdf
Spectrum LambertTransmission::f( const Vector& wo , const Vector& wi ) const{
    return SameHemiSphere( wo , wi ) ? 0.0f : T * INV_PI * AbsCosTheta(wi);
}

// sample a direction randomly
Spectrum LambertTransmission::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
    wi = -CosSampleHemisphere( bs.u , bs.v );
    if( pPdf ) *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

// get the pdf of the sampled direction
float LambertTransmission::pdf( const Vector& wo , const Vector& wi ) const{
    return SameHemisphere( wo , wi ) ? 0.0f : CosHemispherePdf( wi );
}
