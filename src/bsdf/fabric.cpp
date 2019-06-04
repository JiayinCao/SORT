/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "fabric.h"
#include "core/samplemethod.h"

Spectrum Fabric::f( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    // A normalization factor is needed
    const auto theta_o = acos(CosTheta(wo));
    const auto theta_i = acos(CosTheta(wi));
    return baseColor * pow( 1.0f - fabs( sin( 0.5f * ( theta_o + theta_i ) ) ) , N ) * AbsCosTheta(wi);
}


Spectrum Fabric::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    return Bxdf::sample_f(wo, wi, bs, pPdf);
}

float Fabric::pdf(const Vector& wo, const Vector& wi) const {
    return Bxdf::pdf(wo, wi);
}