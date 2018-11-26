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

#include "unigrid.h"
#include "geometry/primitive.h"
#include "geometry/intersection.h"
#include "utility/log.h"
#include "utility/sassert.h"

SORT_STATS_DEFINE_COUNTER(sUGGridCount)
SORT_STATS_DEFINE_COUNTER(sUniformGridX)
SORT_STATS_DEFINE_COUNTER(sUniformGridY)
SORT_STATS_DEFINE_COUNTER(sUniformGridZ)

SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Grid Count", sUGGridCount);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Dimension X", sUniformGridX);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Dimension Y", sUniformGridY);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Dimension Z", sUniformGridZ);
SORT_STATS_AVG_COUNT("Spatial-Structure(UniformGrid)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

IMPLEMENT_CREATOR( UniGrid );

// destructor
UniGrid::~UniGrid()
{
    SORT_PROFILE("Destructe Uniform Grid");
	release();
}

// release the data
void UniGrid::release()
{
	SAFE_DELETE_ARRAY( m_pVoxels );
	for( int i = 0 ; i < 3 ; i++ )
	{
		m_voxelNum[i] = 0;
		m_voxelExtent[i] = 0.0f;
		m_voxelInvExtent[i] = 0.0f;
	}
	m_voxelCount = 0;
}

// get the intersection between the ray and the primitive set
bool UniGrid::GetIntersect( const Ray& r , Intersection* intersect ) const
{
    SORT_PROFILE("Traverse Uniform Grid");
    SORT_STATS(++sRayCount);
    SORT_STATS(sShadowRayCount += intersect == nullptr);
    
	if( m_pVoxels == 0 || m_primitives == 0 )
		return false;

	// get the intersect point
	float maxt;
	float cur_t = Intersect( r , m_bbox , &maxt );
	if( cur_t < 0.0f )
		return false;
	if( intersect )
		intersect->t = std::min( intersect->t , maxt );

	int 	curGrid[3] , dir[3];
	float	delta[3] , next[3];
	for( int i = 0 ; i < 3 ; i++ )
	{
		curGrid[i] = point2VoxelId( r(cur_t) , i );
		dir[i] = ( r.m_Dir[i] > 0.0f ) ? 1 : -1;
		if( r.m_Dir[i] != 0.0f )
			delta[i] = fabs( m_voxelExtent[i] / r.m_Dir[i] );
		else
			delta[i] = FLT_MAX;
	}
	Point gridCorner = voxelId2Point( curGrid );
	for( int i = 0 ; i < 3 ; i++ )
	{
		// get the next t
		float target = gridCorner[i] + ((dir[i]+1)>>1) * m_voxelExtent[i];
		next[i] = ( target - r.m_Ori[i] ) / r.m_Dir[i];
		if( r.m_Dir[i] == 0.0f )
			next[i] = FLT_MAX;
	}

	// traverse the uniform grid
	const unsigned array[] = { 0 , 0 , 1 , 0 , 2 , 2 , 1 , 0  };// [0] and [7] is impossible
	while( ( intersect && cur_t < intersect->t ) || ( intersect == 0 ) )
	{
		// current voxel id
		unsigned voxelId = offset( curGrid[0] , curGrid[1] , curGrid[2] );

		// get the next t
		unsigned nextAxis = (next[0] <= next[1])+((unsigned)(next[1] <= next[2]))*2+((unsigned)(next[2] <= next[0]))*4;
		nextAxis = array[nextAxis];

		// chech if there is intersection in the current grid
		if( getIntersect( r , intersect , voxelId , next[nextAxis] ) )
			return true;

		// get to the next voxel
		curGrid[nextAxis] += dir[nextAxis];

		if( curGrid[nextAxis] < 0 || (unsigned)curGrid[nextAxis] >= m_voxelNum[nextAxis] )
			return ( intersect && intersect->t < maxt && ( intersect->primitive != 0 ));

		// update next
		cur_t = next[nextAxis];
		next[nextAxis] += delta[nextAxis];
	}
	
	return ( intersect && intersect->t < maxt && ( intersect->primitive != 0 ));
}

// build the acceleration structure
void UniGrid::Build()
{
    SORT_PROFILE("Build Uniform Grid");
	if( nullptr == m_primitives || m_primitives->empty() ){
        slog( WARNING , SPATIAL_ACCELERATOR , "There is no primitive in uniform grid." );
		return;
	}

	// find the bounding box first
	computeBBox();

	// get the maxium extent id and distance
	unsigned id = m_bbox.MaxAxisId();
	Vector delta = m_bbox.m_Max - m_bbox.m_Min;
	float extent = delta[id];

	// get the total number of primitives
	unsigned count = (unsigned)m_primitives->size();
	
	// grid per distance
	float gridPerDistance = 3 * powf( (float)count , 0.333f ) / extent ;

	// the grid size
	for( int i = 0 ; i < 3 ; i++ )
	{
		m_voxelNum[i] = (unsigned)(std::min( 256.0f , gridPerDistance * delta[i] ));
		m_voxelInvExtent[i] = m_voxelNum[i] / delta[i];
		m_voxelExtent[i] = 1.0f / m_voxelInvExtent[i];
	}

	m_voxelCount = m_voxelNum[0] * m_voxelNum[1] * m_voxelNum[2];

	// allocate the memory
	SAFE_DELETE_ARRAY( m_pVoxels );
	m_pVoxels = new std::vector<Primitive*>[ m_voxelCount ];

	// distribute the primitives
	std::vector<Primitive*>::const_iterator it = m_primitives->begin();
	while( it != m_primitives->end() )
	{
		unsigned maxGridId[3];
		unsigned minGridId[3];
		for( int i = 0 ; i < 3 ; i++ )
		{
			minGridId[i] = point2VoxelId( (*it)->GetBBox().m_Min , i );
			maxGridId[i] = point2VoxelId( (*it)->GetBBox().m_Max , i );
		}

		for( unsigned i = minGridId[2] ; i <= maxGridId[2] ; i++ )
			for( unsigned j = minGridId[1] ; j <= maxGridId[1] ; j++ )
				for( unsigned k = minGridId[0] ; k <= maxGridId[0] ; k++ )
				{
					BBox bb;
					bb.m_Min = m_bbox.m_Min + Vector( (float)k , (float)j , (float)i ) * m_voxelExtent;
					bb.m_Max = bb.m_Min + m_voxelExtent;

					// only add the primitives if it is actually intersected
					if( (*it)->GetIntersect( bb ) )
						m_pVoxels[offset( k , j , i )].push_back( *it );
				}
		it++;
	}
    
    SORT_STATS(sUniformGridX = m_voxelNum[0]);
    SORT_STATS(sUniformGridY = m_voxelNum[1]);
    SORT_STATS(sUniformGridZ = m_voxelNum[2]);
    SORT_STATS(sUGGridCount = m_voxelCount);
}

// voxel id from point
unsigned UniGrid::point2VoxelId( const Point& p , unsigned axis ) const
{
	return std::min( m_voxelNum[axis] - 1 , (unsigned)( ( p[axis] - m_bbox.m_Min[axis] ) * m_voxelInvExtent[axis] ) );
}

// get the id offset
unsigned UniGrid::offset( unsigned x , unsigned y , unsigned z ) const
{
	return z * m_voxelNum[1] * m_voxelNum[0] + y * m_voxelNum[0] + x;
}

// voxel id to point
Point UniGrid::voxelId2Point( int id[3] ) const
{
	Point p;
	p.x = m_bbox.m_Min.x + id[0] * m_voxelExtent[0];
	p.y = m_bbox.m_Min.y + id[1] * m_voxelExtent[1];
	p.z = m_bbox.m_Min.z + id[2] * m_voxelExtent[2];

	return p;
}

// get intersection between the ray and the primitives in the grid
bool UniGrid::getIntersect( const Ray& r , Intersection* intersect , unsigned voxelId , float nextT ) const
{
    sAssertMsg( voxelId < m_voxelCount , SPATIAL_ACCELERATOR , "asfsa" );
    
	bool inter = false;
    for( auto voxel : m_pVoxels[voxelId] ){
        SORT_STATS(++sIntersectionTest);
		// get intersection
		inter |= voxel->GetIntersect( r , intersect );
		if( intersect == 0 && inter )
			return true;
	}

	return inter && ( intersect->t < nextT + 0.00001f );
}
