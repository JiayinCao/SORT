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
	Point lp = transform.invMatrix( p );
	Vector lwi = transform.invMatrix( wi );

	Point intersect;
	Intersection inter;
	if( _getIntersect( Ray( lp , lwi ) , intersect , FLT_MAX , &inter ) < 0.0f )
		return 0.0f;

	Vector delta = inter.intersect - p;
	float dot = AbsDot( Normalize(delta) , inter.normal );
	return delta.SquaredLength() / ( SurfaceArea() * dot );
}

// get intersection between the light surface and the ray
bool Shape::GetIntersect( const Ray& ray , Intersection* intersect ) const
{
	Ray local = transform.invMatrix( ray );
	Point p;
	if( intersect )
		return _getIntersect( local , p , intersect->t , intersect ) > 0.0f;
	else
		return _getIntersect( local , p , FLT_MAX ) > 0.0f;

	return true;
}