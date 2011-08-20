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

// sample a point on shape
Point Sphere::sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf ) const
{
	return Point();
}

// the surface area of the shape
float Sphere::SurfaceArea() const
{
	return 4 * PI * radius * radius ;
}

// get intersection between the light surface and the ray
bool Sphere::GetIntersect( const Ray& ray , Intersection* intersect ) const
{
	return false;
}