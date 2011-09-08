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
#include "utility/rand.h"

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
	Ray r = transform.invMatrix(Ray( p , wi ));
	if( _getIntersect( r , _p , FLT_MAX ) < 0.0f )
		_p = r( Dot( delta , wi ) );
	
	return transform(_p);
}

// get pdf of specific direction
float Sphere::Pdf( const Point& p ,  const Vector& wi ) const
{
	Point center;
	float sin_theta_sq = radius * radius / ( p - center ).SquaredLength();
	float cos_theta = sqrt( max( 0.0f , 1.0f - sin_theta_sq ) );
	return UniformConePdf( cos_theta );
}

// the surface area of the shape
float Sphere::SurfaceArea() const
{
	return 4 * PI * radius * radius ;
}

// get intersection between a ray and the sphere
float Sphere::_getIntersect( const Ray& r , Point& p , float limit , Intersection* intersect ) const
{
	float _b = 2.0f * ( r.m_Dir.x * r.m_Ori.x + r.m_Dir.y * r.m_Ori.y + r.m_Dir.z * r.m_Ori.z );
	float _c = r.m_Ori.x * r.m_Ori.x + r.m_Ori.y * r.m_Ori.y + r.m_Ori.z * r.m_Ori.z - radius * radius;

	float delta = _b * _b - 4.0f * _c ;
	if( delta < 0.0f )
		return -1.0f;
	delta = sqrt( delta );

	float min_t = ( -_b - delta ) * 0.5f;
	float max_t = ( -_b + delta ) * 0.5f;

	if( min_t > limit || max_t <= 0.0f )
		return -1.0f;

	float t;
	if( min_t > 0.0f )
		t = min_t;
	else if( max_t > limit )
		return -1.0f;
	else
		t = max_t;

	if( t > r.m_fMax || t < r.m_fMin )
		return -1.0f;

	p = r(t);

	if( intersect )
	{
		intersect->t = t;
		Vector n = Normalize(Vector( p.x , p.y , p.z , true ));
		Vector v0 , v1;
		CoordinateSystem( n , v0 , v1 );
		intersect->intersect = transform(p);
		intersect->normal = transform(n);
		intersect->tangent = transform(v0);
		intersect->primitive = const_cast<Sphere*>(this);
	}

	return t;
}

// sample a ray from light
void Sphere::sample_l( const LightSample& ls , Ray& r , float* pdf ) const
{
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Ori = radius * UniformSampleSphere( ls.u , ls.v );
	r.m_Dir = UniformSampleSphere( sort_canonical() , sort_canonical() );
	if( Dot( r.m_Dir , Vector( r.m_Ori.x , r.m_Ori.y , r.m_Ori.z ) ) < 0.0f )
		r.m_Dir = -r.m_Dir;

	if( pdf ) *pdf = 1.0f / ( 8.0f * PI * PI );
}

// get the bounding box of the primitive
const BBox&	Sphere::GetBBox() const
{
	Point center = transform( Point( 0.0f , 0.0f , 0.0f ) );

	if( !m_bbox )
	{
		m_bbox = new BBox();
		Vector vec_r = Vector( radius , radius , radius );
		m_bbox->m_Min = center - vec_r ;
		m_bbox->m_Max = center + vec_r ;
	}

	return *m_bbox;
}