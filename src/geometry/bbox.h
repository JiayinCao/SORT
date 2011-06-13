/*
 * filename :	bbox.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_BBOX
#define SORT_BBOX

// include the header file
#include "../sort.h"
#include "point.h"
#include "ray.h"

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

	return result;
}
// para 'bbox0' :	first bounding box
// para 'bbox1' :	second bounding box
// result       :	a bounding box containing both of 'bbox0' and 'bbox1'
inline BBox Union( const BBox& bbox0 , const BBox& bbox1 )
{
	BBox result;

	for( int i = 0 ; i < 3 ; i++ )
	{
		result.m_Min[i] = min( bbox0.m_Min[i] , bbox1.m_Min[i] );
		result.m_Max[i] = max( bbox0.m_Max[i] , bbox1.m_Max[i] );
	}

	return result;
}

// bounding box and ray intersection
// para 'ray' : the ray
// para 'bb'  : the bounding box
// result     : true if the ray crosses the bounding box
inline bool Intersect( const Ray& ray , const BBox& bb )
{
	//set default value for tmax and tmin
	float tmax = 1000000.0f;
	float tmin = 0.0f;

	for( unsigned axis = 0 ; axis < 3 ; axis ++ )
	{
		if( ray.m_Dir[axis] < 0.00001f && ray.m_Dir[axis] > -0.00001f )
		{
			if( ray.m_Ori[axis] > bb.m_Max[axis] || ray.m_Ori[axis] < bb.m_Min[axis] )
				return false;
		}else
		{
			float ood = 1.0f / ray.m_Dir[axis];
			float t1 = ( bb.m_Max[axis] - ray.m_Ori[axis] ) * ood;
			float t2 = ( bb.m_Min[axis] - ray.m_Ori[axis] ) * ood;

			if( t1 > t2 )
			{
				float t = t1;
				t1 = t2;
				t2 = t;
			}

			tmin = max( t1 , tmin );
			tmax = min( t2 , tmax );

			if( tmin > tmax )
				return false;
		}
	}

	return true;
}

#endif
