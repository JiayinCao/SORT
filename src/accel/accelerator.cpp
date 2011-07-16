/*
 * filename :	accelerator.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "accelerator.h"
#include "geometry/primitive.h"

// compute the bounding box
void Accelerator::_computeBBox()
{
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
