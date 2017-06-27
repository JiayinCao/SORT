/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2017 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
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
