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
#include "fresnelblend.h"
#include "bsdf.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

FresnelBlend::FresnelBlend( const Spectrum& diffuse , const Spectrum& specular , const MicroFacetDistribution* d ) : D(diffuse) , S(specular) , distribution(d){
	m_type = (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION);
}

Spectrum FresnelBlend::f( const Vector& wo , const Vector& wi ) const
{
    if( !SameHemiSphere(wo, wi) ) return 0.0f;
    
    const float cos_theta_o = AbsCosTheta(wo);
    const float cos_theta_i = AbsCosTheta(wi);
    
#define pow5(x) ( (x*x)*(x*x)*x )
    // Diffuse  : f_diffuse( wo , wi ) = 28.0f / ( 23.0f * PI ) * ( 1.0 - R ) * ( 1.0 - ( 1.0 - 0.5 * CosTheta(wo) ) ^ 5 ) * ( 1.0 - ( 1.0 - 0.5f * CosTheta(wi) ) ^ 5
    // Specular : f_specular( wo , wi ) = D(h) * SchlickFresnel(S,Dot(wi,h)) / ( 4.0f * AbsDot( wi , h ) * max( AbsDot(wi,n) , AbsDot(wo,n) )
    const Spectrum diffuse = 0.3875f * D * ( Spectrum( 1.0f ) - S )
                             * ( 1.0f - pow5( 1.0f - 0.5f * ( 1.0f - cos_theta_o ) ) )
                             * ( 1.0f - pow5( 1.0f - 0.5f * ( 1.0f - cos_theta_i ) ) );
    
    Vector h = wo + wi;
    if( h.IsZero() ) return 0.0f;
    h = Normalize(h);
    
    const float IoH = AbsDot( wi , h );
    const Spectrum specular = ( distribution->D(h) * SchlickFresnel(S, IoH) ) / ( 4.0f * IoH * max( cos_theta_i , cos_theta_o ) ) ;
#undef pow5
    
    return diffuse + specular;
}

Spectrum FresnelBlend::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const{
    if( bs.u < 0.5f ){
        // Cosine-weighted sample
        wi = CosSampleHemisphere( 2.0f * bs.u , bs.v );
        if( PointingUp( wo ) ) wi.z = -wi.z;
    }else{
        BsdfSample sample(true);
        Vector wh = distribution->sample_f(sample,wo);
        wi = 2 * Dot( wo , wh ) * wh - wo;
        if( !SameHemiSphere(wo, wi) ) return 0.0f;
    }
    
    if( pdf ) *pdf = Pdf( wo , wi );
    return f( wo , wi );
}

float FresnelBlend::Pdf( const Vector& wo , const Vector& wi ) const{
    if( !SameHemiSphere(wo, wi) ) return 0.0f;
    
    const Vector wh = Normalize( wi + wo );
    float pdf_wh = distribution->D( wh ) * AbsCosTheta( wh );
    return 0.5f * ( AbsCosTheta(wi) * INV_PI + pdf_wh / ( 4.0f * Dot( wo , wh ) ) );
}
