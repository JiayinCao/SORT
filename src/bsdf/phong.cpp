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
#include "phong.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"
#include "math/matrix.h"

Spectrum Phong::f( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    
    // Diffuse  : f_diffuse( wo , wi ) = D / PI
    // Specular : f_specular( wo , wi ) = ( power + 2.0 ) * S * ( ( reflect( wo ) , wi ) ^ power ) / ( 2 * PI )
    Spectrum ret = D * INV_PI;
    if (!S.IsBlack()) {
        const float alpha = SatDot(wi, reflect(wo));
        if (alpha > 0.0f)
            ret += S * (power + 2) * pow(alpha, power) * INV_TWOPI;
    }
    return ret * AbsCosTheta(wi);
}

Spectrum Phong::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    if (bs.u < diffRatio || diffRatio == 1.0f) {
        sAssertMsg(diffRatio != 0.0f, MATERIAL, "Divided by 0!");
        wi = CosSampleHemisphere(bs.u / diffRatio, bs.v);
    }else{
        // the exact way of importance sampling is taken from my algorithm derived in this blog post, not the ones from the mentioned papers
        // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        const float cos_theta = pow(bs.v, 1.0f / (power + 2.0f));
        const float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
        const float phi = TWO_PI * ( bs.u - diffRatio ) / (1.0f - diffRatio);
        Vector dir = SphericalVec(sin_theta, cos_theta, phi);

        Vector r = reflect(wo);
        Vector t0, t1;
        CoordinateSystem( r, t0, t1 );
        Matrix m(t0.x, r.x, t1.x, 0.0f,
                 t0.y, r.y, t1.y, 0.0f,
                 t0.z, r.z, t1.z, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
        wi = m(dir);
    }

    if (pPdf) *pPdf = pdf(wo, wi);
    return f(wo, wi);
}

float Phong::pdf( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const float cos_theta = SatDot(reflect(wo), wi);
    const float pdf_spec = pow( cos_theta , power + 1.0f ) * ( power + 2.0f ) * INV_TWOPI;
    const float pdf_diff = CosHemispherePdf(wi);

    return lerp( pdf_spec, pdf_diff, diffRatio );
}
