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

// include the header file
#include "ashikhmanshirley.h"
#include "bsdf.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

Spectrum AshikhmanShirley::f( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    
    const float cos_theta_o = AbsCosTheta(wo);
    const float cos_theta_i = AbsCosTheta(wi);
    
    // Diffuse  : f_diffuse( wo , wi ) = 28.0f / ( 23.0f * PI ) * ( 1.0 - R ) * ( 1.0 - ( 1.0 - 0.5 * CosTheta(wo) ) ^ 5 ) * ( 1.0 - ( 1.0 - 0.5f * CosTheta(wi) ) ^ 5
    // Specular : f_specular( wo , wi ) = D(h) * SchlickFresnel(S,Dot(wi,h)) / ( 4.0f * AbsDot( wi , h ) * max( AbsDot(wi,n) , AbsDot(wo,n) )
    const Spectrum diffuse = 0.3875f * D * ( Spectrum( 1.0f ) - S ) * ( 1.0f - SchlickWeight( 0.5f * cos_theta_o ) ) * ( 1.0f - SchlickWeight( 0.5f * cos_theta_i ) );
    
    Vector h = wo + wi;
    if( h.IsZero() ) return 0.0f;
    h = Normalize(h);
    
    const float IoH = AbsDot( wi , h );
    const Spectrum specular = ( distribution.D(h) * SchlickFresnel(S, IoH) ) / ( 4.0f * IoH * std::max( cos_theta_i , cos_theta_o ) ) ;
    
    return ( diffuse + specular ) * AbsCosTheta(wi);
}

Spectrum AshikhmanShirley::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
    if( bs.u < 0.5f ){
        // Cosine-weighted sample
        wi = CosSampleHemisphere( 2.0f * bs.u , bs.v );
        if( !SameHemiSphere(wo, wi) ) wi *= -1.0f;
    }else{
        BsdfSample sample(true);
        Vector wh = distribution.sample_f(sample);
        wi = 2 * Dot( wo , wh ) * wh - wo;
    }
    if( pPdf ) *pPdf = pdf( wo , wi );
    
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    
    return f( wo , wi );
}

float AshikhmanShirley::pdf( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    
    const Vector wh = Normalize( wi + wo );
    float pdf_wh = distribution.Pdf(wh);
    return lerp( CosHemispherePdf(wi) , pdf_wh / ( 4.0f * Dot( wo , wh ) ) , 0.5f );
}
