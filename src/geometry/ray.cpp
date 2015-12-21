/*
   FileName:      ray.cpp

   Created Time:  2011-08-04 12:50:09

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "ray.h"

// default constructor
Ray::Ray()
{
	m_Depth = 0;
	m_fMin = 0.0f;
	m_fMax = FLT_MAX;
	m_fPDF = 1.0f;
}
// constructor from a point and a direction
Ray::Ray( const Point& p , const Vector& dir , unsigned depth , float fmin , float fmax)
{
	m_Ori = p;
	m_Dir = dir;
	m_Depth = depth;
	m_fMin = fmin;
	m_fMax = fmax;
	m_fPDF = 1.0f;
}
// copy constructor
Ray::Ray( const Ray& r )
{
	m_Ori = r.m_Ori;
	m_Dir = r.m_Dir;
	m_Depth = r.m_Depth;
	m_fMin = r.m_fMin;
	m_fMax = r.m_fMax;
	m_fPDF = 1.0f;
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
