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

Spectrum HomogeneousMedium::Tr( const Ray& ray ) const{
    const auto e = t * (-fmin( ray.m_fMax , FLT_MAX ));
    return e.Exp();
}

// this is by no-means a finished implementation, it is left this way because there are other code infrustruction not
// fully ready yet.
//  - mediuminteraction is not correctly setup for now
//  - there are several corner cases not well handled yet.
Spectrum HomogeneousMedium::Sample( const Ray& ray , MediumInteraction*& mi ) const{
    const auto ch = clamp( (int)(sort_canonical() * RGBSPECTRUM_SAMPLE) , 0 , RGBSPECTRUM_SAMPLE - 1 );
    const auto d = fmin( -std::log( sort_canonical() ) / t[ch] , ray.m_fMax );

    const auto sample_medium = d < ray.m_fMax;
    if( sample_medium )
        mi = SORT_MALLOC(MediumInteraction)();

    const auto e = t * (-fmin( d , FLT_MAX ));
    const auto tr = e.Exp();

    const auto density = sample_medium ? ( t * tr ) : tr;

    auto pdf = 0.0f;
    for( auto i = 0u ; i < RGBSPECTRUM_SAMPLE ; ++i )
        pdf += density[i];
    pdf /= RGBSPECTRUM_SAMPLE;

    return sample_medium ? ( tr * s / pdf ) : ( tr / pdf );
}