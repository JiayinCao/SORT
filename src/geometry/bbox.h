/*
   FileName:      bbox.h

   Created Time:  2011-08-04 12:49:16

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_BBOX
#define SORT_BBOX

// include the header file
#include "sort.h"
#include "point.h"
#include "ray.h"

/////////////////////////////////////////////////////////////////////////////
// definition of axis aligned bounding box
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
	// para 'delta' :	enlarge the bounding box a little to avoid float format LOG_ERROR
	// result       :	return true if 'p' is in the bounding box
	bool IsInBBox( const Point& p , float delta ) const;

	// get the surface area of the bounding box
	// result :	the surface area of the bounding box
	float SurfaceArea() const;
	// half surface area
	float HalfSurfaceArea() const;

	// get the volumn of the bounding box
	// result :	the volumn of the bounding box
	float Volumn() const;

	// get the id of the axis with the longest extent
	// result :	the id of axis with the longest extent
	unsigned MaxAxisId() const;

	// union bbox
	// para 'p' : a point
	void Union( const Point& p );

	// union bbox
	// para 'box' : another bounding box
	void Union( const BBox& box );

	// delta in a specific axis
	float Delta( unsigned k ) const;

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
	BBox box;
	for( unsigned i = 0 ; i < 3 ; i++ )
	{
		if( p[i] < bbox.m_Min[i] )
			box.m_Min[i] = p[i];
		if( p[i] > bbox.m_Max[i] )
			box.m_Max[i] = p[i];
	}

	return box;
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
// para 'tmax': further away intersected point
// result     : the first intersected point and retur -1.0f if not crossed
inline float Intersect( const Ray& ray , const BBox& bb , float* fmax = 0 )
{
	//set default value for tmax and tmin
	float tmax = ray.m_fMax;
	float tmin = ray.m_fMin;

	for( unsigned axis = 0 ; axis < 3 ; axis ++ )
	{
		if( ray.m_Dir[axis] < 0.00001f && ray.m_Dir[axis] > -0.00001f )
		{
			if( ray.m_Ori[axis] > bb.m_Max[axis] || ray.m_Ori[axis] < bb.m_Min[axis] )
				return -1.0f;
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
				return -1.0f;
		}
	}

	if( fmax )
		*fmax = tmax;

	return tmin;
}

#endif
