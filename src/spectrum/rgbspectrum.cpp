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

#include "rgbspectrum.h"
#include "core/define.h"

const RGBSpectrum RGBSpectrum::m_White(1.0f);

unsigned int RGBSpectrum::GetColor() const{
    unsigned int color = 0;

    color |= ((unsigned char)(255.0f*saturate(data.r)))<<16;
    color |= ((unsigned char)(255.0f*saturate(data.g)))<<8;
    color |= ((unsigned char)(255.0f*saturate(data.b)))<<0;

    return color;
}

void RGBSpectrum::SetColor( unsigned int color ){
    data.r = ((float)( ( color >> 16 ) & 255 )) / 255.0f;
    data.g = ((float)( ( color >> 8 ) & 255 )) / 255.0f;
    data.b = ((float)( ( color >> 0 ) & 255 )) / 255.0f;
}

RGBSpectrum RGBSpectrum::operator + ( const RGBSpectrum& c ) const{
    return data + c.data;
}

RGBSpectrum RGBSpectrum::operator - ( const RGBSpectrum& c ) const{
    return data - c.data;
}

RGBSpectrum RGBSpectrum::operator * ( const RGBSpectrum& c ) const{
    return data * c.data;
}

RGBSpectrum RGBSpectrum::operator / ( const RGBSpectrum& c ) const{
    return data / c.data;
}

RGBSpectrum RGBSpectrum::operator + ( float t ) const{
    return data + t;
}

RGBSpectrum RGBSpectrum::operator - ( float t ) const{
    return data - t;
}

RGBSpectrum RGBSpectrum::operator * ( float t ) const{
    return data * t;
}

RGBSpectrum RGBSpectrum::operator / ( float t ) const{
    return data / t;
}

float RGBSpectrum::GetR() const {
    return data.r;
}
float RGBSpectrum::GetG() const {
    return data.g;
}
float RGBSpectrum::GetB() const {
    return data.b;
}
float RGBSpectrum::GetMaxComponent() const{
    return std::max( data.r , std::max( data.g , data.b ) );
}

RGBSpectrum RGBSpectrum::Clamp( float low , float high ) const{
    return RGBSpectrum( clamp(data, low, high) );
}
