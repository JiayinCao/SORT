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

#include "core/define.h"
#include "spectrum/spectrum.h"
#include "math/vector3.h"

SORT_FORCEINLINE float cosTheta(const Vector &w){
    return w.y;
}

SORT_FORCEINLINE float absCosTheta(const Vector &w){
    return fabsf(w.y);
}

SORT_FORCEINLINE float sinTheta2(const Vector &w){
    return std::max(0.f, 1.f - cosTheta(w)*cosTheta(w));
}

SORT_FORCEINLINE float cosTheta2(const Vector &w){
    return cosTheta(w) * cosTheta(w);
}

SORT_FORCEINLINE float tanTheta2(const Vector &w){
    return 1.0f / cosTheta2(w) - 1.0f;
}

SORT_FORCEINLINE float cosDPhi( const Vector& w0 , const Vector& w1 ){
    return clamp( ( w0.x * w1.x + w0.z * w1.z ) / sqrt( (w0.x * w0.x + w0.z * w0.z)*(w1.x * w1.x + w1.z*w1.z) ) , -1.0f , 1.0f );
}
SORT_FORCEINLINE float sinTheta(const Vector &w){
    return sqrtf(sinTheta2(w));
}
SORT_FORCEINLINE float cosPhi(const Vector &w){
    float sintheta = sinTheta(w);
    if (sintheta == 0.f) return 1.f;
    return clamp(w.x / sintheta, -1.f, 1.f);
}

SORT_FORCEINLINE float sinPhi(const Vector &w) {
    float sintheta = sinTheta(w);
    if (sintheta == 0.f) return 0.f;
    return clamp(w.z / sintheta, -1.f, 1.f);
}

SORT_FORCEINLINE float sinPhi2(const Vector &w) {
    const float sinphi = sinPhi(w);
    return sinphi * sinphi;
}

SORT_FORCEINLINE float cosPhi2(const Vector &w) {
    const float cosphi = cosPhi(w);
    return cosphi * cosphi;
}

SORT_FORCEINLINE float tanTheta( const Vector& w){
    return sinTheta(w) / cosTheta(w);
}

SORT_FORCEINLINE bool sameHemisphere(const Vector &w, const Vector &wp) {
    return w.y * wp.y > 0.f;
}

SORT_FORCEINLINE float sphericalTheta(const Vector &v) {
    return acosf(clamp(v.y, -1.f, 1.f));
}

SORT_FORCEINLINE float sphericalPhi(const Vector &v) {
    float p = atan2f(v.z, v.x);
    return (p < 0.f) ? p + 2.f*PI : p;
}

SORT_FORCEINLINE Vector sphericalVec( float theta , float phi ){
    float x = sin( theta ) * cos( phi );
    float y = cos( theta );
    float z = sin( theta ) * sin( phi );

    return Vector( x , y , z );
}

SORT_FORCEINLINE Vector sphericalVec( float sintheta , float costheta , float phi ){
    float x = sintheta * cos( phi );
    float y = costheta;
    float z = sintheta * sin( phi );
    return Vector( x , y , z );
}

SORT_FORCEINLINE Vector reflect(const Vector& v, const Vector& n){
    return (2.0f * dot(v, n)) * n - v;
}

// an optimized version only works in shading coordinate
SORT_FORCEINLINE Vector reflect(const Vector& v) {
    return Vector(-v.x, v.y, -v.z);
}

//! @param v                    Incident direction. It can come from either inside or outside of the surface.
//! @param n                    Surface normal.
//! @param in_eta               Index of refraction inside the surface.
//! @param ext_eta              Index of refraction outside the surface.
//! @param inner_reflection     Whether it is a total inner reflection.
//! @return                     Refracted vector based on Snell's law.
//! @Note                       Both vectors ( the first parameter and the returned value ) should be pointing outside the surface.
SORT_FORCEINLINE Vector refract(const Vector& v, const Vector& n, float in_eta, float ext_eta, bool& inner_reflection){
    const float coso = dot(v, n);
    const float eta = coso > 0 ? (ext_eta / in_eta) : (in_eta / ext_eta);
    const float t = 1.0f - eta * eta * std::max(0.0f, 1.0f - coso * coso);

    // total inner reflection
    inner_reflection = (t <= 0.0f);
    if (inner_reflection)
        return Vector(0.0f, 0.0f, 0.0f);
    const float scale = coso < 0.0f ? -1.0f : 1.0f;
    return -eta * v + (eta * coso - scale * sqrt(t)) * n;
}
