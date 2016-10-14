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

#ifndef	SORT_INSTANCETRI
#define	SORT_INSTANCETRI

// include the header
#include "triangle.h"

// pre-declera class
class Transform;

////////////////////////////////////////////////////////////////////////////////
// definition of instance triangle
// note :	an instance triangle is just a copy of a triangle, it doesn't 
//			save the position data of the triangle. when performing intersection
//			test , it first transform the ray from world space to object space
//			and then to the prototype-primitive space, it costs more than
//			original triangle , but saves memory space.
class	InstanceTriangle : public Triangle
{
// public method
public:
	// constructor from a Triangle
	InstanceTriangle( unsigned pid , const TriMesh* mesh , const VertexIndex* index , Transform* transform , Material* mat  );
	// destructor
	~InstanceTriangle();

	// get the instersection between a ray and a instance triangle
	bool	GetIntersect( const Ray& r , Intersection* intersect ) const;	
	
	// get the bounding box of the triangle
	const BBox&	GetBBox() const;

// private field
private:
	// the transformation of the triangle
	Transform* transform;
};

#endif
