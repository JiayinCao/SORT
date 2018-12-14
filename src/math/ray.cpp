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

// include the header file
#include "ray.h"

// default constructor
Ray::Ray()
{
	m_Depth = 0;
	m_fMin = 0.0f;
	m_fMax = FLT_MAX;
	m_fPdfW = 0.0f;
	m_fPdfA = 0.0f;
	m_we = 0.0f;
	m_fCosAtCamera = 0.0f;
}
// constructor from a point and a direction
Ray::Ray( const Point& p , const Vector& dir , unsigned depth , float fmin , float fmax)
{
	m_Ori = p;
	m_Dir = dir;
	m_Depth = depth;
	m_fMin = fmin;
	m_fMax = fmax;
	m_fPdfW = 0.0f;
	m_fPdfA = 0.0f;
	m_we = 0.0f;
	m_fCosAtCamera = 0.0f;
}
// copy constructor
Ray::Ray( const Ray& r )
{
	m_Ori = r.m_Ori;
	m_Dir = r.m_Dir;
	m_Depth = r.m_Depth;
	m_fMin = r.m_fMin;
	m_fMax = r.m_fMax;
	m_fPdfW = r.m_fPdfW;
	m_fPdfA = r.m_fPdfA;
	m_we = r.m_we;
	m_fCosAtCamera = r.m_fCosAtCamera;
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
