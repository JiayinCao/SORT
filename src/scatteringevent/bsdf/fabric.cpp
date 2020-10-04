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

#include <fabric_lut.h>
#include "fabric.h"
#include "core/samplemethod.h"
#include "core/rand.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeFabric)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeFabric, Tsl_float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeFabric, Tsl_float, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeFabric, Tsl_float3, normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeFabric)

#define Io  g_fabric_lut

Spectrum Fabric::f( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    const auto i = (int)(( N / 30.0f ) * 255.0f);
    const auto io = Io[i];

    const auto h = normalize( wo + wi );
    return baseColor * pow( 1.0f - fabs(h.x) , N ) * absCosTheta(wi) / io;
}

Spectrum Fabric::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));
    const auto sign = sort_canonical() > 0.5f ? 1.0f : -1.0f;
    const auto sin_theta_h = sign * saturate( 1.0f - pow( sort_canonical() , 1.0f / ( N + 1 ) ) );
    const auto cos_theta_h = sqrt( saturate( 1.0f - SQR( sin_theta_h ) ) );
    const auto phi_h = PI * sort_canonical();

    const auto wh = Vector3f( sin_theta_h , cos_theta_h * sin( phi_h ) , cos_theta_h * cos( phi_h ) );
    wi = reflect( wo , wh );

    if( pPdf )
        *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

float Fabric::pdf(const Vector& wo, const Vector& wi) const {
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    const auto wh = normalize( wo + wi );
    const auto pdf_h = ( N + 1 ) * pow( 1.0f - fabs(wh.x) , N ) * INV_TWOPI;
    return pdf_h / ( 4.0f * dot( wo , wh ) );
}