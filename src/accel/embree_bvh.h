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
 * This is a thirdparty accelerator from Intel.
 * It is well optimized for Intel chip by utilizing SIMD instructions.
 */
class EmbreeBVH : public Accelerator
{
public:
	DEFINE_CREATOR( EmbreeBVH , Accelerator , "embree_bvh" );

	//! Destructor releasing all voxel data.
    ~EmbreeBVH() override;

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

	//! Output log information
	void OutputLog() const override;
};
