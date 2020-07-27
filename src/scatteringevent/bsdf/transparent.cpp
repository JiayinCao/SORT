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

#include "transparent.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeTransparent)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeTransparent, Tsl_float3, attenuation)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeTransparent)

Spectrum Transparent::f( const Vector& wo , const Vector& wi ) const{
    return Spectrum( 0.0f , 0.0f , 0.0f );
}

float Transparent::pdf( const Vector& wo , const Vector& wi ) const{
    return 0.0f;
}

Spectrum Transparent::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const{
    wi = -wo;
    if( pdf )
        *pdf = 1.0f;
    return A;
}