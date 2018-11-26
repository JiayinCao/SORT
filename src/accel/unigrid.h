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

#pragma once

#include "accelerator.h"

//! @brief Uniform Grid.
/**
 * Uniform grid is the simplest spatial acceleration structure in a ray tracer.
 * Unlike other complex data structure, like KD-Tree, uniform grid takes linear
 * time complexity to build. However the travesal efficiency may be lower than
 * its peers.
 */
class UniGrid : public Accelerator
{
public:
	DEFINE_CREATOR( UniGrid , Accelerator , "uniform_grid" );

	//! Destructor releasing all voxel data.
    ~UniGrid() override;

    //! @brief Get intersection between the ray and the primitive set using uniform grid.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an existed intersection, if intersect is not empty, it will fill the
    //! structure and return the nearest intersection.
    //! If intersect is nullptr, it will stop as long as one intersection is found, it is not
    //! necessary to be the nearest one.
    //! False will be returned if there is no intersection at all.
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided, it stops as
    //!                     long as it finds an intersection. It is faster than the one with intersection information
    //!                     data and suitable for shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise it returns false.
    bool GetIntersect( const Ray& r , Intersection* intersect ) const override;

	//! Build uniform grid structure in O(N).
    void Build() override;

private:
	unsigned	                m_voxelCount = 0;               /**< Total number of voxels. */
    unsigned	                m_voxelNum[3] = {};             /**< Number of voxels along each axis. */
	Vector		                m_voxelExtent;                  /**< Extent of one voxel along each axis. */
	Vector		                m_voxelInvExtent;               /**< Inverse of extent of one voxel along each axis. */
	std::vector<Primitive*>*	m_pVoxels = nullptr;            /**< Vector holding all voxels. */

	//! Release all allocated memory.
	void release();
    
	//! @brief Locate the id of the voxel that the point belongs to along a specific axis.
    //! @param p    The point to be evaluated.
    //! @param axis The id of axis to be tested along.
    //! @return     The id of the voxel along the selected axis.
	unsigned point2VoxelId( const Point& p , unsigned axis ) const;
    
	//! @brief Caculate the point with the minimal values along each axis in the voxel.
    //! @param voxel    The id of the voxel to be Evaluated along three dimensions.
    //! @return         The point with minimal value along each axis in the voxel.
	Point	voxelId2Point( int voxel[3] ) const;
    
	//! @brief Translate voxel id from three-dimensional to one-dimentional.
    //! @param x ID of voxel along axis-x.
    //! @param y ID of voxel along axis-y.
    //! @param z ID of voxel along axis-z.
    //! @return  ID of the voxel in one single dimension.
	unsigned offset( unsigned x , unsigned y , unsigned z ) const;
	
    //! @brief Get the nearest intersection between a ray and the primitive set.
    //! @param r            The ray to be tested.
    //! @param intersect    A pointer to the intersection information. If it is empty, it will return true as long as there is an intersection detected, which is not necessarily the nearest one.
    //! @param voxelId      ID of the voxel to be tested.
    //! @param nextT        The intersected position of the ray and the next to-be-traversed voxel along the ray.
    //! @return             It will return true if there is an intersection, otherwise it returns false.
	bool getIntersect( const Ray& r , Intersection* intersect , unsigned voxelId , float nextT ) const;
    
    SORT_STATS_ENABLE( "Spatial-Structure(UniformGrid)" )
};
