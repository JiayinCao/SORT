/*
   FileName:      matrix.cpp

   Created Time:  2011-08-04 12:49:33

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "matrix.h"
#include "point.h"
#include "vector.h"
#include "managers/logmanager.h"

// default constructor
Matrix::Matrix()
{
	m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
	m[4] = 0.0f; m[5] = 1.0f; m[6] = 0.0f; m[7] = 0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10] = 1.0f;m[11] = 0.0f;
	m[12] = 0.0f;m[13]= 0.0f; m[14] = 0.0f;m[15] = 1.0f;
}

// constructor from 16 float
Matrix::Matrix( const float data[16] )
{
	for( int i = 0 ; i < 16 ; i++ )
		m[i] = data[i];
}

// constructrom from 16 float
Matrix::Matrix( float _11 , float _12 , float _13 , float _14 ,
				float _21 , float _22 , float _23 , float _24 ,
				float _31 , float _32 , float _33 , float _34 ,
				float _41 , float _42 , float _43 , float _44 )
{
	m[0] = _11;  m[1] = _12;  m[2] = _13;  m[3] = _14;
	m[4] = _21;  m[5] = _22;  m[6] = _23;  m[7] = _24;
	m[8] = _31;  m[9] = _32;  m[10] = _33; m[11] = _34;
	m[12] = _41; m[13] = _42; m[14] = _43; m[15] = _44;
}

// matrix mutiplication
Matrix Matrix::operator *( const Matrix& mat) const
{
	float data[16];
	for( int i = 0 ; i < 4 ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			int offset = i * 4 + j;
			data[offset] = 0.0f;

			for( int k = 0 ; k < 4 ; k++ )
				data[offset] += m[i*4+k]*mat.m[k*4+j];
		}
	}

	return Matrix(data);
}

// transform point
Point Matrix::operator *( const Point& p ) const
{
	float x = p.x * m[0] + p.y * m[1] + p.z * m[2] + m[3];
	float y = p.x * m[4] + p.y * m[5] + p.z * m[6] + m[7];
	float z = p.x * m[8] + p.y * m[9] + p.z * m[10] + m[11];
	float w = p.x * m[12] + p.y * m[13] + p.z * m[14] + m[15];

	// if w is one , just return the point
	// note it is very common that w is one
	if( w == 1.0f )
		return Point( x , y , z );

	return Point( x , y , z ) / w;
}

// transform vector
Vector Matrix::operator *( const Vector& v ) const
{
	if( v.m_bNormal )
		LOG_ERROR<<"Matrix applied to normal."<<CRASH;

	float _x = v.x * m[0] + v.y * m[1] + v.z * m[2];
	float _y = v.x * m[4] + v.y * m[5] + v.z * m[6];
	float _z = v.x * m[8] + v.y * m[9] + v.z * m[10];

	// return the result
	return Vector( _x , _y , _z );
}

// create a transpose matrix
Matrix Matrix::Transpose() const
{
	return Matrix( 	m[0] , m[4] , m[8] , m[12] , 
					m[1] , m[5] , m[9] , m[13] ,
					m[2] , m[6] , m[10] , m[14] ,
					m[3] , m[7] , m[11] , m[15] );
}

// whether the matrix have scale factor
bool Matrix::HasScale() const
{
	float l0 = Vector( m[0] , m[4] , m[8] ).Length();
	float l1 = Vector( m[1] , m[5] , m[9] ).Length();
	float l2 = Vector( m[2] , m[6] , m[7] ).Length();

#define	IS_ONE(x) ((x)>0.999f && (x)<1.001f )
	return !( IS_ONE(l0) && IS_ONE(l1) && IS_ONE(l2) );
#undef IS_ONE
}