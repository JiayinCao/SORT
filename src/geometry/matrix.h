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
//	definition of matrix
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

	// matrix multiplication
	// para 'm' : the matrix to mutiply
	// result : the composition of the two matrix
	Matrix operator * ( const Matrix& m ) const;

	// transpose the matrix
	Matrix Transpose() const;

// public field
public:
	// the data of the 4x4 matrix
	// m[0]  m[1]  m[2]  m[3]
	// m[4]  m[5]  m[6]  m[7]
	// m[8]  m[9]  m[10] m[11]
	// m[12] m[13] m[14] m[15]
	float	m[16];
};

#endif
