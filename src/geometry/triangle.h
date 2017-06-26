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

#ifndef	SORT_TRIANGLE
#define	SORT_TRIANGLE

#include "primitive.h"

// pre-decleration
class TriMesh;
struct VertexIndex;

//////////////////////////////////////////////////////////////////////////////////
//	definition of triangle
//	note: triangle is the only primitive supported by the system.
class	Triangle : public Primitive
{
// public method
public:
	// constructor
	// para 'pid'     : primitive id
	// para 'trimesh' : the triangle mesh it belongs to
	// para 'index'   : the index buffer
    Triangle( unsigned pid , const TriMesh* mesh , const VertexIndex* index , std::shared_ptr<Material>& mat):
		Primitive(pid,mat) , m_trimesh(mesh) , m_Index(index) {}
	// destructor
	~Triangle(){}

	// check if the triangle is intersected with the ray
	// para 'r' : the ray to check
	// para 'intersect' : the result storing the intersection information
	//					  the intersection is an optimized versiion
	// result   : positive value if intersect
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	// intersection test between a triangle and a bounding box
	// Detail algorithm is descripted in this paper : "Fast 3D Triangle-Box Overlap Testing".
	// http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox_tam.pdf.
	// para 'box' : bounding box
	bool GetIntersect( const BBox& box ) const;

	// get the bounding box of the triangle
	virtual const BBox&	GetBBox() const;

	// get the surface area
	virtual float SurfaceArea() const;

// protected filed
protected:
	// the triangle mesh
	const TriMesh*		m_trimesh;
	// the index
	const VertexIndex*	m_Index;
};

#endif
