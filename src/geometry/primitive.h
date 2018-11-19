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

#include <memory>
#include "bbox.h"
#include "material/material.h"
#include "intersection.h"
#include "shape/shape.h"

class	Intersection;
class	Light;

//////////////////////////////////////////////////////////////////
//	definition of primitive
class	Primitive
{
public:
	// constructor from a id
    Primitive( std::shared_ptr<Material> mat , Shape* shape ):m_mat(mat), m_shape(shape), light(nullptr){}

	// get the intersection between a ray and a primitive
	inline bool GetIntersect( const Ray& r , Intersection* intersect ) const{
		Point tmp;
		bool ret = m_shape->GetIntersect( r, tmp , intersect );
		if( ret && intersect )
			intersect->primitive = const_cast<Primitive*>(this);
		return ret;
	}
	// get the intersection between a bounding box and a ray
    inline bool GetIntersect( const BBox& box ) const { 
		return m_shape->GetIntersect( box );
	}

	// get the bounding box of the primitive
	inline const BBox&	GetBBox() const {
		return m_shape->GetBBox();
	}

	// get surface area of the primitive
	inline float	SurfaceArea() const{
		return m_shape->SurfaceArea();
	}

	// get material
    std::shared_ptr<Material> GetMaterial() const;
	// set material
    inline void	SetMaterial( std::shared_ptr<Material>& mat ) { m_mat = mat; }

	// get light
	inline Light* GetLight() const { return light; }
    // set light
    inline void SetLight( Light* _light ) { light = _light; }

// protected field
protected:
	// the material
    std::shared_ptr<Material>	m_mat;

	// the shape of the primitive
	Shape* 		m_shape;

	// the binded light
	Light*		light;
};
