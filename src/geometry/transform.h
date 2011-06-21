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
#include "point.h"
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

	// check whether the transform is identity
	bool	IdIdentity() const;
	// whether there is scale factor in the matrix
	bool	HasScale() const;

// public field
public:
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

// return the inverse of the transform
inline Transform Inverse( const Transform& t )
{
	return Transform( t.invMatrix , t.matrix );
}

// transform a point
inline Point operator* ( const Transform& t , const Point& p )
{
	float x = p.x * t.matrix.m[0] + p.y * t.matrix.m[1] + p.z * t.matrix.m[2] + t.matrix.m[3];
	float y = p.x * t.matrix.m[4] + p.y * t.matrix.m[5] + p.z * t.matrix.m[6] + t.matrix.m[7];
	float z = p.x * t.matrix.m[8] + p.y * t.matrix.m[9] + p.z * t.matrix.m[10] + t.matrix.m[11];
	float w = p.x * t.matrix.m[12] + p.y * t.matrix.m[13] + p.z * t.matrix.m[14] + t.matrix.m[15];

	// if w is one , just return the point
	// note it is very common that w is one
	if( w == 1.0f )
		return Point( x , y , z );

	return Point( x , y , z ) / w;
}

// transform a vector
// note : the vector could be a normal , which requires special care about the multiplication
inline Vector operator* ( const Transform& t , const Vector& v )
{
	// if it's a normal , use the transpose of inverse matrix
	if( v.m_bNormal )
	{
		float _x = v.x * t.invMatrix.m[0] + v.y * t.invMatrix.m[4] + v.z * t.invMatrix.m[8];
		float _y = v.x * t.invMatrix.m[1] + v.y * t.invMatrix.m[5] + v.z * t.invMatrix.m[9];
		float _z = v.x * t.invMatrix.m[2] + v.y * t.invMatrix.m[6] + v.z * t.invMatrix.m[10];

		// return the result
		return Vector( _x , _y , _z );
	}

	float _x = v.x * t.matrix.m[0] + v.y * t.matrix.m[1] + v.z * t.matrix.m[2];
	float _y = v.x * t.matrix.m[4] + v.y * t.matrix.m[5] + v.z * t.matrix.m[6];
	float _z = v.x * t.matrix.m[8] + v.y * t.matrix.m[9] + v.z * t.matrix.m[10];

	// return the result
	return Vector( _x , _y , _z );
}

#endif
