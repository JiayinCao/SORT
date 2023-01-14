/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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
#include "core/samplemethod.h"
#include "sampler/sample.h"

Bxdf::Bxdf(RenderContext& rc, const Spectrum& w, BXDF_TYPE type, Vector n , bool doubleSided) 
    : ScatteringUnit(w), m_type(type), gnormal( DIR_UP ), doubleSided(doubleSided), rc(rc){
    // Handle the special case where normal map is not applied, in which case, btn and tn will be invalid.
    if( DIR_UP == n ){
        nn = n;
        return;
    }

    normal_map_applied = true;
    nn = normalize(n);
    btn = normalize(cross( nn , Vector( 1.0f , 0.0f , 0.0f ) ));
    tn = normalize(cross( btn , nn ));

    gnormal = bsdfToBxdf( DIR_UP );
}

Bxdf::Bxdf(RenderContext& rc, const Spectrum& ew, const float sw , BXDF_TYPE type, Vector n , bool doubleSided) 
    : ScatteringUnit(ew, sw), m_type(type), gnormal( DIR_UP ), doubleSided(doubleSided), rc(rc){
    // Handle the special case where normal map is not applied, in which case, btn and tn will be invalid.
    if( DIR_UP == n ){
        nn = n;
        return;
    }

    normal_map_applied = true;
    nn = normalize(n);
    btn = normalize(cross( nn , Vector( 1.0f , 0.0f , 0.0f ) ));
    tn = normalize(cross( btn , nn ));

    gnormal = bsdfToBxdf( DIR_UP );
}

Spectrum Bxdf::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
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
    return dot( v , gnormal ) > 0.0f;
}

Spectrum Bxdf::HDR(const Vector& wo) const {
    const unsigned sample_cnt = 4096 * 4;

    Spectrum ret;
    for (auto i = 0u; i < sample_cnt; ++i) {
        Vector wi;
        BsdfSample bs(rc);
        float pdf;
        const auto tmp = sample_f(wo, wi, bs, &pdf);

        sAssert(wi.y >= 0.0f, MATERIAL);

        ret += tmp / pdf;
    }
    return ret / (float)sample_cnt;
}

Spectrum Bxdf::HHR() const {
    const unsigned sample_cnt = 4096;

    Spectrum ret;
    for (auto i = 0u; i < sample_cnt; ++i) {
        const auto r0 = sort_rand<float>(rc);
        const auto r1 = sort_rand<float>(rc);
        const auto wo = UniformSampleHemisphere(r0, r1);
        const auto wo_pdf = UniformHemispherePdf();

        Vector wi;
        BsdfSample bs(rc);
        float wi_pdf;
        const auto tmp = sample_f(wo, wi, bs, &wi_pdf);
        sAssert(wi.y >= 0.0f, MATERIAL);

        if(wi_pdf > 0.0f && !tmp.IsBlack())
            ret += tmp * absCosTheta(wo) / (wo_pdf * wi_pdf);
    }
    return ret * INV_PI / (float)sample_cnt;
}