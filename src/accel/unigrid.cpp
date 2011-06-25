/*
 * filename :	unigrid.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "unigrid.h"
#include "managers/logmanager.h"
#include <math.h>
#include "geometry/primitive.h"

// default constructor
UniGrid::UniGrid()
{
	_init();
}

// constructor from a primitive list
UniGrid::UniGrid( vector<Primitive*>* l ) : Accelerator(l)
{
	_init();
}

// destructor
UniGrid::~UniGrid()
{
	_release();
}

// initialize data
void UniGrid::_init()
{
	for( int i = 0 ; i < 3 ; i++ )
	{
		m_voxelNum[i] = 0;
		m_voxelInvExtent[i] = 0.0f;
	}
	m_voxelCount = 0;
	m_pVoxels = 0;
}

// release the data
void UniGrid::_release()
{
	SAFE_DELETE_ARRAY( m_pVoxels );
	for( int i = 0 ; i < 3 ; i++ )
	{
		m_voxelNum[i] = 0;
		m_voxelInvExtent[i] = 0.0f;
	}
	m_voxelCount = 0;
}

// get the intersection between the ray and the primitive set
bool UniGrid::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	if( m_pVoxels == 0 )
		return false;

	// traverse the uniform grid

	return true;
}

// build the acceleration structure
void UniGrid::Build()
{
	if( m_primitives == 0 || m_primitives->empty() )
		LOG_WARNING<<"There is no data in the uniform grid"<<ENDL;

	// find the bounding box first
	_computeBBox();

	// get the maxium extent id and distance
	unsigned id = m_BBox.MaxAxisId();
	Vector delta = m_BBox.m_Max - m_BBox.m_Min;
	float extent = delta[id];

	// get the total number of primitives
	unsigned count = m_primitives->size();
	
	// grid per distance
	float gridPerDistance = 3 * powf( count , 0.333f ) / extent;

	// the grid size
	for( int i = 0 ; i < 3 ; i++ )
	{
		m_voxelNum[i] = (unsigned)(min( 64.0f , gridPerDistance * delta[i] ));
		m_voxelInvExtent[i] = m_voxelNum[i] / delta[i];
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
			minGridId[i] = _point2VoxelId( (*it)->GetBBox().m_Min , i );
			maxGridId[i] = _point2VoxelId( (*it)->GetBBox().m_Max , i );
		}

		for( int i = minGridId[2] ; i < maxGridId[2] ; i++ )
			for( int j = minGridId[1] ; j < maxGridId[1] ; j++ )
				for( int k = minGridId[0] ; k < maxGridId[0] ; k++ )
				{
					unsigned offset = _offset( k , j , i );
					m_pVoxels[offset].push_back( *it );
				}
		it++;
	}
}

// voxel id from point
unsigned UniGrid::_point2VoxelId( const Point& p , unsigned axis ) const
{
	return (unsigned)( ( p[axis] - m_BBox.m_Min[axis] ) * m_voxelInvExtent[axis] );
}

// get the id offset
unsigned UniGrid::_offset( unsigned x , unsigned y , unsigned z ) const
{
	return z * m_voxelNum[1] * m_voxelNum[0] + y * m_voxelNum[0] + x;
}
