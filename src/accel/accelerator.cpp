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

#include "accelerator.h"
#include "geometry/primitive.h"

SORT_STATS_DEFINE_COUNTER(sRayCount)
SORT_STATS_DEFINE_COUNTER(sShadowRayCount)
SORT_STATS_DEFINE_COUNTER(sIntersectionTest)

// Generate the bounding box for the primitive set.
void Accelerator::computeBBox()
{
	// reset bounding box
	m_bbox.InvalidBBox();

	// update bounding box again
    for( auto primitive : *m_primitives )
		m_bbox.Union( primitive->GetBBox() );

	// enlarge the bounding box a little
	static const float threshold = 0.001f;
	Vector delta = (m_bbox.m_Max - m_bbox.m_Min ) * threshold;
	m_bbox.m_Min -= delta;
	m_bbox.m_Max += delta;
}
