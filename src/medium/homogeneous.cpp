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

#include "homogeneous.h"
#include "core/rand.h"
#include "core/memory.h"
#include "phasefunction.h"
#include "core/render_context.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeHomogeneous)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHomogeneous, Tsl_float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHomogeneous, Tsl_float, emission)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHomogeneous, Tsl_float, absorption)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHomogeneous, Tsl_float, scattering)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeHomogeneous, Tsl_float, anisotropy)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeHomogeneous)

Spectrum HomogeneousMedium::Tr( const Ray& ray , const float max_t, RenderContext& rc) const{
    const auto e = m_globalMediumSample.basecolor * m_globalMediumSample.extinction * (-fmin(max_t, FLT_MAX ));
    return e.Exp();
}

Spectrum HomogeneousMedium::Sample( const Ray& ray , const float max_t , MediumInteraction*& mi , Spectrum& emission , RenderContext& rc) const{
    const auto extinction = m_globalMediumSample.basecolor * m_globalMediumSample.extinction;
    const auto scattering = m_globalMediumSample.basecolor * m_globalMediumSample.scattering;
    const auto absorption = m_globalMediumSample.basecolor * m_globalMediumSample.absorption;

    const auto ch = clamp( (int)(sort_rand<float>(rc) * RGBSPECTRUM_SAMPLE) , 0 , RGBSPECTRUM_SAMPLE - 1 );
    const auto d = fmin( -log( sort_rand<float>(rc) ) / extinction[ch] , max_t );

    const auto sample_medium = d < max_t;
    if (sample_medium) {
        mi = SORT_MALLOC(rc.m_memory_arena, MediumInteraction)();
        mi->intersect = ray(d);
        mi->phaseFunction = SORT_MALLOC(rc.m_memory_arena, HenyeyGreenstein)(m_globalMediumSample.anisotropy);
    }

    const auto e = extinction * (-fmin( d , FLT_MAX ));
    const auto tr = e.Exp();

    const auto density = sample_medium ? (extinction * tr) : tr;

    auto pdf = 0.0f;
    for( auto i = 0u ; i < RGBSPECTRUM_SAMPLE ; ++i )
        pdf += density[i];
    pdf /= RGBSPECTRUM_SAMPLE;

    // This should rarely happen, though.
    if ( UNLIKELY(pdf == 0.0f) )
        return 0.0f;

    // This model is what is used in PBRT and different from 'Production Volume Rendering' by Disney.
    if (sample_medium)
        emission = m_globalMediumSample.basecolor * m_globalMediumSample.emission * m_globalMediumSample.absorption * tr / pdf;

    return sample_medium ? ( tr * scattering / pdf ) : ( tr / pdf );
}