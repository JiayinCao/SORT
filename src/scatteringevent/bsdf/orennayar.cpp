/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "orennayar.h"
#include "scatteringevent/bsdf/bxdf_utils.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeOrenNayar)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeOrenNayar, Tsl_float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeOrenNayar, Tsl_float, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeOrenNayar, Tsl_float3, normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeOrenNayar)

OrenNayar::OrenNayar(RenderContext& rc, const ClosureTypeOrenNayar& params, const Spectrum& weight, bool doubleSided ):
    Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), params.normal, doubleSided), R(params.base_color) {
    // roughness ranges from 0 to infinity
    auto sigma = params.roughness;
    sigma = std::max(0.0f, sigma);

    const auto sigma2 = sigma * sigma;
    A = 1.0f - (sigma2 / (2.0f * (sigma2 + 0.33f)));
    B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

OrenNayar::OrenNayar(RenderContext& rc, const Spectrum& reflectance, float sigma, const Spectrum& weight, const Vector& n, bool doubleSided) :
    Bxdf(rc, weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided), R(reflectance){
    // roughness ranges from 0 to infinity
    sigma = std::max(0.0f, sigma);

    const auto sigma2 = sigma * sigma;
    A = 1.0f - (sigma2 / (2.0f * (sigma2 + 0.33f)));
    B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

// // constructor
// OrenNayar::OrenNayar( const Params& params , const Spectrum& weight , bool doubleSided) :
//     Bxdf( weight , (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION) , params.n, doubleSided) , R(params.baseColor){
//     // roughness ranges from 0 to infinity
//     auto sigma = std::max( 0.0f , params.sigma );

//     const auto sigma2 = sigma * sigma;
//     A = 1.0f - (sigma2 / ( 2.0f * (sigma2 +0.33f)));
//     B = 0.45f * sigma2 / (sigma2 + 0.09f );
// }

// evaluate bxdf
// para 'wo' : out going direction
// para 'wi' : in direction
// result    : the portion that comes along 'wo' from 'wi'
Spectrum OrenNayar::f( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    auto sintheta_i = sinTheta(wi);
    auto sintheta_o = sinTheta(wo);

    auto sinphii = sinPhi(wi);
    auto cosphii = cosPhi(wi);
    auto sinphio = sinPhi(wo);
    auto cosphio = cosPhi(wo);
    auto dcos = cosphii * cosphio + sinphii * sinphio;
    if( dcos < 0.0f ) dcos = 0.0f;

    auto abs_cos_theta_o = (float)absCosTheta(wo);
    auto abs_cos_theta_i = (float)absCosTheta(wi);

    if( abs_cos_theta_i < 0.00001f && abs_cos_theta_o < 0.00001f )
        return 0.0f;

    float sinalpha , tanbeta;
    if( abs_cos_theta_o > abs_cos_theta_i )
    {
        sinalpha = sintheta_i;
        tanbeta = sintheta_o / abs_cos_theta_o;
    }else
    {
        sinalpha = sintheta_o;
        tanbeta = sintheta_i / abs_cos_theta_i;
    }

    return R * INV_PI * ( A + B * dcos * sinalpha * tanbeta ) * absCosTheta(wi);
}
