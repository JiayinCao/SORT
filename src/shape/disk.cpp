/*
   FileName:      disk.cpp

   Created Time:  2011-08-19 22:08:45

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "disk.h"
#include "utility/samplemethod.h"
#include "sampler/sample.h"
#include "geometry/vector.h"

// sample a point on shape
Point Disk::sample_l( const LightSample& ls , const Point& p , Vector& wi , float* pdf , Vector& normal ) const
{
	float u , v;
	UniformSampleDisk( ls.u , ls.v , u , v );

	Point axis0 ( 1 , 0 , 0 );
	Point axis1 ( 0 , 0 , 1 );

	Point lp = transform( ( axis0 * u + axis1 * v ) * radius );
	normal = transform( Vector( 0 , 1 , 0 ) );
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

// the surface area of the shape
float Disk::SurfaceArea() const
{
	return PI * radius * radius;
}
