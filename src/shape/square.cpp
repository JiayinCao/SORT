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
#include "utility/samplemethod.h"
#include "utility/rand.h"

IMPLEMENT_CREATOR( Square );

// sample a point on shape
Point Square::sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf ) const
{
	float u = 2 * ls.u - 1.0f;
	float v = 2 * ls.v - 1.0f;
	Point lp = transform( Point( radius * u , 0.0f , radius * v ) );
	Vector normal = transform( Vector( 0 , 1 , 0 ) );
	Vector delta = lp - p;
	wi = Normalize( delta );

	float dot = Dot( -wi , normal );
	if( pdf ) 
	{
		if( dot <= 0 )
			*pdf = 0.0f;
		else
			*pdf = delta.SquaredLength() / ( SurfaceArea() * dot );
	}

	return lp;
}

// sample a ray from light
void Square::sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const
{
	float u = 2 * ls.u - 1.0f;
	float v = 2 * ls.v - 1.0f;
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Ori = transform( Point( radius * u , 0.0f , radius * v ) );
	r.m_Dir = transform( UniformSampleHemisphere( sort_canonical() , sort_canonical() ) );
	n = transform.invMatrix.Transpose()( Vector( 0.0f , 1.0f , 0.0f ) );

	if( pdf ) *pdf = 1.0f / ( radius * radius * 8.0f * PI );
}

// the surface area of the shape
float Square::SurfaceArea() const
{
	return radius * radius * 4.0f;
}

// get intersected point between the ray and the shape
float Square::_getIntersect( const Ray& ray , Point& p , float limit , Intersection* intersect ) const
{
	if( ray.m_Dir.y == 0.0f )
		return -1.0f;

	float t = -ray.m_Ori.y / ray.m_Dir.y;
	if( t > limit || t <= ray.m_fMin || t > ray.m_fMax )
		return -1.0f;
	p = ray(t);

	if( p.x > radius || p.x < -radius )
		return -1.0f;
	if( p.z > radius || p.z < -radius )
		return -1.0f;

	if( intersect )
	{
		intersect->t = t;
		intersect->intersect = transform( p );
		intersect->normal = transform.invMatrix.Transpose()(Vector( 0.0f , 1.0f , 0.0f ));
		intersect->tangent = transform(Vector( 0.0f , 0.0f , 1.0f ));
		intersect->primitive = const_cast<Square*>(this);
	}

	return t;
}

// get the bounding box of the primitive
const BBox&	Square::GetBBox() const
{
	if( !m_bbox )
	{
		m_bbox = new BBox();
		m_bbox->Union( transform( Point( radius , 0.0f , radius ) ) );
		m_bbox->Union( transform( Point( radius , 0.0f , -radius ) ) );
		m_bbox->Union( transform( Point( -radius , 0.0f , radius ) ) );
		m_bbox->Union( transform( Point( -radius , 0.0f , -radius ) ) );
	}

	return *m_bbox;
}