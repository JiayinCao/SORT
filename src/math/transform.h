/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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

#include "matrix.h"
#include "math/vector3.h"
#include "point.h"
#include "math/ray.h"
#include <math.h>

class Transform;

// pre-declera functions
Ray     operator* ( const Transform& t , const Ray& r );

////////////////////////////////////////////////////////////////////////////
//  definition of transform
class   Transform{
public:
    // Default constructor
    Transform(){}
    // constructor from two matrix
    Transform( const Matrix& m , const Matrix& invm ): matrix(m) , invMatrix( invm ){}

    // check whether the transform is identity
    bool    IdIdentity() const;
    // whether there is scale factor in the matrix
    bool    HasScale() const;

    // the operator for transformation
    //Point   operator()( const Point& p ) const { return this->matrix.TransformPoint(p); }
    //Vector  operator()( const Vector& v ) const { return *this * v; }
    Point   TransformPoint( const Point& p ) const{
        return matrix.TransformPoint(p);
    }
    Vector  TransformVector( const Vector& v ) const{
        return matrix.TransformVector(v);
    }
    Vector  TransformNormal( const Vector& n ) const{
        return invMatrix.Transpose().TransformVector(n);
    }

    Ray     operator()( const Ray& r ) const {
        return *this * r;
    }

    // Get the inversed transform
    Transform GetInversed() const { return Transform( invMatrix , matrix ); }

    // * operator for transform
    Transform operator*( const Transform& t ) const
    {
        Matrix m = matrix * t.matrix;
        Matrix invm = t.invMatrix * invMatrix;

        return Transform( m , invm );
    }

public:
    // the matrix for transformation
    Matrix  matrix;
    // the inverse of the original matrix
    Matrix  invMatrix;
};

// from matrix
SORT_FORCEINLINE Transform FromMatrix( const Matrix& m )
{
    // get inverse matrix
    Matrix invm;
    m.Inverse(invm);
    return Transform( m , invm );
}

// translate
SORT_FORCEINLINE Transform Translate( const Vector& v )
{
    Matrix m(   1.0f , 0.0f , 0.0f , v.x ,
                0.0f , 1.0f , 0.0f , v.y ,
                0.0f , 0.0f , 1.0f , v.z ,
                0.0f , 0.0f , 0.0f , 1.0f );
    Matrix invm( 1.0f , 0.0f , 0.0f , -v.x ,
                 0.0f , 1.0f , 0.0f , -v.y ,
                 0.0f , 0.0f , 1.0f , -v.z ,
                 0.0f , 0.0f , 0.0f , 1.0f );
    return Transform( m , invm );
}

SORT_FORCEINLINE Transform Translate( float x , float y , float z )
{
    Matrix m(   1.0f , 0.0f , 0.0f , x ,
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
SORT_FORCEINLINE Transform Scale( float x , float y , float z )
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
SORT_FORCEINLINE Transform Scale( float s )
{
    return Scale( s , s , s );
}

// rotate around x axis
SORT_FORCEINLINE Transform RotateX( float angle )
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
SORT_FORCEINLINE Transform RotateY( float angle )
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
SORT_FORCEINLINE Transform RotateZ( float angle )
{
    float sin_t = sin( angle );
    float cos_t = cos( angle );
    Matrix m( cos_t , -sin_t , 0.0f , 0.0f ,
              sin_t , cos_t , 0.0f , 0.0f ,
              0.0f , 0.0f , 1.0f , 0.0f ,
              0.0f , 0.0f , 0.0f , 1.0f );
    return Transform( m , m.Transpose() );
}

SORT_FORCEINLINE Transform ViewLookat( const Point& eye , const Vector& dir , const Vector& up )
{
    const Vector right = cross( up , dir );
    const Vector adjusted_up = cross( dir , right );

    const float x = right.x * eye.x + right.y * eye.y + right.z * eye.z;
    const float y = adjusted_up.x * eye.x + adjusted_up.y * eye.y + adjusted_up.z * eye.z;
    const float z = dir.x * eye.x + dir.y * eye.y + dir.z * eye.z;

    const Matrix m( right.x , right.y , right.z , -x ,
                    adjusted_up.x , adjusted_up.y , adjusted_up.z , -y ,
                    dir.x , dir.y , dir.z , -z ,
                    0.0f , 0.0f , 0.0f , 1.0f );

    return FromMatrix(m);
}

// perspective matrix
SORT_FORCEINLINE Transform Perspective( float scaleX , float scaleY )
{
    // the following values don't matter much in a ray tracer because there is no Z-buffer at all.
    const float n = 0.01f;
    const float f = 1000.0f;
    Matrix m( scaleX , 0.0f , 0.0f , 0.0f ,
              0.0f , scaleY , 0.0f , 0.0f ,
              0.0f , 0.0f , f/(f-n) , -f*n/(f-n),
              0.0f , 0.0f , 1.0f , 0.0f );
    return FromMatrix(m);
}

// return the inverse of the transform
SORT_FORCEINLINE Transform Inverse( const Transform& t )
{
    return Transform( t.invMatrix , t.matrix );
}

// return the transpose of the transform
SORT_FORCEINLINE Transform Transpose( const Transform& t )
{
    return Transform( t.matrix.Transpose() , t.invMatrix.Transpose() );
}

// transform a ray
SORT_FORCEINLINE Ray  operator* ( const Transform& t , const Ray& r ){
    return Ray( t.TransformPoint(r.m_Ori) , t.TransformVector(r.m_Dir) , r.m_Depth , r.m_fMin , r.m_fMax );
}
