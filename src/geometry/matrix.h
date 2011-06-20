/*
 * filename :	matrix.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MATRIX
#define	SORT_MATRIX

class Point;
class Vector;

////////////////////////////////////////////////////////////////////
//	defination of matrix
class	Matrix
{
// public method
public:
	// default constructor , initialize a identity matrix
	Matrix();
	// constructor from a float array containing 16 floats
	// para 'data' : 16 float for the matrix
	Matrix( const float data[] );
	// constructrom from 16 float
	Matrix( float _11 , float _12 , float _13 , float _14 ,
			float _21 , float _22 , float _23 , float _24 ,
			float _31 , float _32 , float _33 , float _34 ,
			float _41 , float _42 , float _43 , float _44 );
	// destructor
	~Matrix(){}

	// transform a point
	Point	operator ()( const Point& p ) const;
	// transform a vector
	Vector	operator ()( const Vector& v ) const;

// public field
public:
	// the data of the 4x4 matrix
	// m[0]  m[1]  m[2]  m[3]
	// m[4]  m[5]  m[6]  m[7]
	// m[8]  m[9]  m[10] m[11]
	// m[12] m[13] m[14] m[15]
	float	m[16];
};

// create a transpose matrix
inline Matrix Transpose( const Matrix& m )
{
	return Matrix( 	m.m[0] , m.m[4] , m.m[8] , m.m[12] , 
					m.m[1] , m.m[5] , m.m[9] , m.m[13] ,
					m.m[2] , m.m[6] , m.m[10] , m.m[14] ,
					m.m[3] , m.m[7] , m.m[11] , m.m[15] );
}

#endif
