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

#include "heterogeneous.h"
#include "core/rand.h"
#include "core/memory.h"
#include "core/render_context.h"
#include "material/material.h"
#include "phasefunction.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeHeterogenous)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHeterogenous, Tsl_float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHeterogenous, Tsl_float, emission)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHeterogenous, Tsl_float, absorption)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHeterogenous, Tsl_float, scattering)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHeterogenous, Tsl_float, anisotropy)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeHeterogenous)

Spectrum HeterogenousMedium::Tr(const Ray& ray, const float max_t, RenderContext& rc) const {
    // get the step size and count
    auto        step_size = m_material->GetVolumeStep();
    const auto  step_cnt = m_material->GetVolumeStepCnt();

    auto t = 0.0f;
    Spectrum exponent(0.0f);

    // ray marching
    for (auto i = 0u; i < step_cnt; ++i) {
        const auto dt = t + step_size <= max_t ? step_size : max_t - t;
        const auto new_t = t + dt * sort_rand<float>(rc);

        // take a sample in the medium
        MediumSample ms;
        MediumInteraction tmp_mi;
        tmp_mi.intersect = ray(new_t);
        tmp_mi.mesh = m_mesh;
        m_material->EvaluateMediumSample(tmp_mi, ms);

        exponent -= ms.basecolor * ms.extinction * dt;

        t += dt;

        if (t >= max_t)
            break;
    }

    return exponent.Exp();
}

Spectrum HeterogenousMedium::Sample(const Ray& ray, const float max_t, MediumInteraction*& mi, Spectrum& emission, RenderContext& rc) const {
    // Distance Sample, Jan Novak
    // https://cs.dartmouth.edu/~wjarosz/publications/novak18monte-slides-3-distance-sampling.pdf

    // get the step size and count
    auto        step_size = m_material->GetVolumeStep();
    const auto  step_cnt = m_material->GetVolumeStepCnt();

    // always start from 0.0 regardless the min_t value setup in ray
    auto t = 0.0f;

    // a random value
    auto r = sort_rand<float>(rc);

    // accumulative transmittance
    auto accum_transmittance = Spectrum(1.0f);

    // maybe a better sampling algorithm for channel picking later
    const auto ch = clamp((int)(sort_rand<float>(rc) * RGBSPECTRUM_SAMPLE), 0, RGBSPECTRUM_SAMPLE - 1);

    // ray marching
    for (auto i = 0u; i < step_cnt; ++i) {
        const auto dt = t + step_size <= max_t ? step_size : max_t - t;
        const auto new_t = t + dt * sort_rand<float>(rc);

        // take a sample in the medium
        MediumSample ms;
        MediumInteraction tmp_mi;
        tmp_mi.intersect = ray(new_t);
        tmp_mi.mesh = m_mesh;
        m_material->EvaluateMediumSample(tmp_mi, ms);

        // beam transmittance along the ray through the short distance
        const auto extinction = ms.basecolor * ms.extinction;
        const auto exponent = -dt * extinction;
        const auto beam_transmitancy = exponent.Exp();

        if (1.0f - r >= beam_transmitancy[ch]) {
            // sample a medium and scatter the ray
            const auto new_dt = -log(1.0f - r) / extinction[ch];

            mi = SORT_MALLOC(rc.m_memory_arena, MediumInteraction)();
            mi->intersect = ray(t + new_dt);
            mi->phaseFunction = SORT_MALLOC(rc.m_memory_arena, HenyeyGreenstein)(ms.anisotropy);
            
            const auto new_exponent = -new_dt * extinction;
            const auto new_beam_transmitancy = new_exponent.Exp();
            accum_transmittance *= new_beam_transmitancy;
            const auto new_pdf = accum_transmittance * extinction;
            const auto pdf = (new_pdf[0] + new_pdf[1] + new_pdf[2]) / 3.0f;
            accum_transmittance /= pdf;

            // This model is what is used in PBRT and different from 'Production Volume Rendering' by Disney.
            emission = ms.emission * ms.basecolor * ms.absorption * accum_transmittance;

            return accum_transmittance * ms.scattering * ms.basecolor;
        } else {
            accum_transmittance *= beam_transmitancy;
            r = 1.0f - (1.0f - r) / beam_transmitancy[ch];
        }

        t += dt;

        if (t >= max_t)
            break;
    }

    // sampling the surface behind the volume instead of the volume itself.
    const auto pdf = (accum_transmittance[0] + accum_transmittance[1] + accum_transmittance[2]) / 3.0f;
    return accum_transmittance / pdf;
}