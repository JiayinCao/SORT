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
	const float threshold = 0.0001f;
	m_BBox.m_Min.x -= threshold;
	m_BBox.m_Min.y -= threshold;
	m_BBox.m_Min.z -= threshold;
	m_BBox.m_Max.x += threshold;
	m_BBox.m_Max.y += threshold;
	m_BBox.m_Max.z += threshold;
}
