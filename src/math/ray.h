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
#include "math/vector3.h"
#include "math/point.h"
#include "float.h"
#include "spectrum/spectrum.h"

////////////////////////////////////////////////////////////////////////////
class Ray
{
public:
	// default constructor
	Ray();
	// constructor from a point and a direction
	// para 'ori'   :	original point of the ray
	// para 'dir'   :	direction of the ray , it's the programmer's responsibility to normalize it
	// para 'depth' :	the depth of the curren ray , if not set , default value is 0
	// para 'fmin'  :	the minium range of the ray . It could be set a very small value to avoid false self intersection
	// para 'fmax'  :	the maxium range of the ray . A ray with 'fmax' not equal to 0 is actually a line segment, usually used for shadow ray.
	Ray( const Point& ori , const Vector& dir , unsigned depth = 0 , float fmin = 0.0f , float fmax = FLT_MAX );
	// copy constructor
	// para 'r' :	a ray to copy
	Ray( const Ray& r );
	// destructor
	~Ray();
	
	// operator to get a point from the ray
	// para 't' :	the distance from the retrive point if the direction of the ray is normalized.
	// reslut   :	A point ( o + t * d )
	Point operator()(float t) const;

// the original point and direction are also public
	// original point of the ray
	Point	m_Ori;
	// direction , the direction of the ray
	Vector	m_Dir;

	// the depth for the ray
	int		m_Depth;

	// the maxium and minium value in the ray
	float	m_fMin;
	float	m_fMax;

	float	m_fPdfW;
	float	m_fPdfA;
	float	m_fCosAtCamera;

	// importance value of the ray
	Spectrum m_we;
};
