/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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
 * time complexity to build. However the traversal efficiency may be lower than
 * its peers.
 */
class UniGrid : public Accelerator{
public:
    DEFINE_RTTI( UniGrid , Accelerator );

    //! @brief      Get intersection between the ray and the primitive set using KD-Tree.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an intersection, it will fill the structure and return the nearest intersection.
    //! This intersection could possibly be a fully transparent intersection, it is up to the higher
    //! level logic to handle (semi)transparency later.
    //!
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided,
    //!                     it stops as long as it finds an intersection. It is faster
    //!                     than the one with intersection information data and suitable
    //!                     for shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise
    //!                     it returns false.
    bool GetIntersect( const Ray& r , SurfaceInteraction& intersect ) const override;

#ifndef ENABLE_TRANSPARENT_SHADOW
    //! @brief This is a dedicated interface for detecting shadow rays.
    //!
    //! Instead of merging the interface with 'GetIntersect', this is a separate interface purely for occlusion detection.
    //! There is a need for it so that we can achieve better performance. There will be less branch in this interfaces and
    //! most importantly the traversed node doesn't need to be sorted.
    //!
    //! @param r            The ray to be tested.
    //! @return             Whether the ray is occluded by anything.
    bool IsOccluded( const Ray& r ) const override;
#endif

    //! @brief Get multiple intersections between the ray and the primitive set using spatial data structure.
    //!
    //! This is a specific interface designed for SSS during disk ray casting. Without this interface, the algorithm has to use the
    //! above one to acquire all intersections in a brute force way, which obviously introduces quite some duplicated work.
    //! The intersection returned doesn't guarantee the order of the intersection of the results, but it does guarantee to get the
    //! nearest N intersections.
    //! WARNING, it is quite possible to find an intersection that is fully transparent and still taking consideration of the light
    //! coming from that point. This is not strictly correct, but I would choose to live with it because it is not worth the extra
    //! performance overhead to fix the problem since it is very minor.
    //!
    //! @param  r           The input ray to be tested.
    //! @param  intersect   The intersection result that holds all intersection.
    //! @param  matID       We are only interested in intersection with the same material, whose material id should be set to matID.
    void GetIntersect( const Ray& r , BSSRDFIntersections& intersect , RenderContext& rc, const StringID matID = INVALID_SID ) const override;

    //! Build uniform grid structure in O(N).
    //!
    //! @param primitives       A vector holding all primitives.
    //! @param bbox             The bounding box of the scene.
    void    Build(const std::vector<const Primitive*>& primitives, const BBox& bbox) override;

    //! @brief      Serializing data from stream, this data structure is not configurable in Blender.
    //!
    //! @param      Stream where the serialization data comes from. Depending on different
    //!             situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override{}

	//! @brief	Clone the accelerator.
	//!
	//! Only configuration will be cloned, not the data inside the accelerator, this is for primitives that has volumes attached.
	//!
	//! @return		Cloned accelerator.
	std::unique_ptr<Accelerator>	Clone() const override;

private:
    /**< Total number of voxels. */
    unsigned                                    m_voxelCount = 0;
    /**< Number of voxels along each axis. */
    unsigned                                    m_voxelNum[3] = {};
    /**< Extent of one voxel along each axis. */
    Vector                                      m_voxelExtent;
    /**< Inverse of extent of one voxel along each axis. */
    Vector                                      m_voxelInvExtent;
    /**< Vector holding all voxels. */
    std::vector<std::vector<const Primitive*>>  m_voxels;

    //! @brief      Locate the id of the voxel that the point belongs to along a specific axis.
    //!
    //! @param p        The point to be evaluated.
    //! @param axis     The id of axis to be tested along.
    //! @return         The id of the voxel along the selected axis.
    unsigned point2VoxelId( const Point& p , unsigned axis ) const;

    //! @brief      Translate voxel id from three-dimensional to one-dimensional.
    //! @param x        ID of voxel along axis-x.
    //! @param y        ID of voxel along axis-y.
    //! @param z        ID of voxel along axis-z.
    //! @return         ID of the voxel in one single dimension.
    unsigned offset( unsigned x , unsigned y , unsigned z ) const;

    //! @brief      Get the nearest intersection between a ray and the primitive set.
    //! @param r            The ray to be tested.
    //! @param intersect    A pointer to the intersection information. If it is empty, it will return
    //!                     true as long as there is an intersection detected, which is not necessarily
    //!                     the nearest one.
    //! @param voxelId      ID of the voxel to be tested.
    //! @param nextT        The intersected position of the ray and the next to-be-traversed voxel along
    //!                     the ray.
    //! @return             It will return true if there is an intersection, otherwise it returns false.
    bool traverse( const Ray& r , SurfaceInteraction* intersect , unsigned voxelId , float nextT ) const;

    //! @brief      Get the nearest intersection between a ray and the primitive set.
    //! @param r            The ray to be tested.
    //! @param intersect    Intersection data structure holds all intersection results.
    //! @param voxelId      ID of the voxel to be tested.
    //! @param nextT        The intersected position of the ray and the next to-be-traversed voxel along
    //!                     the ray.
    //! @param matID        Material ID to avoid if it is not invalid.
    void traverse( const Ray& r , BSSRDFIntersections& intersect , unsigned voxelId , float nextT, RenderContext& rc , const StringID matID ) const;

    SORT_STATS_ENABLE( "Spatial-Structure(UniformGrid)" )
};
