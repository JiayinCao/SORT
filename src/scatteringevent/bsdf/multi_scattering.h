/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "microfacet.h"

static inline float E(const int sample_cnt, const float roughness, const float n_dot_v, const float* e_data){
    sAssert(e_data != nullptr, MATERIAL);

    const auto fi = roughness * (sample_cnt - 1);
    const auto fj = n_dot_v * (sample_cnt - 1);
    const auto i = (int)(fi);
    const auto j = (int)(fj);
    const auto di = fi - (float)i;
    const auto dj = fj - (float)j;
    
    const auto ni = std::min( i + 1 , sample_cnt - 1 );
    const auto nj = std::min( j + 1 , sample_cnt - 1 );

    const auto e0 = e_data[i * sample_cnt + j];
    const auto e1 = e_data[i * sample_cnt + nj];
    const auto e2 = e_data[ni * sample_cnt + nj];
    const auto e3 = e_data[ni * sample_cnt + j];

    const auto e03 = slerp(e0, e3, di);
    const auto e12 = slerp(e1, e2, di);
    return slerp(e03, e12, dj);
}
static inline float Eavg(const int sample_cnt, const float roughness, const float* eavg_data){
    sAssert(eavg_data != nullptr, MATERIAL);

    const auto fi = roughness * (sample_cnt - 1);
    const auto i = (int)(fi);
    const auto ni = std::min( sample_cnt - 1 , i + 1 );
    const auto di = fi - (float)i;

    const auto eavg0 = eavg_data[i];
    const auto eavg1 = eavg_data[ni];
    return slerp(eavg0, eavg1, di);
}

// Revisiting Physically Based Shading at Imageworks
// http://www.aconty.com/pdf/s2017_pbs_imageworks_slides.pdf
static inline Spectrum MicrofacetMs(const Vector& wo, const Vector& wi, const MicroFacetDistribution* dist, const Fresnel* fresnel){
    const auto n_dot_o = wo.y;
    const auto n_dot_i = wi.y;
    const auto e_avg = dist->Eavg();
    if(e_avg == 1.0f)
        return 0.0f;
    
    // energy conservation part for pure reflective material
    const auto f_ms = INV_PI * (1.0f - dist->E(n_dot_o)) * (1.0f - dist->E(n_dot_i)) / (1.0f - e_avg);

    // A Multi-Faceted Exploration (Part 2)
    // https://blog.selfshadow.com/2018/06/04/multi-faceted-part-2/
    // energy conservation for the fresnel part
    const auto fresnel_avg = fresnel->EvaluateAvg();
    return f_ms * fresnel_avg * fresnel_avg * (1.f - e_avg) / ( 1.0f - fresnel_avg * e_avg);
}

// This is the approximation with fresnel
static inline float E_F(const float f0, const float roughness, const float cos_theta){
    constexpr auto kMsLutSampleCnt = multi_scattering_ggs_fresnel::sample_cnt;
    constexpr auto kMsLutSliceCnt = kMsLutSampleCnt * kMsLutSampleCnt;
    const float* e_data = multi_scattering_ggs_fresnel::g_ms_E;

    const auto f0_fidx = f0 * (kMsLutSampleCnt - 1);
    const auto f0_idx = (int)(f0_fidx);
    const auto f0_idx_next = std::min(f0_idx + 1, kMsLutSampleCnt - 1);

    const auto s0 = E(kMsLutSampleCnt, roughness, cos_theta, e_data + kMsLutSliceCnt * f0_idx);
    const auto s1 = E(kMsLutSampleCnt, roughness, cos_theta, e_data + kMsLutSliceCnt * f0_idx_next);
    return slerp(s0, s1, f0_fidx - (float)f0_idx);
}

static inline float Eavg_F(const float f0, const float roughness){
    constexpr auto kMsLutSampleCnt = multi_scattering_ggs_fresnel::sample_cnt;
    constexpr auto kMsLutSliceCnt = kMsLutSampleCnt;
    const float* eavg_data = multi_scattering_ggs_fresnel::g_ms_Eavg;

    const auto f0_fidx = f0 * (kMsLutSampleCnt - 1);
    const auto f0_idx = (int)(f0_fidx);
    const auto f0_idx_next = std::min(f0_idx + 1, kMsLutSampleCnt - 1);

    const auto s0 = Eavg(kMsLutSampleCnt, roughness, eavg_data + kMsLutSliceCnt * f0_idx);
    const auto s1 = Eavg(kMsLutSampleCnt, roughness, eavg_data + kMsLutSliceCnt * f0_idx_next);
    return slerp(s0, s1, f0_fidx - (float)f0_idx);
}

// Diffuse attenutation to perserve energy conservation
// A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling
// https://www.researchgate.net/publication/2378872_A_Microfacet_Based_Coupled_Specular-Matte_BRDF_Model_with_Importance_Sampling
static inline Spectrum DiffuseAttenuation(const Spectrum f0, const float roughness, const float cos_o, const float cos_i){
    // Note, there is a Pi in the denominator of the equation in the original paper.
    // The pi is implicitly hidden in lambert model brdf so it is skipped here.
    // This does introduce some implicit connection since in a strict sense, this only works with lambert brdf as diffuse
    // However, the visual impact is really not that big, so I'm fine with it.
    const auto eavg_f_0 = Eavg_F(f0[0], roughness);
    const auto eavg_f_1 = Eavg_F(f0[1], roughness);
    const auto eavg_f_2 = Eavg_F(f0[2], roughness);
    const auto r = eavg_f_0 == 1.f ? 0.f : (1.0f - E_F(f0[0], roughness, cos_o)) * (1.0f - E_F(f0[0], roughness, cos_i)) / (1.f - eavg_f_0);
    const auto g = eavg_f_1 == 1.f ? 0.f : (1.0f - E_F(f0[1], roughness, cos_o)) * (1.0f - E_F(f0[1], roughness, cos_i)) / (1.f - eavg_f_1);
    const auto b = eavg_f_2 == 1.f ? 0.f : (1.0f - E_F(f0[2], roughness, cos_o)) * (1.0f - E_F(f0[2], roughness, cos_i)) / (1.f - eavg_f_2);
    return Spectrum(r, g, b);
}