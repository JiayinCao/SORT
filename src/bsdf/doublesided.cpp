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
#include "doublesided.h"
#include "bsdf.h"
#include "sampler/sample.h"

Spectrum DoubleSided::f(const Vector& wo, const Vector& wi) const
{
    const bool back0 = CosTheta(wo) < 0.0f;
    const bool back1 = CosTheta(wi) < 0.0f;
    if (back0 ^ back1) return 0.0f;

    if (!back0) return bxdf0 ? bxdf0->f(wo, wi) : 0.0f;
    if (back0) return bxdf1 ? bxdf1->f(-wo, -wi) : 0.0f;
    return 0.0f;
}

Spectrum DoubleSided::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    const bool back0 = CosTheta(wo) < 0.0f;
    const bool back1 = CosTheta(wi) < 0.0f;
    if (back0 ^ back1) return 0.0f;

    Spectrum ret;
    if (!back0) ret = bxdf0 ? bxdf0->sample_f(wo, wi, bs, nullptr) : 0.0f;
    if (back0){
        ret = bxdf1 ? bxdf1->sample_f(-wo, wi, bs, nullptr) : 0.0f;
        wi = -wi;
    }
    if (pPdf) *pPdf = pdf(wo, wi);
    return ret;
}

float DoubleSided::pdf(const Vector& wo, const Vector& wi) const {
    const bool back0 = CosTheta(wo) < 0.0f;
    const bool back1 = CosTheta(wi) < 0.0f;
    if (back0 ^ back1) return 0.0f;

    if (!back0) return bxdf0 ? bxdf0->Pdf(wo, wi) : 0.0f;
    if (back0) return bxdf1 ? bxdf1->Pdf(-wo, -wi) : 0.0f;
    return 0.0f;
}
