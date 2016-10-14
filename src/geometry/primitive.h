/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_PRIMITIVE
#define	SORT_PRIMITIVE

// include header file
#include "utility/smartptr.h"
#include "bbox.h"
#include "material/material.h"
#include "utility/referencecount.h"
#include "intersection.h"

// pre-decleration
class	Intersection;
class	Light;

//////////////////////////////////////////////////////////////////
//	definition of primitive
class	Primitive
{
// public method
public:
	// constructor from a id
	Primitive( unsigned id , Material* mat ) { m_primitive_id = id; m_mat = mat; light = 0; }
	// destructor
	virtual ~Primitive(){}

	// get the intersection between a ray and a primitive
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const = 0;
	// get the intersection between a bounding box and a ray
    virtual bool GetIntersect( const BBox& box ) const { return true; }

	// get the bounding box of the primitive
	virtual const BBox&	GetBBox() const = 0;

	// get surface area of the primitive
	virtual float	SurfaceArea() const = 0;

	// delete the cache
	virtual void ClearBBoxCache();

	// set primitive id
	void	SetID( unsigned id ) { m_primitive_id = id; }
	// get primitive id
	unsigned GetID() const { return m_primitive_id; }

	// get material
	Material* GetMaterial() const;
	// set material
	void	SetMaterial( Material* mat ) { m_mat = mat; }

	// get light
	Light* GetLight() const { return light; }

// protected field
protected:
	// bounding box
	mutable SmartPtr<BBox> m_bbox;
	// id for the primitive
	unsigned		m_primitive_id;
	// the material
	Reference<Material>	m_mat;

	// the binded light
	Light*		light;
};

#endif
