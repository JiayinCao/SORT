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

#include <vector>
#include "core/define.h"
#include "math/bbox.h"
#include "core/creator.h"
#include "core/stats.h"
#include "core/profile.h"
#include "stream/stream.h"

//! @brief Spatial acceleration structure interface.
/**
 * Accelerator is an interface rather than a base class. There is no instance of it.
 * It is responsible for acceleration of intersection tests between ray and primitives.
 * Tracing a ray against the scene without a spatial acceleration structure is O(N) where
 * N is the number of primitives. Spatial acceleration structure can optimize the 
 * algorithm so that it is O(lg(N)), a significant improvement over the naive brute 
 * force ray tracing. Common spatial structures include KD-Tree, BVH and Uniform Grid.
 */
class	Accelerator : public SerializableObject{
public:
	//! @brief  Empty destructor.
    virtual ~Accelerator() = default;

    //! @brief Get intersection between the ray and the primitive set using BVH.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an existed intersection, if intersect is not empty, it will fill the
    //! structure and return the nearest intersection.If intersect is empty, it will stop 
    //! as long as one intersection is found, it is not necessary to be the nearest one.
    //! False will be returned if there is no intersection at all.
    //!
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided, it 
    //!                     stops as long as it finds an intersection. It is faster than 
    //!                     the one with intersection information data and suitable for 
    //!                     shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise 
    //!                     it returns false.
	virtual bool GetIntersect( const class Ray& r , class Intersection* intersect ) const = 0;

    //! @brief Build the acceleration structure.
	virtual void Build() = 0;

	//! @brief Get the bounding box of the primitive set.
    //!
    //! @return Bounding box of the spatial acceleration structure.
	inline const BBox& GetBBox() const { 
        return m_bbox; 
    }

    //! @brief  Set primitive set in the acceleration structure.
    //!
    //! @param pri  The set of primitives in the scene.
	inline void SetPrimitives( std::vector<class Primitive*>* pri ){
		m_primitives = pri;
	}
    
    //! @brief  Whether the spatial data structure is constructed.
    //!
    //! @return     Whether the spatial data structure is constructed.
    inline bool GetIsValid() const { 
        return m_isValid; 
    }

protected:
    /**< The vector holding all primitive pointers. */
	std::vector<class Primitive*>*      m_primitives;
    /**< The bounding box of all primitives. */
    BBox                                m_bbox;
    /**< Whether the spatial structure is constructed before. */
    bool                                m_isValid = false;

	//! @brief Generate the bounding box for the primitive set.
	void computeBBox();
};
