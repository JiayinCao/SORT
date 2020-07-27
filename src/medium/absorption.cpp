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

#include "absorption.h"
#include "core/rand.h"
#include "core/memory.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeAbsorption)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeAbsorption, Tsl_float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeAbsorption, Tsl_float, absorption)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeAbsorption)

Spectrum AbsorptionMedium::Tr( const Ray& ray , const float max_t ) const{
    const auto e = m_globalMediumSample.basecolor * (m_globalMediumSample.absorption * -fmin(max_t, FLT_MAX ));
    return e.Exp();
}

// Since there is no scattering, medium interaction is never sampled in this type of medium.
Spectrum AbsorptionMedium::Sample( const Ray& ray, const float max_t, MediumInteraction*& mi , Spectrum& emission ) const{
    return Tr( ray , max_t );
}