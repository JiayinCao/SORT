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

#pragma once

/*
    This file exists for the lack of exp support in compile time in C++ 14.

    Commonly Used Taylor Series
    http://people.math.sc.edu/girardi/m142/handouts/10sTaylorPolySeries.pdf

    exp(x) = 1 + x + x^2/2! + x^3/3! + x^4/4! + ...
*/

// This function is purely for the function below, it clearly doesn't handle all corner cases of input.
constexpr double pow_frac( const double x , const int n ){
    return ( n == 0 ) ? 1.0 : pow_frac( x , n - 1 ) * x / (double)n;
}

// This is a taylor approximation of exp function, might not be super accurate.
constexpr double exp_compile_fractional( const double x ){
    return  pow_frac( x , 0 ) + pow_frac( x , 1 ) + pow_frac( x , 2 ) + pow_frac( x , 3 ) + pow_frac( x , 4 ) +
            pow_frac( x , 5 ) + pow_frac( x , 6 ) + pow_frac( x , 7 ) + pow_frac( x , 8 ) + pow_frac( x , 9 );
}

// Log(n) time solution to acquire pow(e,n), where n is an integer.
constexpr double exp_compile_integer( const int n ){
    // e (mathematical constant)
    // https://en.wikipedia.org/wiki/E_(mathematical_constant)
    constexpr double e = 2.71828;
    if( n == 0 )
        return 1.0;

    const auto half_x = n / 2;
    const auto ex = exp_compile_integer( half_x );
    return ex * ex * ( ( n % 2 ) ? e : 1.0 );
}

// Combined solution to calcuclate x of any input.
constexpr double exp_compile( const double x ){
    if( x < 0.0 )
        return 1.0 / exp_compile( -x );
    
    const int       ix = (int)(x);      // floor is not supported in compile time.
    const double    fx = x - ix;        // It should always be between 0 and 1.
    return exp_compile_fractional( fx ) * exp_compile_integer( ix );
}