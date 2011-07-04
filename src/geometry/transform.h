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
#include "ray.h"
#include <math.h>

// pre-declera functions
Point	operator* ( const Transform& t , const Point& p );
Vector	operator* ( const Transform& t , const Vector& v );
Ray		operator* ( const Transform& t , const Ray& r );

////////////////////////////////////////////////////////////////////////////
//	definition of transform
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

	// the operator for transformation
	Point	operator()( const Point& p ) const { return this->matrix * p; }
	Vector	operator()( const Vector& v ) const { return *this * v; }
	Ray		operator()( const Ray& r ) const { return *this * r; }

	// * operator for transform
	Transform operator*( const Transform& t ) const
	{
		Matrix m = matrix * t.matrix;
		Matrix invm = t.invMatrix * invMatrix;

		return Transform( m , invm );
	}

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

inline Transform Translate( float x , float y , float z )
{
	Matrix m( 	1.0f , 0.0f , 0.0f , x ,
				0.0f , 1.0f , 0.0f , y ,
				0.0f , 0.0f , 1.0f , z ,
				0.0f , 0.0f , 0.0f , 1.0f );
	Matrix invm( 1.0f , 0.0f , 0.0f , -x ,
				 0.0f , 1.0f , 0.0f , -y ,
				 0.0f , 0.0f , 1.0f , -z ,
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
inline Transform Scale( float s )
{
	return Scale( s , s , s );
}

// rotate around x axis
inline Transform RotateX( float angle )
{
	float sin_t = sin( angle );
	float cos_t = cos( angle );
	Matrix m( 1.0f , 0.0f , 0.0f , 0.0f ,
			  0.0f , cos_t , -sin_t , 0.0f ,
			  0.0f , sin_t , cos_t , 0.0f ,
			  0.0f , 0.0f , 0.0f , 1.0f );

	return Transform( m , m.Transpose() );
}

// rotate around y axis
inline Transform RotateY( float angle )
{
	float sin_t = sin( angle );
	float cos_t = cos( angle );
	Matrix m( cos_t , 0.0f , sin_t , 0.0f ,
			  0.0f , 1.0f , 0.0f , 0.0f ,
			  -sin_t , 0.0f , cos_t , 0.0f ,
			  0.0f , 0.0f , 0.0f , 1.0f );
	return Transform( m , m.Transpose() );
}

// rotate around z axis
inline Transform RotateZ( float angle )
{
	float sin_t = sin( angle );
	float cos_t = cos( angle );
	Matrix m( cos_t , -sin_t , 0.0f , 0.0f ,
			  sin_t , cos_t , 0.0f , 0.0f ,
			  0.0f , 0.0f , 1.0f , 0.0f ,
			  0.0f , 0.0f , 0.0f , 1.0f );
	return Transform( m , m.Transpose() );
}

// return the inverse of the transform
inline Transform Inverse( const Transform& t )
{
	return Transform( t.invMatrix , t.matrix );
}

// transform a point
inline Point operator* ( const Transform& t , const Point& p )
{
	return t.matrix * p;
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
		return Vector( _x , _y , _z , true );
	}
	
	return t.matrix * v;
}

// transform a ray
inline Ray	operator* ( const Transform& t , const Ray& r )
{
	return Ray( t(r.m_Ori) , t(r.m_Dir) );
}

#endif
