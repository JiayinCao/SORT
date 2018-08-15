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
#include "blend.h"
#include "bsdf.h"
#include "sampler/sample.h"

Spectrum Blend::f(const Vector& wo, const Vector& wi) const
{
    if (bxdf0 == nullptr && bxdf1 == nullptr) return 0.0f;
    if (bxdf0 == nullptr) return bxdf1->f(wo, wi);
    if (bxdf1 == nullptr) return bxdf0->f(wo, wi);

    return lerp(bxdf0->f(wo, wi), bxdf1->f(wo, wi), weight);
}

Spectrum Blend::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    if (bxdf0 == nullptr && bxdf1 == nullptr)
        return 0.0f;

    if (bs.u < weight || weight == 1.0f || bxdf0 == nullptr )
        bxdf1->sample_f(wo, wi, BsdfSample(true), nullptr);
    else
        bxdf0->sample_f(wo, wi, BsdfSample(true), nullptr);
    if (pPdf) *pPdf = pdf(wo, wi);
    return f(wo, wi);
}

float Blend::pdf(const Vector& wo, const Vector& wi) const {
    if (bxdf0 == nullptr && bxdf1 == nullptr) return 0.0f;
    if (bxdf0 == nullptr) return bxdf1->Pdf(wo, wi);
    if (bxdf1 == nullptr) return bxdf0->Pdf(wo, wi);

    return lerp(bxdf0->Pdf(wo, wi), bxdf1->Pdf(wo, wi), weight);
}
