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

// #include <OSL/oslexec.h>
#include "core/define.h"
#include "core/sassert.h"
#include "math/utils.h"

#define RGBSPECTRUM_SAMPLE      3

// @brief   Basic color representation based on RGB color.
/**
 * The color space is linear and sRGB. SORT doesn't support more advanced HDR color spaces, like
 * Rec 2020. All colors are in sRGB space and linear in SORT.
 */
class   RGBSpectrum{
public:
    //! @brief  Default constructor.
    SORT_FORCEINLINE RGBSpectrum():r(0.0f),g(0.0f),b(0.0f){}

    //! @brief  Constructor from three float values.
    //!
    //! @param  r   Value in red channel.
    //! @param  g   Value in green channel.
    //! @param  b   Value in blue channel.
    SORT_FORCEINLINE RGBSpectrum( float r , float g , float b ):r(r),g(g),b(b){}

    //! @brief  Constructor from a given OSL vec3 data type.
    //!
    //! @param  v   Data in OSL vec format.
    // SORT_FORCEINLINE RGBSpectrum( const OSL::Vec3& v ):RGBSpectrum( v.x , v.y , v.z ){}

    //! @brief  Constructor from a single value that propogates to all channels
    //!
    //! @param  g   Value to be propergated.
    SORT_FORCEINLINE RGBSpectrum( float g ):RGBSpectrum(g,g,g){}

    //! @brief  Get the value of the maximum channel.
    //!
    //! @return     The value of the maximum channel.
    SORT_FORCEINLINE float GetMaxComponent() const {
        return std::max(r, std::max(g, b));
    }

    //! @brief  Clamping the color.
    //!
    //! @param  low     Minimum value of the range of clamping.
    //! @param  high    Maximum value of the range of clamping.
    //! @return         The clamped color.
    SORT_FORCEINLINE RGBSpectrum Clamp(float low, float high) const {
        return RGBSpectrum(clamp(r, low, high), clamp(g,low,high), clamp(b,low,high));
    }

    //! = operator
    //!
    //! @param  color   Source color to copy from.
    //! @return         The copied color.
    const RGBSpectrum& operator = ( const RGBSpectrum& color ){
        x = color.x; y = color.y; z = color.z;
        return *this;
    }

    //! @brief  Return value in specific channel.
    //!
    //! It is the upper level code's repsonsibility to make sure that i is within valid range.
    //! Otherwise, it is very likely to crash the system.
    //!
    //! @param  i       Index of channel.
    //! @return         Copy of the value of intersetd.
    SORT_FORCEINLINE float operator []( int i ) const {
        sAssert( i >= 0 && i < 3 , GENERAL );
        return data[i]; 
    }

    //! @brief  Return value in specific channel.
    //!
    //! It is the upper level code's repsonsibility to make sure that i is within valid range.
    //! Otherwise, it is very likely to crash the system.
    //!
    //! @param  i       Index of channel.
    //! @return         Reference of the value of interest.
    SORT_FORCEINLINE float& operator []( int i ) {
        return data[i];
    }

    //! @brief  Whether the color is totally black.
    //!
    //! @return     Whether the color is black.
    SORT_FORCEINLINE bool IsBlack() const{
        return ( r == 0.0f ) && ( g == 0.0f ) && ( b == 0.0f );
    }

    //! @brief  Get the intensity of the color.
    //!
    //! @return     Intensity of the color.
    SORT_FORCEINLINE float GetIntensity() const{
        static const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
        return YWeight[0] * r + YWeight[1] * g + YWeight[2] * b;
    }

    //! @brief  Return the x^e of each channel.
    //!
    //! @return     A color with each channel as exp of the original color.
    SORT_FORCEINLINE RGBSpectrum Exp() const {
        return RGBSpectrum( exp( r ) , exp( g ) , exp( b ) );
    }

