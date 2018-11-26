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

#include "utility/define.h"
#include <vector>
#include "geometry/bbox.h"
#include "utility/creator.h"
#include "utility/stats.h"
#include "utility/profile.h"

class Primitive;
class Intersection;
class Ray;

//! @brief Spatial acceleration structure interface.
/**
 * Accelerator is an interface rather than a base class. There is no instance of it.
 * It is responsible for acceleration of intersection tests between ray and primitives.
 * Tracing a ray against the scene without a spatial acceleration structure is O(N) where
 * N is the number of primitives. Spatial acceleration structure can optimize the 
 * algorithm so that it is O(lg(N)), a significant improvement over the naive brute 
 * force ray tracing. Common spatial structures inlcude KD-Tree, BVH and Uniform Grid.
 */
class	Accelerator
{
public:
	//! Destructor of Accelerator, nothing is done in it.
    virtual ~Accelerator() {};

    //! @brief Get intersection between the ray and the primitive set.
    //!
    //! Pretty much all spatial accelerators perform this operation in O(lg(N)) where n is 
    //! he number of primitives in the set. It will return true if there is intersection 
    //! between the ray and the primitive set. In case of an existed intersection, if intersect 
    //! is not empty, it will fill the structure and return the nearest intersection.
    //! If intersect is nullptr, it will stop as long as one intersection is found, it is not
    //! necessary to be the nearest one.
    //! False will be returned if there is no intersection at all.POC
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided, it stops as
    //!                     long as it finds an intersection. It is faster than the one with intersection information
    //!                     data and suitable for shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise it returns false.
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const = 0;

    //! @brief Build the acceleration structure.
	virtual void Build() = 0;

	//! @brief Get the bounding box of the primitive set.
    //! @return Bounding box of the spatial acceleration structure.
	const BBox& GetBBox() const { return m_bbox; }

    //! @brief Set primitive set in the acceleration structure.
    //! @param pri The set of primitives in the scene.
	void SetPrimitives( std::vector<Primitive*>* pri ){
		m_primitives = pri;
	}

protected:
	std::vector<Primitive*>*    m_primitives;   /**< The vector holding all pritmitive pointers. */
	BBox                        m_bbox;         /**< The bounding box of all pritmives. */

	//! Generate the bounding box for the primitive set.
	void computeBBox();
};
