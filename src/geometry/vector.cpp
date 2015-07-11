/*
   FileName:      vector.cpp

   Created Time:  2011-08-04 12:51:20

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header files
#include "vector.h"
#include "point.h"

// constructor from point
Vector::Vector( const Point& p )
{
	x = p.x;
	y = p.y;
	z = p.z;
}