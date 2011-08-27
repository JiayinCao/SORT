/*
   FileName:      shape.cpp

   Created Time:  2011-08-27 13:36:04

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "shape.h"
#include "geometry/intersection.h"

// get the pdf of specific direction
float Shape::Pdf( const Point& p , const Vector& wi ) const
{
	Point intersect;
	if( _getIntersect( Ray( p , wi ) , intersect ) < 0.0f )
		return 0.0f;

	Vector delta = Normalize(intersect - p);
	
	float dot = Dot( delta , Vector( 0.0f , 1.0f , 0.0f ) );
	if( dot <= 0.0f )
		return 0.0f;

	return delta.SquaredLength() / ( SurfaceArea() * dot );
}

// get intersection between the light surface and the ray
bool Shape::GetIntersect( const Ray& ray , Intersection* intersect ) const
{
	float t;
	if( intersect )
	{
		t = _getIntersect( ray , intersect->intersect , intersect->t );
		
		if( t > 0.0f )
		{
			intersect->t = t;
			intersect->intersect = ( ray(intersect->t) );
			intersect->normal = Vector( 0.0f , 1.0f , 0.0f , true );
			return true;
		}else
			return false;
	}else
	{
		Point p;
		return _getIntersect( ray , p , FLT_MAX ) > 0.0f;
	}

	return true;
}