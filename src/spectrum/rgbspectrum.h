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
    SORT_FORCEINLINE RGBSpectrum(){}
    // constructor from three float
    SORT_FORCEINLINE RGBSpectrum( float r , float g , float b ):data( r , g , b ) {}
    // constructor from three unsigned char
    SORT_FORCEINLINE RGBSpectrum( unsigned char r , unsigned char g , unsigned char b ):data( r / 255.0f , g / 255.0f , b / 255.0f ){}
    // constructor from a OSL vector3
    SORT_FORCEINLINE RGBSpectrum( const OSL::Vec3& v ):data( v.x , v.y , v.z ){}
    // constructor from one float
    SORT_FORCEINLINE RGBSpectrum( float g ):data(g){}
    // constructor from and unsigned char
    SORT_FORCEINLINE RGBSpectrum( unsigned char g ):data(g/255.0f){}
    // constructor from float3
    SORT_FORCEINLINE RGBSpectrum( const float3& color ):data(color){}
    // destructor
    SORT_FORCEINLINE ~RGBSpectrum() = default;

    // get the color
    unsigned int GetColor() const;
    // set the color
    void SetColor( unsigned int color );
    SORT_FORCEINLINE void SetColor( float r , float g , float b ){
        data = float3( r , g , b );
    }

	SORT_FORCEINLINE RGBSpectrum operator + (const RGBSpectrum& c) const {
		return data + c.data;
	}

	SORT_FORCEINLINE RGBSpectrum operator - (const RGBSpectrum& c) const {
		return data - c.data;
	}

	SORT_FORCEINLINE RGBSpectrum operator * (const RGBSpectrum& c) const {
		return data * c.data;
	}

	SORT_FORCEINLINE RGBSpectrum operator / (const RGBSpectrum& c) const {
		return data / c.data;
	}

	SORT_FORCEINLINE RGBSpectrum operator + (float t) const {
		return data + t;
	}

	SORT_FORCEINLINE RGBSpectrum operator - (float t) const {
		return data - t;
	}

	SORT_FORCEINLINE RGBSpectrum operator * (float t) const {
		return data * t;
	}

	SORT_FORCEINLINE RGBSpectrum operator / (float t) const {
		return data / t;
	}

	SORT_FORCEINLINE float GetR() const {
		return data.r;
	}

	SORT_FORCEINLINE float GetG() const {
		return data.g;
	}

	SORT_FORCEINLINE float GetB() const {
		return data.b;
	}

	SORT_FORCEINLINE float GetMaxComponent() const {
		return std::max(data.r, std::max(data.g, data.b));
	}

	SORT_FORCEINLINE RGBSpectrum Clamp(float low, float high) const {
		return RGBSpectrum(clamp(data, low, high));
	}

	SORT_FORCEINLINE RGBSpectrum& operator+= (const RGBSpectrum& c) {
		return *this = *this + c; 
	}

	SORT_FORCEINLINE RGBSpectrum& operator-= (const RGBSpectrum& c) {
		return *this = *this - c;
	}

	SORT_FORCEINLINE RGBSpectrum& operator*= (const RGBSpectrum& c) {
		return *this = *this * c;
	}

	SORT_FORCEINLINE RGBSpectrum& operator/= (const RGBSpectrum& c) {
		return *this = *this / c;
	}

	SORT_FORCEINLINE RGBSpectrum& operator+=(float t) {
		return *this = *this + t;
	}

	SORT_FORCEINLINE RGBSpectrum& operator-=(float t) {
		return *this = *this - t;
	}

	SORT_FORCEINLINE RGBSpectrum& operator*=(float t) {
		return *this = *this * t;
	}

	SORT_FORCEINLINE RGBSpectrum& operator/=(float t) {
		return *this = *this / t;
	}

    SORT_FORCEINLINE float operator []( int i ) const {
		return data[i]; 
	}

    SORT_FORCEINLINE float& operator []( int i ) {
		return data[i];
	}

    // whether the spectrum is black
    SORT_FORCEINLINE bool IsBlack() const{
        return data.isZero();
    }

    // get intensity
    SORT_FORCEINLINE float GetIntensity() const{
        static const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
        return YWeight[0] * data.r + YWeight[1] * data.g + YWeight[2] * data.b;
    }

    SORT_FORCEINLINE void ToLinear(){
        data.r = GammaToLinear(data.r);
        data.g = GammaToLinear(data.g);
        data.b = GammaToLinear(data.b);
    }
    SORT_FORCEINLINE void ToGamma(){
        data.r = LinearToGamma(data.r);
        data.g = LinearToGamma(data.g);
        data.b = LinearToGamma(data.b);
    }

    SORT_FORCEINLINE RGBSpectrum Exp() const {
        return exp(data);
    }

    SORT_FORCEINLINE RGBSpectrum Sqrt() const {
        return sqrt(data);
    }

private:
    float3  data;

    friend SORT_FORCEINLINE RGBSpectrum operator-( float t , const RGBSpectrum& s );

public:
    static const RGBSpectrum    m_White;
};

SORT_FORCEINLINE  RGBSpectrum operator+( float t , const RGBSpectrum& s ){
    return s + t;
}
SORT_FORCEINLINE  RGBSpectrum operator-( float t , const RGBSpectrum& s ){
    return t - s.data;
}
SORT_FORCEINLINE  RGBSpectrum operator*( float t , const RGBSpectrum& s ){
    return s * t;
}

#define WHITE_SPECTRUM      RGBSpectrum::m_White
#define FULL_WEIGHT         WHITE_SPECTRUM