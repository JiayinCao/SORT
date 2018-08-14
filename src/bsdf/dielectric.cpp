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

#include "dielectric.h"
#include "sampler/sample.h"

Spectrum Dielectric::f(const Vector& wo, const Vector& wi) const{
    return mf_refract.f(wo, wi) + mf_reflect.f(wo, wi);
}

Spectrum Dielectric::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    const float fr = fresnel.Evaluate(CosTheta(wo)).GetIntensity();
    const float r = fr * R.GetIntensity();
    const float t = ( 1.0f - fr ) * T.GetIntensity();
    const float spec_Ratio = r / (r + t);

    Spectrum ret;
    BsdfSample new_bs(true);
    if (bs.u < spec_Ratio || spec_Ratio == 1.0f)
        ret = mf_reflect.sample_f(wo, wi, new_bs, pPdf);
    else
        ret = mf_refract.sample_f(wo, wi, new_bs, pPdf);
    if (pPdf) *pPdf = pdf(wo, wi);
    return ret;
}

float Dielectric::pdf(const Vector& wo, const Vector& wi) const {
    const float fr = fresnel.Evaluate(CosTheta(wo)).GetIntensity();
    const float r = fr * R.GetIntensity();
    const float t = (1.0f - fr) * T.GetIntensity();
    const float spec_Ratio = r / (r + t);

    const float pdf_refract = mf_refract.pdf(wo, wi);
    const float pdf_reflect = mf_reflect.pdf(wo, wi);
    return lerp(pdf_refract, pdf_reflect, spec_Ratio);
}