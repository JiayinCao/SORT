/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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

#include <OSL/oslexec.h>
#include "core/define.h"
#include "math/utils.h"
#include "math/float3.h"

#define RGBSPECTRUM_SAMPLE      3

///////////////////////////////////////////////////////////////////
// definition of rgb spectrum
class   RGBSpectrum{
public:
    // default constructor
    RGBSpectrum(){}
    // constructor from three float
    RGBSpectrum( float r , float g , float b ):data( r , g , b ) {}
    // constructor from three unsigned char
    RGBSpectrum( unsigned char r , unsigned char g , unsigned char b ):data( r / 255.0f , g / 255.0f , b / 255.0f ){}
    // constructor from a OSL vector3
    RGBSpectrum( const OSL::Vec3& v ):data( v.x , v.y , v.z ){}
    // constructor from one float
    RGBSpectrum( float g ):data(g){}
    // constructor from and unsigned char
    RGBSpectrum( unsigned char g ):data(g/255.0f){}
    // constructor from float3
    RGBSpectrum( const float3& color ):data(color){}
    // destructor
    ~RGBSpectrum() = default;

    // get the color
    unsigned int GetColor() const;
    // set the color
    void SetColor( unsigned int color );
    void SetColor( float r , float g , float b ){
        data = float3( r , g , b );
    }
    // get each component
    float   GetR() const;
    float   GetG() const;
    float   GetB() const;
    float   GetMaxComponent() const;

    // clamp the spectrum
    RGBSpectrum Clamp( float low = 0.0f , float high = 0.0f ) const;

    // operators
    RGBSpectrum operator+( const RGBSpectrum& c ) const;
    RGBSpectrum operator-( const RGBSpectrum& c ) const;
    RGBSpectrum operator*( const RGBSpectrum& c ) const;
    RGBSpectrum operator/( const RGBSpectrum& c ) const;

    RGBSpectrum operator+( float t ) const;
    RGBSpectrum operator-( float t ) const;
    RGBSpectrum operator*( float t ) const;
    RGBSpectrum operator/( float t ) const;

    RGBSpectrum& operator+= ( const RGBSpectrum& c ) { *this = *this + c ; return *this; }
    RGBSpectrum& operator-= ( const RGBSpectrum& c ) { *this = *this - c ; return *this; }
    RGBSpectrum& operator*= ( const RGBSpectrum& c ) { *this = *this * c ; return *this; }
    RGBSpectrum& operator/= ( const RGBSpectrum& c ) { *this = *this / c ; return *this; }

    RGBSpectrum& operator+=( float t ){ *this = *this + t ; return *this; }
    RGBSpectrum& operator-=( float t ){ *this = *this - t ; return *this; }
    RGBSpectrum& operator*=( float t ){ *this = *this * t ; return *this; }
    RGBSpectrum& operator/=( float t ){ *this = *this / t ; return *this; }

    inline float operator []( int i ) const { return data[i]; }
    inline float& operator []( int i ) { return data[i]; }

    //whether the spectrum is black
    bool IsBlack() const{
        return data.isZero();
    }

    // get intensity
    float GetIntensity() const{
        static const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
        return YWeight[0] * data.r + YWeight[1] * data.g + YWeight[2] * data.b;
    }

    void ToLinear(){
        data.r = GammaToLinear(data.r);
        data.g = GammaToLinear(data.g);
        data.b = GammaToLinear(data.b);
    }
    void ToGamma(){
        data.r = LinearToGamma(data.r);
        data.g = LinearToGamma(data.g);
        data.b = LinearToGamma(data.b);
    }

    RGBSpectrum Exp() const {
        return exp(data);
    }

    RGBSpectrum Sqrt() const {
        return sqrt(data);
    }

private:
    float3  data;

    friend inline RGBSpectrum operator-( float t , const RGBSpectrum& s );

public:
    static const RGBSpectrum    m_White;
};

inline RGBSpectrum operator+( float t , const RGBSpectrum& s ){
    return s + t;
}
inline RGBSpectrum operator-( float t , const RGBSpectrum& s ){
    return t - s.data;
}
inline RGBSpectrum operator*( float t , const RGBSpectrum& s ){
    return s * t;
}

#define WHITE_SPECTRUM      RGBSpectrum::m_White
#define FULL_WEIGHT         WHITE_SPECTRUM