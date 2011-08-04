/*
   FileName:      vector.cpp

   Created Time:  2011-08-04 12:51:20

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header files
#include "vector.h"
#include <math.h>
#include "managers/logmanager.h"
#include "transform.h"
#include "utility/assert.h"

// default constructor, all of the components are set zero
Vector::Vector( bool normal ):
m_bNormal( normal )
{
	x = 0;
	y = 0;
	z = 0;
}

// copy constructor
Vector::Vector( const Vector& vector ):
m_bNormal( vector.m_bNormal )
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

// constructor from three float data
Vector::Vector( float _x , float _y , float _z , bool normal ):
m_bNormal( normal )
{
	x = _x;
	y = _y;
	z = _z;
}

// destructor does nothing here
Vector::~Vector()
{
}

// + operator
Vector Vector::operator +( const Vector& v ) const
{
	return Vector( x + v.x , y + v.y , z + v.z , m_bNormal );
}

// += operator
Vector&	Vector::operator +=( const Vector& v )
{
	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

// - operator
Vector	Vector::operator -( const Vector& v ) const
{
	return Vector( x - v.x , y - v.y , z - v.z , m_bNormal );
}

// -= operator
Vector&	Vector::operator -=( const Vector& v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

// * operator with a float
Vector Vector::operator *( float f ) const
{
	return Vector( x * f , y * f , z * f , m_bNormal );
}

// *= operator with a float
Vector&	Vector::operator *=( float f )
{
	x *= f;
	y *= f;
	z *= f;

	return *this;
}

// / operator
Vector Vector::operator / ( float f ) const
{
	Sort_Assert( f != 0.0f );

	float t = 1.0f / f;
	
	return (*this) * t;
}

// /= operator
Vector& Vector::operator /= ( float f )
{
	Sort_Assert( f != 0.0f );

	float t = 1.0f / f;

	x *= t;
	y *= t;
	z *= t;

	return *this;
}

// [] operator
float Vector::operator [] ( unsigned id ) const
{
	if( id == 0 )
		return x;
	if( id == 1 )
		return y;
	if( id == 2 )
		return z;

	Sort_Assert( id < 3 );

	return 0;
}

// [] operator
float& Vector::operator [] ( unsigned id )
{
	if( id == 0 )
		return x;
	if( id == 1 )
		return y;
	Sort_Assert( id < 3 );

	return z;
}

// - operator
Vector Vector::operator - () const 
{
	return Vector( -x , -y , -z , m_bNormal );
}

// == operator
bool Vector::operator == ( const Vector& v ) const
{
	if( v.x == x && v.y == y && v.z == z )
		return true;
	return false;
}

// != operator
bool Vector::operator != ( const Vector& v ) const
{
	if( v.x != x && v.y != y && v.z != z )
		return false;
	return true;
}

// sqaured length
float Vector::SquaredLength() const 
{
	return x * x + y * y + z * z;
}

// length
float Vector::Length() const
{
	return sqrt( SquaredLength() );
}

// normalize the vector
Vector& Vector::Normalize()
{
	// get the length of the vector
	float len = Length();

	// divide the vector
	if( len == 0 )
		LOG_WARNING<<"Try to normalize a zero length vector!!"<<ENDL;
	
	if( len != 0 )
		*this /= len;

	return *this;
}

// para 'v' :	vector to copy
Vector& Vector::operator = ( const Vector& v )
{
	x = v.x;
	y = v.y;
	z = v.z;

	m_bNormal = v.m_bNormal;

	return *this;
}

// rotate a vector along the current vector
void Vector::Rotate( Vector& vec , float angle ) const
{
	float temp;
	float cos_ang = cos( angle );
	float sin_ang = sin( angle );

	Vector o = vec * cos_ang;

	temp = Dot( vec , *this );
	temp *= ( 1 - cos_ang );

	o += *this * temp;
	Vector cross = Cross( *this , vec );

	vec = o + cross * sin_ang;
}