    //! @brief  Return the squared root of each channel.
    //!
    //! @return     A color with each channel as squared root of the original color.
    SORT_FORCEINLINE RGBSpectrum Sqrt() const {
        return RGBSpectrum( sqrt( r ) , sqrt( g ) , sqrt( b ) );
    }

    //! @brief  Whether the color is valid.
    //!
    //! @return     Whether the color contains Nan or Inf
    SORT_FORCEINLINE bool IsValid() const {
        if( isnan( r ) || isnan( g ) || isnan( b ) )
            return false;
        if( isinf( r ) || isinf( g ) || isinf( b ) )
            return false;
        return true;
    }

    union{
        struct{
            float x , y , z;
        };
        struct{
            float r , g , b;
        };
        struct{
            float data[3];
        };
    };

    static const RGBSpectrum    m_White;
};

#define WHITE_SPECTRUM      RGBSpectrum::m_White
#define FULL_WEIGHT         WHITE_SPECTRUM

SORT_STATIC_FORCEINLINE RGBSpectrum operator + ( const RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    return RGBSpectrum( c0.r + c1.r , c0.g + c1.g , c0.b + c1.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator - ( const RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    return RGBSpectrum( c0.r - c1.r , c0.g - c1.g , c0.b - c1.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator * ( const RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    return RGBSpectrum( c0.r * c1.r , c0.g * c1.g , c0.b * c1.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator / ( const RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    return RGBSpectrum( c0.r / c1.r , c0.g / c1.g , c0.b / c1.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator + ( const RGBSpectrum& c0 , const float f ){
    return RGBSpectrum( c0.r + f , c0.g + f , c0.b + f );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator - ( const RGBSpectrum& c0 , const float f ){
    return RGBSpectrum( c0.r - f , c0.g - f , c0.b - f );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator * ( const RGBSpectrum& c0 , const float f ){
    return RGBSpectrum( c0.r * f , c0.g * f , c0.b * f );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator / ( const RGBSpectrum& c0 , const float f ){
    return RGBSpectrum( c0.r / f , c0.g / f , c0.b / f );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator + ( const float f , const RGBSpectrum& c0 ){
    return RGBSpectrum( f + c0.r , f + c0.g , f + c0.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator - ( const float f , const RGBSpectrum& c0 ){
    return RGBSpectrum( f - c0.r , f - c0.g , f - c0.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator * ( const float f , const RGBSpectrum& c0 ){
    return RGBSpectrum( f * c0.r , f * c0.g , f * c0.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator / ( const float f , const RGBSpectrum& c0 ){
    return RGBSpectrum( f / c0.r , f / c0.g , f / c0.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator += ( RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    c0.r += c1.r; c0.g += c1.g; c0.b += c1.b;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator -= ( RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    c0.r -= c1.r; c0.g -= c1.g; c0.b -= c1.b;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator *= ( RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    c0.r *= c1.r; c0.g *= c1.g; c0.b *= c1.b;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator /= ( RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    c0.r /= c1.r; c0.g /= c1.g; c0.b /= c1.b;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator += ( RGBSpectrum& c0 , const float f ){
    c0.r += f; c0.g += f; c0.b += f;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator -= ( RGBSpectrum& c0 , const float f ){
    c0.r -= f; c0.g -= f; c0.b -= f;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator *= ( RGBSpectrum& c0 , const float f ){
    c0.r *= f; c0.g *= f; c0.b *= f;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator /= ( RGBSpectrum& c0 , const float f ){
    c0.r /= f; c0.g /= f; c0.b /= f;
    return c0;
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator == ( const RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    return ( c0.r == c1.r ) && ( c0.g == c1.g ) && ( c0.b == c1.b );
}

SORT_STATIC_FORCEINLINE RGBSpectrum operator != ( const RGBSpectrum& c0 , const RGBSpectrum& c1 ){
    return ( c0.r != c1.r ) || ( c0.g != c1.g ) || ( c0.b != c1.b );
}