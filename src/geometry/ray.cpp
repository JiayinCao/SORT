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
Ray::Ray( const Point& p , const Vector& dir )
{
	m_Ori = p;
	m_Dir = dir;
}
// copy constructor
Ray::Ray( const Ray& r )
{
	m_Ori = r.m_Ori;
	m_Dir = r.m_Dir;
}
// destructor
Ray::~Ray()
{
}
