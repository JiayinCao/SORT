/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

// include the header file
#include "accelerator.h"

////////////////////////////////////////////////////////////////////////////////
// definition of uniform grid
class UniGrid : public Accelerator
{
// public method
public:
	DEFINE_CREATOR( UniGrid , "uniform_grid" );

	// destructor
    ~UniGrid();

	// get the intersection between the ray and the primitive set
	// para 'r' : the ray
	// para 'intersect' : the intersection result
	// result   : 'true' if the ray pirece one of the triangle in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	// build the acceleration structure
	virtual void Build();

	// output log information
	void OutputLog() const;

// private field
private:
	// the number of voxels
	unsigned	m_voxelCount = 0;
	// the voxel count
    unsigned	m_voxelNum[3] = {};
	// extent of grid in each dimension
	Vector		m_voxelExtent;
	Vector		m_voxelInvExtent;
	// the voxel data
	vector<Primitive*>*	m_pVoxels = nullptr;
	// the delta
	const float m_delta = 0.00001f;

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
