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

// include the header
#include "instancetri.h"
#include "intersection.h"
#include "trimesh.h"

// constructor from a triangle
InstanceTriangle::InstanceTriangle( unsigned pid , const TriMesh* mesh , const VertexIndex* index , Transform* t , std::shared_ptr<Material>& mat ):
Triangle( pid , mesh , index , mat) , transform( t )
{
}

// destructor
InstanceTriangle::~InstanceTriangle()
{
}

// get the intersection
bool	InstanceTriangle::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	// transform the ray
	Ray ray = transform->invMatrix( r ) ;

	// get the intersection result 
	bool result =  Triangle::GetIntersect( ray , intersect );
	if( result == false )
		return false;

	// transform the intersection
	if( intersect )
	{
		intersect->intersect = (*transform)(intersect->intersect);
		intersect->normal = ((transform->invMatrix.Transpose())(intersect->normal)).Normalize();	// to be changed
		intersect->tangent = ((*transform)(intersect->tangent)).Normalize();
	}

	return true;
}

// get the bounding box of the triangle
const BBox& InstanceTriangle::GetBBox() const
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
		// to be modified
        m_bbox = std::unique_ptr<BBox>( new BBox() );

        const auto& mem = m_trimesh->m_pMemory;
		int id0 = m_Index[0].posIndex;
		int id1 = m_Index[1].posIndex;
		int id2 = m_Index[2].posIndex;
	
		// get three vertexes
		const Point& p0 = (*transform)(mem->m_PositionBuffer[id0]);
		const Point& p1 = (*transform)(mem->m_PositionBuffer[id1]);
		const Point& p2 = (*transform)(mem->m_PositionBuffer[id2]);

		// transform the point
		m_bbox->Union( p0 );
		m_bbox->Union( p1 );
		m_bbox->Union( p2 );
	}

	return *m_bbox;
}
