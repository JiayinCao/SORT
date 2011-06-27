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
#include "geometry/intersection.h"

// default constructor
UniGrid::UniGrid():
m_delta( 0.00001f )
{
	_init();
}

// constructor from a primitive list
UniGrid::UniGrid( vector<Primitive*>* l ) :
Accelerator(l) , m_delta( 0.01f )
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
		m_voxelExtent[i] = 0.0f;
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
		m_voxelExtent[i] = 0.0f;
		m_voxelInvExtent[i] = 0.0f;
	}
	m_voxelCount = 0;
}

// get the intersection between the ray and the primitive set
bool UniGrid::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	if( m_pVoxels == 0 || m_primitives == 0 )
	{
		LOG_WARNING<<"There is no primitive data in uniform grid."<<ENDL;
		return false;
	}

	// clear the mail box first
	memset( m_pMailBox , 0 , sizeof( unsigned char ) * ( m_primitives->size() + 7 ) / 8 );

	// get the intersect point
	float cur_t = Intersect( r , m_BBox , &(intersect->t) );
	float maxt = intersect->t;
	if( cur_t < 0.0f )
		return false;

	int 	curGrid[3] , dir[3];
	float	delta[3] , next[3];
	for( int i = 0 ; i < 3 ; i++ )
	{
		curGrid[i] = _point2VoxelId( r(cur_t) , i );
		dir[i] = ( r.m_Dir[i] > 0.0f ) ? 1 : -1;
		if( r.m_Dir[i] != 0.0f )
			delta[i] = fabs( m_voxelExtent[i] / r.m_Dir[i] );
		else
			delta[i] = FLT_MAX;
	}
	Point gridCorner = _voxelId2Point( curGrid );
	for( int i = 0 ; i < 3 ; i++ )
	{
		// get the next t
		float target = gridCorner[i] + ((dir[i]+1)>>1) * m_voxelExtent[i];
		next[i] = ( target - r.m_Ori[i] ) / r.m_Dir[i];
	}

	// traverse the uniform grid
	const unsigned array[] = { 0 , 0 , 1 , 0 , 2 , 2 , 1 , 0  };// [0] and [7] is impossible
	while( cur_t < intersect->t )
	{
		// current voxel id
		unsigned voxelId = _offset( curGrid[0] , curGrid[1] , curGrid[2] );

		// get the next t
		unsigned nextAxis = (next[0] <= next[1])+((unsigned)(next[1] <= next[2]))*2+((unsigned)(next[2] <= next[0]))*4;
		nextAxis = array[nextAxis];

		// chech if there is intersection in the current grid
		if( _getIntersect( r , intersect , voxelId , next[nextAxis] ) )
			return true;

		// get to the next voxel
		curGrid[nextAxis] += dir[nextAxis];

		if( curGrid[nextAxis] < 0 || (unsigned)curGrid[nextAxis] >= m_voxelNum[nextAxis] )
			return intersect->t < maxt;

		// update next
		cur_t = next[nextAxis];
		next[nextAxis] += delta[nextAxis];
	}
	
	return intersect->t < maxt;
}

// build the acceleration structure
void UniGrid::Build()
{
	if( m_primitives == 0 || m_primitives->empty() )
		LOG_WARNING<<"There is no primitive in the uniform grid"<<ENDL;

	// find the bounding box first
	_computeBBox();

	// get the maxium extent id and distance
	unsigned id = m_BBox.MaxAxisId();
	Vector delta = m_BBox.m_Max - m_BBox.m_Min;
	float extent = delta[id];

	// get the total number of primitives
	unsigned count = m_primitives->size();
	
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
			minGridId[i] = _point2VoxelId( (*it)->GetBBox().m_Min , i );
			maxGridId[i] = _point2VoxelId( (*it)->GetBBox().m_Max , i );
		}

		for( unsigned i = minGridId[2] ; i <= maxGridId[2] ; i++ )
			for( unsigned j = minGridId[1] ; j <= maxGridId[1] ; j++ )
				for( unsigned k = minGridId[0] ; k <= maxGridId[0] ; k++ )
				{
					unsigned offset = _offset( k , j , i );
					m_pVoxels[offset].push_back( *it );
				}
		it++;
	}

	// create the mail box
	SAFE_DELETE_ARRAY(m_pMailBox);
	m_pMailBox = new unsigned char[ ( count + 7 ) / 8  ];
}

// voxel id from point
unsigned UniGrid::_point2VoxelId( const Point& p , unsigned axis ) const
{
	return min( m_voxelNum[axis] - 1 , (unsigned)( ( p[axis] - m_BBox.m_Min[axis] ) * m_voxelInvExtent[axis] ) );
}

// get the id offset
unsigned UniGrid::_offset( unsigned x , unsigned y , unsigned z ) const
{
	return z * m_voxelNum[1] * m_voxelNum[0] + y * m_voxelNum[0] + x;
}

// voxel id to point
Point UniGrid::_voxelId2Point( int id[3] ) const
{
	Point p;
	p.x = m_BBox.m_Min.x + id[0] * m_voxelExtent[0];
	p.y = m_BBox.m_Min.y + id[1] * m_voxelExtent[1];
	p.z = m_BBox.m_Min.z + id[2] * m_voxelExtent[2];

	return p;
}

// get intersection between the ray and the triangles in the grid
bool UniGrid::_getIntersect( const Ray& r , Intersection* intersect , unsigned voxelId , float nextT ) const
{
	if( voxelId >= m_voxelCount )
		LOG_ERROR<<"Voxel id is out of range.("<<voxelId<<"/"<<m_voxelCount<<")"<<CRASH;

	bool inter = false;
	vector<Primitive*>::iterator it = m_pVoxels[voxelId].begin();
	while( it != m_pVoxels[voxelId].end() )
	{
		unsigned id = (*it)->GetID();
		unsigned offset = id / 8;
		unsigned flag = 0x00000001 << ( id % 8 );

		// get intersection
		if( 0 == (flag & m_pMailBox[offset]) )
			inter |= (*it)->GetIntersect( r , intersect );

		// mark the triangle as checked
		m_pMailBox[offset] |= flag;

		it++;
	}

	return inter && ( intersect->t < nextT + m_delta );
}

// output log information
void UniGrid::OutputLog() const
{
	LOG<<"Accelerator Type : Uniform Grid"<<ENDL;
	LOG<<"Total Grid Count : "<<m_voxelCount<<ENDL;
	LOG<<"Grid Dimenstion  : "<<m_voxelNum[0]<<"*"<<m_voxelNum[1]<<"*"<<m_voxelNum[2]<<ENDL;

	unsigned count = 0;
	for( unsigned i = 0 ; i < m_voxelCount ; i++ )
		count += m_pVoxels[i].size();
	LOG<<"Triangles per Grid: "<<(float)count/(float)m_voxelCount<<ENDL;
}

