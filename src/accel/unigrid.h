/*
 * filename :	unigrid.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_UNIGRID
#define	SORT_UNIGRID

// include the header file
#include "accelerator.h"

////////////////////////////////////////////////////////////////////////////////
// definition of uniform grid
class UniGrid : public Accelerator
{
// public method
public:
	// default constructor
	UniGrid();
	// constructor from a primitive list
	// para 'l' : the primitive list
	UniGrid( vector<Primitive*>* l );
	// destructor
	~UniGrid();

	// get the intersection between the ray and the primitive set
	// para 'r' : the ray
	// para 'intersect' : the intersection result
	// result   : 'true' if the ray pirece one of the triangle in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	// build the acceleration structure
	virtual void Build();

// private field
private:
	// the number of voxels
	unsigned	m_voxelCount;
	// the voxel count
	unsigned	m_voxelNum[3];
	// extent of grid in each dimension
	float		m_voxelExtent[3];
	float		m_voxelInvExtent[3];
	// the voxel data
	vector<Primitive*>*	m_pVoxels;
	// the delta
	const float m_delta;

	// initialize the data
	void _init();
	// release the data
	void _release();
	// from point to voxel
	unsigned _point2VoxelId( const Point& p , unsigned axis ) const;
	// give the bottom left corner of the voxel
	Point	_voxelId2Point( int voxel[3] ) const;
	// get the id offset
	unsigned _offset( unsigned x , unsigned y , unsigned z ) const;
	// get intersect in grid
	// if the voxel id is out of range , there will be a runtime error
	bool _getIntersect( const Ray& r , Intersection* intersect , unsigned voxelId , float nextT ) const;
};

#endif
