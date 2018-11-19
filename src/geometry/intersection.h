/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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

// include the header
#include "math/point.h"
#include "spectrum/spectrum.h"
#include <float.h>

// pre-decleration class
class Primitive;

///////////////////////////////////////////////////////////////////////
//	definition of intersection
class	Intersection
{
public :
	// get the emissive
	Spectrum Le( const Vector& wo , float* directPdfA = 0 , float* emissionPdf = 0 ) const;

// public field
public:
	// the interesection point
	Point	intersect;
	// the shading normal
	Vector	normal;
    // the geometry normal
    Vector  gnormal;
	// tangent vector
	Vector	tangent;
	// the uv coordinate
    float	u = 0.0f , v = 0.0f;
	// the delta distance from the orginal point
	float	t = FLT_MAX;
	// the intersected primitive
	Primitive* 	primitive = nullptr;
};