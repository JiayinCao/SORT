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

#include "doublesided.h"
#include "sampler/sample.h"
#include "scatteringevent/bsdf/bxdf_utils.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeDoubleSided)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDoubleSided, Tsl_closure, closure0)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeDoubleSided, Tsl_closure, closure1)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeDoubleSided)

Spectrum DoubleSided::f(const Vector& wo, const Vector& wi) const{
    const auto back0 = cosTheta(wo) < 0.0f;
    const auto back1 = cosTheta(wi) < 0.0f;
    if (back0 ^ back1) return 0.0f;

    if (!back0) return m_se0 ? m_se0->Evaluate_BSDF(wo, wi) : 0.0f;
    if (back0) return m_se1 ? m_se1->Evaluate_BSDF(-wo, -wi) : 0.0f;
    return 0.0f;
}

Spectrum DoubleSided::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    const auto back0 = cosTheta(wo) < 0.0f;

    Spectrum ret;
    auto dummy_pdf = 0.0f;
    if (!back0) ret = m_se0 ? m_se0->Sample_BSDF(wo, wi, bs, dummy_pdf, rc) : 0.0f;
    if (back0){
        ret = m_se1 ? m_se1->Sample_BSDF(-wo, wi, bs, dummy_pdf, rc) : 0.0f;
        wi = -wi;
    }
    if (pPdf) *pPdf = pdf(wo, wi);
    return ret;
}

float DoubleSided::pdf(const Vector& wo, const Vector& wi) const {
    const auto back0 = cosTheta(wo) < 0.0f;
    const auto back1 = cosTheta(wi) < 0.0f;
    if (back0 ^ back1) return 0.0f;

    if (!back0) return m_se0 ? m_se0->Pdf_BSDF(wo, wi) : 0.0f;
    if (back0) return m_se1 ? m_se1->Pdf_BSDF(-wo, -wi) : 0.0f;
    return 0.0f;
}
