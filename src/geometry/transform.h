/*
 * filename :	transform.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_TRANSFORM
#define	SORT_TRANSFORM

// include the header
#include "matrix.h"
#include "vector.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
//	defination of transform
class	Transform
{
// public method
public:
	// Default constructor
	Transform(){}
	// constructor from two matrix
	Transform( const Matrix& m , const Matrix& invm ): matrix(m) , invMatrix( invm ){}
	// destructor
	~Transform(){}

// private field
private:
	// the matrix for tranformation
	Matrix	matrix;
	// the inverse of the original matrix
	Matrix	invMatrix;
};

// translate
inline Transform Translate( const Vector& v )
{
	Matrix m( 	1.0f , 0.0f , 0.0f , v.x ,
				0.0f , 1.0f , 0.0f , v.y ,
				0.0f , 0.0f , 1.0f , v.z ,
				0.0f , 0.0f , 0.0f , 1.0f );
	Matrix invm( 1.0f , 0.0f , 0.0f , -v.x ,
				 0.0f , 1.0f , 0.0f , -v.y ,
				 0.0f , 0.0f , 1.0f , -v.z ,
				 0.0f , 0.0f , 0.0f , 1.0f );
	return Transform( m , invm );
}

// scale
inline Transform Scale( float x , float y , float z )
{
	Matrix m( x , 0.0f , 0.0f , 0.0f ,
			  0.0f , y , 0.0f , 0.0f ,
			  0.0f , 0.0f , z , 0.0f ,
			  0.0f , 0.0f , 0.0f , 1.0f );
	Matrix invm( 1.0f / x , 0.0f , 0.0f , 0.0f ,
				 0.0f , 1.0f / y , 0.0f , 0.0f ,
				 0.0f , 0.0f , 1.0f / z , 0.0f ,
				 0.0f , 0.0f , 0.0f , 1.0f );
	return Transform( m , invm );
}

// rotate around x axis
inline Transform RotateX( float angle )
{
	float sin_t = sin( angle );
	float cos_t = cos( angle );
	Matrix m( 1.0f , 0.0f , 0.0f , 0.0f ,
			  0.0f , cos_t , -sin_t , 0.0f ,
			  0.0f , sin_t , -cos_t , 0.0f ,
			  0.0f , 0.0f , 0.0f , 1.0f );

	return Transform( m , Transpose(m) );
}

// rotate around y axis
inline Transform RotateY( float angle )
{
	float sin_t = sin( angle );
	float cos_t = cos( angle );
	Matrix m( cos_t , 0.0f , sin_t , 0.0f ,
			  0.0f , 1.0f , 0.0f , 0.0f ,
			  -sin_t , 0.0f , -cos_t , 0.0f ,
			  0.0f , 0.0f , 0.0f , 1.0f );
	return Transform( m , Transpose(m) );
}

// rotate around z axis
inline Transform RotateZ( float angle )
{
	float sin_t = sin( angle );
	float cos_t = cos( angle );
	Matrix m( cos_t , -sin_t , 0.0f , 0.0f ,
			  sin_t , -cos_t , 0.0f , 0.0f ,
			  0.0f , 0.0f , 1.0f , 0.0f ,
			  0.0f , 0.0f , 0.0f , 1.0f );
	return Transform( m , Transpose(m) );
}

#endif
