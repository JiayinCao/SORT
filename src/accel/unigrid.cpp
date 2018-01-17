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
#include "managers/logmanager.h"
#include "geometry/primitive.h"
#include "geometry/intersection.h"
#include "log/log.h"

IMPLEMENT_CREATOR( UniGrid );

// destructor
UniGrid::~UniGrid()
{
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
	if( m_pVoxels == 0 || m_primitives == 0 )
		return false;

	// get the intersect point
	float maxt;
	float cur_t = Intersect( r , m_bbox , &maxt );
	if( cur_t < 0.0f )
		return false;
	if( intersect )
		intersect->t = min( intersect->t , maxt );

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
		m_voxelNum[i] = (unsigned)(min( 256.0f , gridPerDistance * delta[i] ));
		m_voxelInvExtent[i] = m_voxelNum[i] / delta[i];
		m_voxelExtent[i] = 1.0f / m_voxelInvExtent[i];
	}

	m_voxelCount = m_voxelNum[0] * m_voxelNum[1] * m_voxelNum[2];

	// allocate the memory
	SAFE_DELETE_ARRAY( m_pVoxels );
	m_pVoxels = new vector<Primitive*>[ m_voxelCount ];

	// distribute the primitives
	vector<Primitive*>::const_iterator it = m_primitives->begin();
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

					// only add the triangle if it is actually intersected
					if( (*it)->GetIntersect( bb ) )
						m_pVoxels[offset( k , j , i )].push_back( *it );
				}
		it++;
	}
}

// voxel id from point
unsigned UniGrid::point2VoxelId( const Point& p , unsigned axis ) const
{
	return min( m_voxelNum[axis] - 1 , (unsigned)( ( p[axis] - m_bbox.m_Min[axis] ) * m_voxelInvExtent[axis] ) );
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

// get intersection between the ray and the triangles in the grid
bool UniGrid::getIntersect( const Ray& r , Intersection* intersect , unsigned voxelId , float nextT ) const
{
	if( voxelId >= m_voxelCount )
		LOG_ERROR<<"Voxel id is out of range.("<<voxelId<<"/"<<m_voxelCount<<")"<<CRASH;

	bool inter = false;
    for( auto voxel : m_pVoxels[voxelId] ){
		// get intersection
		inter |= voxel->GetIntersect( r , intersect );
		if( intersect == 0 && inter )
			return true;
	}

	return inter && ( intersect->t < nextT + 0.00001f );
}

// output log information
void UniGrid::OutputLog() const
{
	unsigned count = 0;
	for( unsigned i = 0 ; i < m_voxelCount ; i++ )
		count += (unsigned)m_pVoxels[i].size();
    
    slog( INFO , SPATIAL_ACCELERATOR , "Spatial accelerator is Uniform Grid." );
    slog( DEBUG , SPATIAL_ACCELERATOR , stringFormat( "Total grid count is %d. Grid dimension is %d x %d x %d. Average number of triangles per grid is %f" , m_voxelCount , m_voxelNum[0] , m_voxelNum[1] , m_voxelNum[2] , ((m_voxelCount==0)?0:(float)count/(float)m_voxelCount) ) );
}

