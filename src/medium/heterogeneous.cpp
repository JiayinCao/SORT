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

#include "heterogeneous.h"
#include "core/rand.h"
#include "core/memory.h"
#include "material/material.h"
#include "phasefunction.h"

Spectrum HeterogenousMedium::Tr( const Ray& ray , const float max_t ) const{
    // get the step size and count
    auto        step_size = m_material->GetVolumeStep();
    const auto  step_cnt = m_material->GetVolumeStepCnt();

    auto t = 0.0f;
    Spectrum exponent(0.0f);

    // ray marching
    for (auto i = 0u; i < step_cnt; ++i) {
        const auto dt = t + step_size <= max_t ? step_size : max_t - t;
        const auto new_t = t + dt * sort_canonical();

        // take a sample in the medium
        MediumSample ms;
        MediumInteraction tmp_mi;
        tmp_mi.intersect = ray(new_t);
        m_material->EvaluateMediumSample(tmp_mi, ms);

        exponent -= ms.basecolor * ms.extinction * dt;

        t += dt;

        if (t >= max_t)
            break;
    }

    return exponent.Exp();
}

Spectrum HeterogenousMedium::Sample( const Ray& ray , const float max_t , MediumInteraction*& mi, Spectrum& emission) const{
    // Distance Sample, Jan Novak
    // https://cs.dartmouth.edu/~wjarosz/publications/novak18monte-slides-3-distance-sampling.pdf

    // get the step size and count
    auto        step_size = m_material->GetVolumeStep();
    const auto  step_cnt  = m_material->GetVolumeStepCnt();
    
    // always start from 0.0 regardless the min_t value setup in ray
    auto t = 0.0f;

    // a random value
    auto r = sort_canonical();

    auto accum_transmittance = Spectrum(1.0f);

    // ray marching
    for (auto i = 0u; i < step_cnt; ++i) {
        const auto dt = t + step_size <= max_t ? step_size : max_t - t;
        const auto new_t = t + dt * sort_canonical();

        // take a sample in the medium
        MediumSample ms;
        MediumInteraction tmp_mi;
        tmp_mi.intersect = ray(new_t);
        m_material->EvaluateMediumSample(tmp_mi, ms);

        // beam transmittancy along the ray through the short distance
        const auto extinction = ms.basecolor * ms.extinction;
        const auto exponent = -dt * extinction;
        const auto beam_transmitancy = exponent.Exp();

        // maybe a better sampling algorithm for channel picking later
        const auto ch = clamp((int)(sort_canonical() * RGBSPECTRUM_SAMPLE), 0, RGBSPECTRUM_SAMPLE - 1);

        if (1.0f - r >= beam_transmitancy[ch]) {
            // sample a medium and scatter the ray
            const auto new_dt = -std::log(1.0f - r) / extinction[ch];

            const auto new_exponent = -new_dt * extinction;
            const auto new_beam_transmitancy = new_exponent.Exp();
            const auto new_pdf = new_beam_transmitancy * extinction;

            const auto pdf = (new_pdf[0] + new_pdf[1] + new_pdf[2]) / 3.0f;

            mi = SORT_MALLOC(MediumInteraction)();
            mi->intersect = ray( t + new_dt );
            mi->phaseFunction = SORT_MALLOC(HenyeyGreenstein)(ms.anisotropy);

            accum_transmittance *= new_beam_transmitancy / pdf;

            // add emission
            emission = ms.emission * ms.basecolor * ms.absorption * accum_transmittance;

            return accum_transmittance * ms.scattering * ms.basecolor;
        } else {
            const auto new_pdf = beam_transmitancy;
            const auto pdf = (new_pdf[0] + new_pdf[1] + new_pdf[2]) / 3.0f;

            accum_transmittance *= beam_transmitancy / pdf;

            r = 1.0f - (1.0f - r) / pdf;
        }

        t += dt;

        if (t >= max_t)
            break;
    }

    // sampling the surface behind the volume instead of the volume itself.
    return accum_transmittance;
}