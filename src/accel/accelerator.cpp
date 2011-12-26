/*
   FileName:      accelerator.cpp

   Created Time:  2011-08-04 12:52:56

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "accelerator.h"
#include "geometry/primitive.h"

// compute the bounding box
void Accelerator::_computeBBox()
{
	// reset bounding box
	m_BBox.InvalidBBox();

	// update bounding box again
	vector<Primitive*>::const_iterator it = m_primitives->begin();
	while( it != m_primitives->end() )
	{
		m_BBox.Union( (*it)->GetBBox() );	
		it++;
	}

	// enlarge the bounding box a little
	const float threshold = 0.001f;
	Vector delta = (m_BBox.m_Max - m_BBox.m_Min ) * threshold;
	m_BBox.m_Min -= delta;
	m_BBox.m_Max += delta;
}
