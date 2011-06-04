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
	// para 'f' :	scaler
	// result   :	a scaled point
	Point operator * ( float f ) const;
	// para 'f' : 	scaler
	// result   :	current point after scaling
	Point& operator *= ( float f );
	// para 'f' :	divider
	// result   :	a divided point
	Point operator / ( float f ) const;
	// para 'f' :	divider
	// result   :	current vector after dividing
	Point& operator /= ( float f );

	// [] operator
	// result   :	component with the specific id
	float operator []( unsigned id ) const;
	// [] operator
	// result   :	component with the specific id
	float& operator []( unsigned id );

	// == operator
	// para 'p' :	a point to compare
	// result   :	'true' if the 'v' is the same with current vector , 'false' else
	bool operator == ( const Point& p ) const ;
	// != operator
	// para 'p' :	a point to compare
	// result   :	'true' if the 'v' is not the same with current vector , 'false' else
	bool operator != ( const Point& p ) const ;

//public field
public:
	float x , y , z;
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

#endif


