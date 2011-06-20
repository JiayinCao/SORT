/*
 * filename :	matrix.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "matrix.h"

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
	
