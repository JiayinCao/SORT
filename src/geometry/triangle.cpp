/*
 * filename :	triangle.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "triangle.h"
#include "trimesh.h"
#include "point.h"
#include "vector.h"
#include "ray.h"

// constructor
Triangle::Triangle( const TriMesh* trimesh , const Point* vb , const unsigned* index ):
m_trimesh( trimesh ) , m_ib( index ) , m_vb( vb )
{
}

// check if the triangle is intersected with the ray
float Triangle::Intersect( const Ray& r ) const
{
	// get three vertexes
	const Point& p0 = m_vb[ m_ib[0] ];
	const Point& p1 = m_vb[ m_ib[1] ];
	const Point& p2 = m_vb[ m_ib[2] ];

	// get the vector
	Vector v0 = p1 - p0;
	Vector v1 = p2 - p1;
	Vector v2 = p0 - p2;

	// get the normal of the plane
	Vector n = Cross( v1 , v2 );

	// get the intersected point
	float t = Dot( n , p0 - r.m_Ori ) / Dot( n , r.m_Dir );
	if( t < 0.0f )
		return t;
	Point p = r(t);

	Vector _v0 = p - p0;
	Vector _v1 = p - p1;
	Vector _v2 = p - p2;

	Vector r0 = Cross( _v0 , v0 );
	Vector r1 = Cross( _v1 , v1 );
	Vector r2 = Cross( _v2 , v2 );

	if( Dot( r0 , r1 ) > 0.0f && Dot( r0 , r2 ) > 0.0f )
		return t;

	// the ray doesn't cross the triangle
	return -1.0f;
}
