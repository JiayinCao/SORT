/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

// include the header file
#include "vector3.h"
#include "utility/sassert.h"

/////////////////////////////////////////////////////////////////////
// definition of point
class Point
{
	//public method
public:
	// default constructor
	Point()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	// constructor from three float data
	// para 'x' :	x component of the point
	// para 'y' :	y component of the point
	// para 'z' :	z component of the point
	Point( float _x , float _y , float _z )
	{
		x = _x;
		y = _y;
		z = _z;
	}
	// copy constructor
	// para 'p' :	a point to copy
	Point( const Point& p )
	{
		x = p.x;
		y = p.y;
		z = p.z;
	}
	// constructor from a vector
	Point( const Vector& v )
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	// some math operations
	// para 'v' :	vector to add
	// result   :	a point with the offset 'v' to the current point
	Point operator + ( const Vector& v ) const
	{
		return Point( x + v.x , y + v.y , z + v.z );
	}
	// para 'p' :	point to merge
	// result   :   a mixed point
	Point operator + ( const Point& v ) const
	{
		return Point( x + v.x , y + v.y , z + v.z );
	}
	// para 'v' :	vector to add
	// result   :   current point after the offset 'v' is applied
	Point& operator += ( const Vector& v )
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}
	// para 'v' :	vector to minus
	// result   :	a point with the offset '-v' to the current point
	Point operator - ( const Vector& v ) const
	{
		return Point( x - v.x , y - v.y , z - v.z );
	}

	// para 'v' :	vector to minus
	// result   :	current point after the offset '-v' is applied
	Point& operator -= ( const Vector& v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

	// para 'v' :	a point to calculate the offset
	// result   :	the offset between them
	Vector operator - ( const Point& p ) const
	{
		return Vector( x - p.x , y - p.y , z - p.z );
	}

	// para 'f' :	scaler
	// result   :	a scaled point
	Point operator * ( float f ) const
	{
		return Point( f * x , f * y , f * z );
	}

	// para 'f' : 	scaler
	// result   :	current point after scaling
	Point& operator *= ( float f )
	{
		x *= f;
		y *= f;
		z *= f;

		return *this;
	}
	// para 'f' :	divider
	// result   :	a divided point
	Point operator / ( float f ) const
	{
		sAssert( fabs(f) > 0.000001f , GENERAL );

		float t = 1.0f / f;

		return (*this) * t;
	}
	// para 'f' :	divider
	// result   :	current vector after dividing
	Point& operator /= ( float f )
	{
		sAssert( fabs(f) > 0.000001f , GENERAL );

		float t = 1.0f / f;

		x *= t;
		y *= t;
		z *= t;

		return *this;
	}
	// para 'p' :   point to copy
	Point& operator = ( const Point& p )
	{
		x = p.x;
		y = p.y;
		z = p.z;

		return *this;
	}


	// [] operator
	float operator [] ( unsigned id ) const
	{
		return data[id];
	}

	// [] operator
	float& operator[]( unsigned id )
	{
		return data[id];
	}

	// == operator
	// para 'p' :	a point to compare
	// result   :	'true' if the 'v' is the same with current vector , 'false' else
	bool operator == ( const Point& p ) const 
	{
		if( p.x == x && p.y == y && p.z == z )
			return true;
		return false;
	}

	// != operator
	// para 'p' :	a point to compare
	// result   :	'true' if the 'v' is not the same with current vector , 'false' else
	bool operator != ( const Point& p ) const 
	{
		if( p.x == x && p.y == y && p.z == z )
			return false;
		return true;
	}

    operator Vector3f() const
    {
        return Vector3f( x , y , z );
    }

	//public field
public:
	union
	{
		struct
		{
			float x , y , z;
		};
		float data[3];
	};
};

// some global math operations
// para 'v' :	vector to add
// result   :	a point with the offset 'v' to the current point
inline Point operator + ( const Vector& v , const Point& p )
{
	return p + v;
}
// para 'f' :	scaler
// result   :	a scaled point
inline Point operator * ( float f , const Point& p )
{
	return p * f;
}
// para 'p0' :	a point
// para 'p1' :	another point
// result    :	the squared distance between the two points
inline float SquaredDistance( const Point& p0 , const Point& p1 )
{
	return (p0-p1).SquaredLength();
}
// para 'p0' :	a point
// para 'p1' :	another point
// result    :	the distance between the two points
inline float Distance( const Point& p0 , const Point& p1 )
{
	return (p0-p1).Length();
}
