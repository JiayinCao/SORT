/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "scatteringevent.h"
#include "bsdf/bxdf.h"
#include "bssrdf/bssrdf.h"
#include "sampler/sample.h"

ScatteringEvent::ScatteringEvent( const Intersection& intersection , const SE_Flag flag )
: m_flag(flag), m_intersection( intersection ){
    m_n = Normalize(intersection.normal);
    m_bt = Normalize(Cross( m_n , intersection.tangent ));
    m_t = Normalize(Cross( m_bt , m_n ));
}

Vector ScatteringEvent::worldToLocal( const Vector& v , bool forceTransform ) const{
    if( ( m_flag & SE_SUB_EVENT ) && !forceTransform ) return v;
    return Vector( Dot(v,m_t) , Dot(v,m_n) , Dot(v,m_bt) );
}

Vector ScatteringEvent::localToWorld( const Vector& v ) const{
    if( m_flag & SE_SUB_EVENT ) return v;
    return Vector(  v.x * m_t.x + v.y * m_n.x + v.z * m_bt.x ,
                    v.x * m_t.y + v.y * m_n.y + v.z * m_bt.y ,
                    v.x * m_t.z + v.y * m_n.z + v.z * m_bt.z );
}

float ScatteringEvent::SampleScatteringType( SE_Flag& flag ) const{
    float bxdf_weight = 0.0f , bssrdf_weight = 0.0f;
    for( auto i = 0u ; i < m_bxdfCnt ; ++i )
        bxdf_weight += m_bxdfs[i]->GetWeight().GetIntensity();
    for( auto i = 0u ; i < m_bssrdfCnt ; ++i )
        bssrdf_weight += m_bssrdfs[i]->GetWeight().GetIntensity();

    const auto pdf_bxdf = bxdf_weight / ( bxdf_weight + bssrdf_weight );
    const auto r = sort_canonical();
    flag = ( r < pdf_bxdf ) ? SE_EVALUATE_BXDF : SE_EVALUATE_BSSRDF;
    return flag == SE_EVALUATE_BXDF ? pdf_bxdf : 1.0f - pdf_bxdf;
}

Spectrum ScatteringEvent::Evaluate_BSDF( const Vector& wo , const Vector& wi ) const{
    const auto swo = worldToLocal( wo );
    const auto swi = worldToLocal( wi );
    Spectrum r;
    for( auto i = 0u ; i < m_bxdfCnt ; ++i )
        r += m_bxdfs[i]->F( swo , swi ) * m_bxdfs[i]->GetWeight();
    return r;
}

Spectrum ScatteringEvent::Sample_BSDF( const Vector& wo , Vector& wi , const class BsdfSample& bs , float& pdf ) const{
    Spectrum ret;
    if( m_bxdfCnt == 0 )
        return ret;

    const auto bxdf_id = std::min( (int)(bs.t*(float)m_bxdfCnt) , (int)(m_bxdfCnt-1) );
    const Bxdf* bxdf = m_bxdfs[bxdf_id];

    // transform the 'wo' from world space to shading coordinate
    auto swo = worldToLocal( wo );

    // sample the direction
    ret = bxdf->Sample_F( swo , wi , bs , &pdf ) * bxdf->GetWeight();

    // if there is no properbility of sampling that direction , just return 0.0f
    if( pdf == 0.0f )
        return ret;
    
    // setup pdf
    for( auto i = 0u; i < m_bxdfCnt ; ++i )
        if( i != bxdf_id ){
            pdf += m_bxdfs[i]->Pdf( wo , wi );
            ret += m_bxdfs[i]->F(wo,wi) * m_bxdfs[i]->GetWeight();
        }
    pdf /= m_bxdfCnt;

    // transform the direction back
    wi = localToWorld( wi );

    return ret;
}

float ScatteringEvent::Pdf_BSDF( const Vector& wo , const Vector& wi ) const{
    const auto lwo = worldToLocal( wo );
    const auto lwi = worldToLocal( wi );

    auto pdf = 0.0f;
    for( auto i = 0u ; i < m_bxdfCnt ; ++i )
        pdf += m_bxdfs[i]->Pdf( lwo , lwi );
    return pdf / m_bxdfCnt;
}

void ScatteringEvent::Sample_BSSRDF( const Scene& scene , const Vector& wo , const Point& po , BSSRDFIntersections& inter , float& pdf ) const{
    // uniform sampling is used for now, this needs to be revised later for better convergence rate.
    const auto bssrdf_id = std::min( (int)( sort_canonical() * (float)m_bssrdfCnt) , (int)(m_bssrdfCnt-1) );
    const auto bssrdf = m_bssrdfs[bssrdf_id];
    bssrdf->Sample_S( scene , wo , po , inter );
    pdf = 1.0f / (float) m_bssrdfCnt;
}