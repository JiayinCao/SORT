/*
 * filename :	ray.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "ray.h"

// default constructor
Ray::Ray()
{
}
// constructor from a point and a direction
Ray::Ray( const Point& p , const Vector& dir , unsigned depth , float fmin , float fmax)
{
	m_Ori = p;
	m_Dir = dir;
	m_Depth = depth;
	m_fMin = fmin;
	m_fMax = fmax;
}
// copy constructor
Ray::Ray( const Ray& r )
{
	m_Ori = r.m_Ori;
	m_Dir = r.m_Dir;
	m_Depth = r.m_Depth;
	m_fMin = r.m_fMin;
	m_fMax = r.m_fMax;
}
// destructor
Ray::~Ray()
{
}

// operator to get a point on the ray
Point Ray::operator ()( float t ) const
{
	return m_Ori + t * m_Dir;
}
