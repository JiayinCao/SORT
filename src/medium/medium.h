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

#include "core/define.h"
#include "spectrum/spectrum.h"
#include "math/ray.h"
#include "math/interaction.h"
#include "core/strid.h"

class MaterialBase;

enum SE_Interaction : char {
    SE_REFLECTION = 0,
    SE_LEAVING = 1,
    SE_ENTERING = 2,
};

SORT_FORCEINLINE SE_Interaction update_interaction_flag(const float cos_theta_wi, const float cos_theta_wo) {
    if (cos_theta_wi > 0.0f && cos_theta_wo < 0.0f) {
        // the ray is leaving the surface from back side to front, removing the medium if presented.
        return SE_LEAVING;
    }
    else if (cos_theta_wi < 0.0f && cos_theta_wo > 0.0f) {
        // the ray is entering the surface from from to back, push the medium to medium stack if there is medium attached.
        return SE_ENTERING;
    }

    // the ray is reflected, since there is no update happening in volume stack, whether it is reflected from inside or
    // outside is not of our interest anymore.
    return SE_REFLECTION;
}

//! @brief  Medium is a data structure holding volumetric rendering data.
class Medium{
public:
	//! @brief	Constructor taking the material that spawns the medium.
	//!
	//! @param	mat			Material that spawns the medium.
	Medium( const MaterialBase* material ) : material( material ){}

    //! @brief  Evaluation of beam transmittance.
    //!
    //! Beam transmittance is how much percentage of radiance get attenuated during
    //! traveling through the medium. It is a spectrum dependent attenuation.
    //!
    //! @param  ray         The ray, which it uses to evaluate beam transmittance.
    //! @param  max_t       The maximum distance to be considered, usually this is the distance the ray travels before it hits a surface.
    //! @return             The attenuation of each spectrum channel.
    virtual Spectrum Tr(const Ray& ray, const float max_t) const = 0;

    //! @brief  Importance sampling a point along the ray in the medium.
    //!
    //! @param ray          The ray we use to take sample.
    //! @param  max_t       The maximum distance to be considered, usually this is the distance the ray travels before it hits a surface.
    //! @param mi           The interaction sampled.
    //! @return             The beam transmittance between the ray origin and the interaction.
    virtual Spectrum Sample( const Ray& ray , const float max_t , MediumInteraction*& interaction ) const = 0;

	//! @brief	Get the material that spawns the medium.
	//!
	//! @return				The material that spawns the medium.
	SORT_FORCEINLINE  const MaterialBase* GetMaterial() const {
		return material;
	}

protected:
	/**< Material that spawn the medium. */
	const MaterialBase*	material;
};

// There is only support up to 8 medium overlap each other, exceeding this limit will cause problems in rendering.
#define MEDIUM_MAX_CNT      8

//! @brief  Data structure to hold all mediums.
/**
 * Technically, MediumStack is not a stack. It is more of a general data container holding all mediums.
 * It is called 'stack' because it is a stack if every volume is fully contained in other volumes, which
 * could likely to be true most of the time.
 * This is the equivalent data structure of ScatteringEvent, except that this is for volumetric rendering.
 * These two data structures don't share a common code infrastructure because the fundamental differences
 * between them.
 */
class MediumStack {
public:
    //! @brief  Default constructor that does nothing.
    MediumStack() = default;

    //! @brief  Destructor that does nothing. It is not responsible for deallocating all memory it holds.
    ~MediumStack() = default;

    //! @brief  Add a medium in the data structure.
    //!
    //! @param  medium      Medium to be added.
    //! @return             Whether the medium is added in the stack.
    bool    AddMedium(const Medium* medium);

    //! @brief  Remove a medium from the data structure.
    //!
    //! @param  medium_id   This is the unique id of the material that spawns the medium.
    //! @return             Whether the medium is removed. If the medium is not even in the container, it will return false.
    bool    RemoveMedium(const StringID medium_id);

    //! @brief  Sample a point in the mediums.
    //!
    //! @param  r           The ray along which to take the sample.
    //! @param  max_t       The maximum distance to be considered, usually this is the distance the ray travels before it hits a surface.
    //! @param  mi          The medium interaction taken as a sample, null if no sample is taken in the medium.
    //! @return             Attenuation along the ray all the way to the sampled point.
    Spectrum    Sample(const Ray& r, const float max_t, MediumInteraction*& mi ) const;

public:
    /**< Mediums it holds. */
    const Medium*    m_mediums[MEDIUM_MAX_CNT] = { nullptr };

    /**< Number of mediums in the stack currently. */
    unsigned         m_mediumCnt = 0;
};