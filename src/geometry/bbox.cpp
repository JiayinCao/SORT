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
#include "bbox.h"
#include "math/vector3.h"
#include "utility/sassert.h"

// default constructor
BBox::BBox():
m_Min( FLT_MAX , FLT_MAX , FLT_MAX ),
m_Max( -FLT_MAX , -FLT_MAX , -FLT_MAX )
{
}

// constructor from two points
BBox::BBox( const Point& p0 , const Point& p1 , bool sorted )
{
	if( sorted )
	{
		m_Min = p0;
		m_Max = p1;
		return;
	}

	if( p0.x < p1.x )
	{
		m_Min.x = p0.x;
		m_Max.x = p1.x;
	}else
	{
		m_Min.x = p1.x;
		m_Max.x = p0.x;
	}

	if( p0.y < p1.y )
	{
		m_Min.y = p0.y;
		m_Max.y = p1.y;
	}else
	{
		m_Min.y = p1.y;
		m_Max.y = p0.y;
	}

	if( p0.z < p1.z )
	{
		m_Min.z = p0.z;
		m_Max.z = p1.z;
	}else
	{
		m_Min.z = p1.z;
		m_Max.z = p0.z;
	}
}

// copy constructor
BBox::BBox( const BBox& bbox )
{
	m_Min = bbox.m_Min;
	m_Max = bbox.m_Max;
}

// check whether a point is in the current bounding box
bool BBox::IsInBBox( const Point& p , float delta ) const
{
	if( p.x > m_Max.x + delta || p.x < m_Min.x - delta )
		return false;
	if( p.y > m_Max.y + delta || p.y < m_Min.y - delta )
		return false;
	if( p.z > m_Max.z + delta || p.z < m_Min.z - delta )
		return false;

	return true;
}

// set the bounding box as invalid
void BBox::InvalidBBox()
{
	m_Min = Point( FLT_MAX , FLT_MAX , FLT_MAX );
	m_Max = Point( -FLT_MAX , -FLT_MAX , -FLT_MAX );
}

// get the surface area of the bounding box
float BBox::SurfaceArea() const
{
	return 2.0f * HalfSurfaceArea();
}

// half surface area
float BBox::HalfSurfaceArea() const
{
	Vector offset = m_Max - m_Min;

	return offset.x * offset.y + offset.y * offset.z + offset.z * offset.x;
}

// get the volumn of the bounding box
float BBox::Volumn() const
{
	Vector offset = m_Max - m_Min;

	return offset.x * offset.y * offset.z;
}

// get the longest axis
unsigned BBox::MaxAxisId() const
{
	Vector offset = m_Max - m_Min;

	if( offset.x > offset.y && offset.x > offset.z )
		return 0;
	if( offset.y > offset.x && offset.y > offset.z )
		return 1;

	return 2;
}

// union the bounding box
void BBox::Union( const Point& p )
{
	for( unsigned i = 0 ; i < 3 ; i++ )
	{
		if( p[i] < m_Min[i] )
			m_Min[i] = p[i];
		if( p[i] > m_Max[i] )
			m_Max[i] = p[i];
	}
}

// union the bounding box
void BBox::Union( const BBox& box )
{
	for( int i = 0 ; i < 3 ; i++ )
	{
		m_Min[i] = std::min( m_Min[i] , box.m_Min[i] );
		m_Max[i] = std::max( m_Max[i] , box.m_Max[i] );
	}
}

// delta in a specific axis
float BBox::Delta( unsigned k ) const
{
	sAssert( k < 3 , GENERAL );
	return m_Max[k]-m_Min[k];
}
