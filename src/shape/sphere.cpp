/*
   FileName:      sphere.cpp

   Created Time:  2011-08-19 22:02:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header
#include "sphere.h"
#include "geometry/ray.h"
#include "geometry/intersection.h"
#include "geometry/vector.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

// sample a point on shape
Point Sphere::sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf ) const
{
	Point center = transform( Point( 0.0f , 0.0f , 0.0f ) );
	Vector delta = center - p;
	Vector dir = Normalize( delta );
	Vector wcx , wcy;
	CoordinateSystem( dir , wcx , wcy );

	Matrix m( 	wcx.x , dir.x , wcy.x , 0.0f ,
				wcx.y , dir.y , wcy.y , 0.0f ,
				wcx.z , dir.z , wcy.z , 0.0f ,
				0.0f , 0.0f , 0.0f , 0.0f );

	float sq_sin_theta = radius * radius / delta.SquaredLength();
	float cos_theta = sqrt( max( 0.0f , 1.0f - sq_sin_theta ) );

	wi = UniformSampleCone( ls.u , ls.v , cos_theta );
	wi = m(wi);

	if( pdf ) *pdf = UniformConePdf( cos_theta );

	Point _p;
	Ray r = transform.invMatrix( Ray( p , wi ) );
	if( _getIntersect( r , FLT_MAX , _p ) < 0.0f )
		_p = r( Dot( delta , wi ) );
	
	return transform(_p);
}

// the surface area of the shape
float Sphere::SurfaceArea() const
{
	return 4 * PI * radius * radius ;
}

// get intersection between the light surface and the ray
bool Sphere::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	Sort_Assert( intersect != 0 );

	Point p;
	float t = _getIntersect( r , intersect->t , p );
	if( t > 0.0f )
	{
		intersect->t = t;
		intersect->intersect = p;
		intersect->normal = Normalize(Vector( p.x , p.y , p.z , true ));
		return true;
	}

	return false;
}

// get intersection between a ray and the sphere
float Sphere::_getIntersect( const Ray& r , float limit , Point& p ) const
{
	float _b = 2.0f * ( r.m_Dir.x * r.m_Ori.x + r.m_Dir.y * r.m_Ori.y + r.m_Dir.z * r.m_Ori.z );
	float _c = r.m_Ori.x * r.m_Ori.x + r.m_Ori.y * r.m_Ori.y + r.m_Ori.z * r.m_Ori.z - radius * radius;

	float delta = _b * _b - 4.0f * _c ;
	if( delta < 0.0f )
		return false;
	delta = sqrt( delta );

	float min_t = ( -_b - delta ) * 0.5f;
	float max_t = ( -_b + delta ) * 0.5f;

	if( min_t > limit || max_t <= 0.0f )
		return false;

	float t;
	if( min_t > 0.0f )
		t = min_t;
	else if( max_t > limit )
		return -1.0f;
	else
		t = max_t;

	p = r(t);

	return t;
}
