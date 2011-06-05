/*
 * filename :	bbox.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_BBOX
#define SORT_BBOX

// include the header file
#include "point.h"

/////////////////////////////////////////////////////////////////////////////
// defination of axis aligned bounding box
class BBox
{
// public method
public:
	// default constructor
	BBox();
	// constructor from two point
	// para 'p0'     :	a point
	// para 'p1'     :	another point
	// para 'sorted' :	if sorted is true , it means p0 is minium point and p1 is maxium point
	BBox( const Point& p0 , const Point& p1 , bool sorted = true );
	// copy constructor
	// para 'bbox'   :	a bounding box to be copied
	BBox( const BBox& bbox );

	// check whether a specific point is in the bounding box
	// para 'p'     :	a point
	// para 'delta' :	enlarge the bounding box a little to avoid float format error
	// result       :	return true if 'p' is in the bounding box
	bool IsInBBox( const Point& p , float delta ) const;

	// get the surface area of the bounding box
	// result :	the surface area of the bounding box
	float SurfaceArea() const;

	// get the volumn of the bounding box
	// result :	the volumn of the bounding box
	float Volumn() const;

	// get the id of the axis with the longest extent
	// result :	the id of axis with the longest extent
	unsigned MaxAxisId() const;

// public data
public:
	// the minium and maxium point of the bounding box
	Point	m_Min;
	Point	m_Max;
};

// global functions
// para 'bbox' :	a bounding box to encapture
// para 'p'    :	a point to encapture
// result      :	a bounding box containg both 'bbox' and 'p'
inline BBox Union( const BBox& bbox , const Point& p )
{
	BBox result;

	for( unsigned i = 0 ; i < 3 ; i++ )
	{
		if( p[i] < bbox.m_Min[i] )
		{
			result.m_Min[i] = p[i];
			result.m_Max[i] = bbox.m_Max[i];
		}
		else if( p[i] > bbox.m_Max[i] )
		{
			result.m_Min[i] = bbox.m_Min[i];
			result.m_Max[i] = p[i];
		}else
		{
			result.m_Min[i] = bbox.m_Min[i];
			result.m_Max[i] = bbox.m_Max[i];
		}
	}
}
// para 'bbox0' :	first bounding box
// para 'bbox1' :	second bounding box
// result       :	a bounding box containing both of 'bbox0' and 'bbox1'
inline BBox Union( const BBox& bbox0 , const BBox& bbox1 )
{
	BBox result;

	for( int i = 0 ; i < 3 ; i++ )
	{
		if( bbox0.m_Min[i] < bbox1.m_Min[i] )
			result.m_Min[i] = bbox0.m_Min[i];
		else
			result.m_Min[i] = bbox1.m_Min[i];

		if( bbox0.m_Max[i] > bbox1.m_Max[i] )
			result.m_Max[i] = bbox0.m_Max[i];
		else
			result.m_Max[i] = bbox1.m_Max[i];
	}

	return result;
}

#endif
