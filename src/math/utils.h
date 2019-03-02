/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include <math.h>
#include "core/define.h"

#define	SQR(x)		(Pow<2>(x))

//! @brief	Power of a value.
//!
//! Instead of using the native one provided by C++, this involves O(ln(N)) multiplications
//! where 'N' is the number of power.
//!
//! @param	n	The number of power.
//! @param	x	Value to be evaluated.
//! @return		x^n
template<unsigned n>
inline float Pow( float x ){
	static_assert(n > 0, "Power can't be negative");
    const auto x2 = Pow<n / 2>(x);
    return x2 * x2 * Pow<n & 1>(x);
}

template<>
inline float Pow<0>( float x ){
	return 1.0f;
}

template<>
inline float Pow<1>( float x ){
	return x;
}

//! @brief	Clamp before calculating square root to avoid NaN.
//!
//! @param	x	Value to be evaluated.
//! @return		The square root of @param x.
inline float ssqrt( const float x ){
	return sqrt( std::max( 0.0f , x ) );
}