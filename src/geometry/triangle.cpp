/*
   FileName:      triangle.cpp

   Created Time:  2011-08-04 12:51:06

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "triangle.h"
#include "trimesh.h"
#include "intersection.h"
#include "utility/samplemethod.h"

// check if the triangle is intersected with the ray
bool Triangle::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	// get the memory
	// note : reference is not used here because it's not thread-safe
	BufferMemory* mem = m_trimesh->m_pMemory;
	int id0 = m_Index[0].posIndex;
	int id1 = m_Index[1].posIndex;
	int id2 = m_Index[2].posIndex;

	// get three vertexes
	const Point& p0 = mem->m_PositionBuffer[id0] ;
	const Point& p1 = mem->m_PositionBuffer[id1] ;
	const Point& p2 = mem->m_PositionBuffer[id2] ;

	float delta = 0.0001f;
	Vector e1 = p1 - p0;
	Vector e2 = p2 - p0;
	Vector s1 = Cross( r.m_Dir , e2 );
	float divisor = Dot( s1 , e1 );
	if( fabs(divisor) < delta )
		return false;
	float invDivisor = 1.0f / divisor;

	Vector d = r.m_Ori - p0;
	float u = Dot( d , s1 ) * invDivisor;
	if( u < -delta || u > 1.0f + delta )
		return false;
	Vector s2 = Cross( d , e1 );
	float v = Dot( r.m_Dir , s2 ) * invDivisor;
	if( v < -delta || u + v > 1.0f + delta )
		return false;
	float t = Dot( e2 , s2 ) * invDivisor;
	if( t < r.m_fMin || t > r.m_fMax )
		return false;

	if( intersect == 0 )
		return t > r.m_fMin && t < r.m_fMax;

	// if t is out of range , return false
	if( t > intersect->t )
		return false;

	// store the intersection
	intersect->intersect = r(t);

	float w = 1 - u - v;

	// store normal if the info is available
	id0 = m_Index[0].norIndex;
	id1 = m_Index[1].norIndex;
	id2 = m_Index[2].norIndex;

	intersect->normal = ( w * mem->m_NormalBuffer[id0] + u * mem->m_NormalBuffer[id1] + v * mem->m_NormalBuffer[id2]).Normalize();
	intersect->tangent = ( w * mem->m_TangentBuffer[id0] + u * mem->m_TangentBuffer[id1] + v * mem->m_TangentBuffer[id2]).Normalize();

	// store texture coordinate
	if( mem->m_iTBCount > 0 )
	{
		id0 = 2*m_Index[0].texIndex;
		id1 = 2*m_Index[1].texIndex;
		id2 = 2*m_Index[2].texIndex;

		float u0 = mem->m_TexCoordBuffer[id0];
		float u1 = mem->m_TexCoordBuffer[id1];
		float u2 = mem->m_TexCoordBuffer[id2];
		float v0 = mem->m_TexCoordBuffer[id0+1];
		float v1 = mem->m_TexCoordBuffer[id1+1];
		float v2 = mem->m_TexCoordBuffer[id2+1];
		intersect->u = w * u0 + u * u1 + v * u2;
		intersect->v = w * v0 + u * v1 + v * v2;
	}else
	{
		intersect->u = 0.0f;
		intersect->v = 0.0f;
	}

	intersect->t = t;
	intersect->primitive = const_cast<Triangle*>(this);

    return t > 0.0f ;
}

// get the bounding box of the triangle
const BBox& Triangle::GetBBox() const
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
		m_bbox = new BBox();

		// get the memory
		Reference<BufferMemory> mem = m_trimesh->m_pMemory;
		int id0 = m_Index[ 0 ].posIndex;
		int id1 = m_Index[ 1 ].posIndex;
		int id2 = m_Index[ 2 ].posIndex;

		// get three vertexes
		const Point& p0 = mem->m_PositionBuffer[id0] ;
		const Point& p1 = mem->m_PositionBuffer[id1] ;
		const Point& p2 = mem->m_PositionBuffer[id2] ;

		m_bbox->Union( p0 );
		m_bbox->Union( p1 );
		m_bbox->Union( p2 );
	}

	return *m_bbox;
}

// get the surface area of the triangle
float Triangle::SurfaceArea() const
{
	// get the memory
	Reference<BufferMemory> mem = m_trimesh->m_pMemory;
	int id0 = m_Index[ 0 ].posIndex;
	int id1 = m_Index[ 1 ].posIndex;
	int id2 = m_Index[ 2 ].posIndex;

	// get three vertexes
	const Point& p0 = mem->m_PositionBuffer[id0] ;
	const Point& p1 = mem->m_PositionBuffer[id1] ;
	const Point& p2 = mem->m_PositionBuffer[id2] ;
	
	Vector e0 = p1 - p0 ;
	Vector e1 = p2 - p0 ;
	Vector t = Cross( e0 , e1 );

	return t.Length() * 0.5f;
}

// Project vertex along specific axis
static void Project(const Point* points, int count , const Vector& axis, float& min, float& max)
{
	for( int i = 0; i < count; ++i ){
		float val = Dot( axis , (Vector)points[i] );
		if (val < min) min = val;
		if (val > max) max = val;
	}
}

// intersection test between a triangle and a bounding box
// Detail algorithm is descripted in this paper : "Fast 3D Triangle-Box Overlap Testing".
// http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox_tam.pdf.
// para 'triangle' : the input triangle
// para 'bb' : bounding box
bool Triangle::GetIntersect( const BBox& box ) const
{
	// get the memory
	Reference<BufferMemory> mem = m_trimesh->m_pMemory;
	int id0 = m_Index[ 0 ].posIndex;
	int id1 = m_Index[ 1 ].posIndex;
	int id2 = m_Index[ 2 ].posIndex;
	Point tri[3] = { mem->m_PositionBuffer[id0] , mem->m_PositionBuffer[id1] , mem->m_PositionBuffer[id2] };

	float triMin , triMax;	// will intialize later
	float boxMin = FLT_MAX, boxMax = -FLT_MAX;

	Vector	boxN[3] = { Point( 1.0f , 0.0f , 0.0f ) , 
						Point( 0.0f , 1.0f , 0.0f ) ,
						Point( 0.0f , 0.0f , 1.0f )};
	
	// Case 1 : try separating axis perpendicular to box surface normal , 3 tests
	// along bounding box face normal directions first
	for( int i = 0 ; i < 3 ; ++i ){
		triMin = FLT_MAX;
		triMax = -FLT_MAX;
		Project( tri , 3 , boxN[i] , triMin , triMax );
		if( triMin > box.m_Max[i] || triMax < box.m_Min[i] )
			return false;
	}

	// Case 2 : try triangle plane , 1 test
	// get triangle normal
	Vector triN = Cross( tri[1] - tri[0] , tri[2] - tri[0] );
	//triN = triN * ( 1.0f / triN.Length() );	// no need to normalize it at all
	float triOffset = Dot( triN , (Vector)tri[0] );
	Point bbp[8] = { box.m_Min , 
		Point( box.m_Min.x , box.m_Min.y , box.m_Max.z ) ,
		Point( box.m_Min.x , box.m_Max.y , box.m_Min.z ),
		Point( box.m_Min.x , box.m_Max.y , box.m_Max.z ),
		Point( box.m_Max.x , box.m_Min.y , box.m_Min.z ),
		Point( box.m_Max.x , box.m_Min.y , box.m_Max.z ),
		Point( box.m_Max.x , box.m_Max.y , box.m_Min.z ),
		box.m_Max };
	Project( bbp , 8 , triN , boxMin , boxMax );
	if( boxMax < triOffset || boxMin > triOffset )
		return false;

	// Case 3 : try cross product planes , 9 tests
	Vector triangleEdges[3] = { tri[0] - tri[1] , tri[1] - tri[2] , tri[2] - tri[3] };
	for( int i = 0 ; i < 3 ; ++i ){
		for( int j = 0 ; j < 3 ; ++j ){
			triMin = boxMin = FLT_MAX;
			triMax = boxMax = -FLT_MAX;
			Vector new_axis = Cross( triangleEdges[i] , boxN[j] );
			Project( bbp , 8 , new_axis , boxMin , boxMax );
			Project( tri , 3 , new_axis , triMin , triMax );

			if( boxMax < triMin || boxMin > triMax )
				return false;
		}
	}

	return true;
}