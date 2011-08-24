/*
   FileName:      square.cpp

   Created Time:  2011-08-21 11:38:05

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "square.h"
#include "geometry/intersection.h"
#include "sampler/sample.h"

// sample a point on shape
Point Square::sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf ) const
{
	float u = 2 * ls.u - 1.0f;
	float v = 2 * ls.v - 1.0f;
	Point lp = transform( Point( radius * u , 0.0f , radius * v ) );
	Vector normal = transform( Vector( 0 , 1 , 0 ) );
	Vector delta = lp - p;
	wi = Normalize( delta );

	float dot = SatDot( -wi , normal );
	if( pdf ) 
	{
		if( dot == 0 )
			*pdf = 0.0f;
		else
			*pdf = delta.SquaredLength() / ( PI * radius * radius * dot );
	}

	return lp;
}

// get pdf of specific direction
float Square::Pdf( const Point& p , const Point& lp ,  const Vector& wi ) const
{
	Vector normal = transform( Vector( 0.0f , 1.0f , 0.0f ) );
	float dot = Dot( normal , wi );
	if( dot <= 0.0f )
		return 0.0f;

	Vector delta = ( p - lp );

	return delta.SquaredLength() / ( PI * radius * radius * dot );
}

// the surface area of the shape
float Square::SurfaceArea() const
{
	return PI * radius * radius * 4.0f;
}

// get intersection between the light surface and the ray
bool Square::GetIntersect( const Ray& ray , Intersection* intersect ) const
{
	if( ray.m_Dir.z == 0.0f )
		return false;

	float t = -ray.m_Ori.y / ray.m_Dir.y;
	if( t > intersect->t || t <= 0.0f )
		return false;
	Point p = ray(t);

	if( p.x > radius || p.x < -radius )
		return false;
	if( p.z > radius || p.z < -radius )
		return false;

	intersect->t = t;
	intersect->intersect = ( ray(intersect->t) );
	intersect->normal = Vector( 0.0f , 1.0f , 0.0f , true );

	return true;
}
