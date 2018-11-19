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

#include "geometry/ray.h"

////////////////////////////////////////////////////////////////////
//	definition of matrix
class	Matrix
{
public:
	// default constructor , initialize a identity matrix
	Matrix();
	// constructor from a float array containing 16 floats
	// para 'data' : 16 float for the matrix
	Matrix( const float data[] );
	// constructor from 16 float
	Matrix( float _11 , float _12 , float _13 , float _14 ,
			float _21 , float _22 , float _23 , float _24 ,
			float _31 , float _32 , float _33 , float _34 ,
			float _41 , float _42 , float _43 , float _44 );

	// matrix multiplication
	// para 'm' : the matrix to multiply
	// result : the composition of the two matrix
	Matrix operator * ( const Matrix& m ) const;

	// transform a point
	// para 'p' : the point to transform
	// result   : the transformed point
	Point operator * ( const Point& p ) const;
	Point operator () ( const Point& p ) const { return *this * p; }

	// transform a vector
	// para 'v' : the vector to transform
	// result   : transformed vector
	// note     : a matrix transformation applied to a normal is invalid
	Vector operator * ( const Vector& v ) const;
	Vector operator () ( const Vector& v ) const { return *this * v; }

	// transform a ray
	// para 'r' : the ray to transform
	// result   : transformed ray
	Ray operator * ( const Ray& r ) const
	{
		return Ray( *this * r.m_Ori , *this * r.m_Dir , r.m_Depth , r.m_fMin , r.m_fMax );
	}
	Ray operator () ( const Ray& r ) const
	{
		return *this * r;
	}

	// transpose the matrix
	Matrix Transpose() const;
	// determinant of the matrix
	float	Determinant() const;
	// inverse of the matrix
	bool Inverse(Matrix& out) const;

	// whether the matrix have scale factor
	bool	HasScale() const;

public:
	// the data of the 4x4 matrix
	// m[0]  m[1]  m[2]  m[3]
	// m[4]  m[5]  m[6]  m[7]
	// m[8]  m[9]  m[10] m[11]
	// m[12] m[13] m[14] m[15]
	float	m[16];
};
