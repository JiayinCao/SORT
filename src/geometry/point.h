/*
 * filename:	point.h
 *
 * programmer:	Cao Jiayin
 */

#ifndef SORT_POINT
#define	SORT_POINT

// include the header file
#include "vector.h"

/////////////////////////////////////////////////////////////////////
// defination of point
class Point
{
//public method
public:
	// default constructor
	Point();
	// constructor from three float data
	// para 'x' :	x component of the point
	// para 'y' :	y component of the point
	// para 'z' :	z component of the point
	Point( float _x , float _y , float _z );
	// copy constructor
	// para 'p' :	a point to copy
	Point( const Point& p );
	// destructor
	~Point();

	// some math operations
	// para 'v' :	vector to add
	// result   :	a point with the offset 'v' to the current point
	Point operator + ( const Vector& v ) const;
	// para 'v' :	vector to add
	// result   :   current point after the offset 'v' is applied
	Point& operator += ( const Vector& v );
	// para 'v' :	vector to minus
	// result   :	a point with the offset '-v' to the current point
	Point operator - ( const Vector& v ) const;
	// para 'v' :	vector to minus
	// result   :	current point after the offset '-v' is applied
	Point& operator -= ( const Vector& v );
	// para 'v' :	a point to calculate the offset
	// result   :	the offset between them
	Vector operator - ( const Point& p ) const;

	// [] operator
	// result   :	component with the specific id
	float operator []( unsigned id );

//public field
public:
	float x , y , z;
};

// some global math operations
// para 'v' :	vector to add
// result   :	a point with the offset 'v' to the current point
Point operator + ( const Vector& v , const Point& p );


#endif


