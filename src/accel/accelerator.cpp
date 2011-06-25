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
}
