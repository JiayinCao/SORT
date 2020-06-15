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

#include <tslversion.h>
#include <shading_system.h>
#include <shader_arg_types.h>
#include "spectrum/spectrum.h"
#include "medium/medium.h"

class ScatteringEvent;
class MediumStack;
class MaterialBase;
class Mesh;

enum CLOSURE_TYPE {
    SURFACE_CLOSURE_LAMBERT = 0,
    SURFACE_CLOSURE_OREN_NAYAR,
    SURFACE_CLOSURE_DISNEY,
    SURFACE_CLOSURE_MICROFACET_REFLECTION,
    SURFACE_CLOSURE_MICROFACET_REFRACTION,
    SURFACE_CLOSURE_ASHIKHMANSHIRLEY,
    SURFACE_CLOSURE_PHONG,
    SURFACE_CLOSURE_LAMBERT_TRANSMITTANCE,
    SURFACE_CLOSURE_MIRROR,
    SURFACE_CLOSURE_DIELETRIC,
    SURFACE_CLOSURE_MICROFACET_REFLECTION_DIELETRIC,
    SURFACE_CLOSURE_HAIR,
    SURFACE_CLOSURE_FOURIER_BDRF,
    SURFACE_CLOSURE_MERL_BRDF,
    SURFACE_CLOSURE_COAT,
    SURFACE_CLOSURE_DOUBLESIDED,
    SURFACE_CLOSURE_DISTRIBUTIONBRDF,
    SURFACE_CLOSURE_FABRIC,
    SURFACE_CLOSURE_SSS,
    SURFACE_CLOSURE_TRANSPARENT,

    VOLUME_CLOSURE_ABSORPTION,
    VOLUME_CLOSURE_HOMOGENEOUS,
    VOLUME_CLOSURE_HETEROGENOUS,
};

constexpr unsigned SURFACE_CLOSURE_CNT  = 20;
constexpr unsigned VOLUME_CLOSURE_CNT   = 3;
constexpr unsigned VOLUME_CLOSURE_BASE  = SURFACE_CLOSURE_CNT;
constexpr unsigned CLOSURE_CNT          = SURFACE_CLOSURE_CNT + VOLUME_CLOSURE_CNT;

//! @brief  Register all closures supported by SORT.
//!
//! @param  shadingsys      Shading system of OSL.
void RegisterClosures(Tsl_Namespace::ShadingSystem* shadingsys);

//! @brief  Process the closure tree result and populate the BSDF.
//!
//! @param  closure         The closure tree in the osl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @param  se              The result scattering event.
void ProcessSurfaceClosure(const Tsl_Namespace::ClosureTreeNodeBase* closure, const Tsl_Namespace::float3& w, ScatteringEvent& se);

/*
//! @brief  Process the closure tree result and populate the MediumStack.
//!
//! @param  closure         The closure tree in the osl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @param  mediumStack     The medium stack container that holds the result.
//! @param  flag            A flag indicates whether to add or remove the medium.
//! @param	material		The material that spawns the medium.
//! @param  mesh            The mesh that wraps the volume.
void ProcessVolumeClosure(const OSL::ClosureColor* closure, const OSL::Color3& w, MediumStack& mediumStack, const SE_Interaction flag, const MaterialBase* material, const Mesh* mesh);

//! @brief  Evaluate the properties of the volume at specific position in a volume.
//!
//! Neither of absorption volurm, nor the homogeneous volume need this function. This is purely for heteregenous medium.
//!
//! @param  closure         The closure tree in the osl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @param  ms              The medium sample to be returned.
void ProcessVolumeClosure(const OSL::ClosureColor* closure, const OSL::Color3& w, MediumSample& mediumStack);

//! @brief  Evaluate how opaque the surface is in the OSL shader.
//!
//! @param  closure         The closure tree in the osl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @return                 The opacity of the intersection.
Spectrum ProcessOpacity(const OSL::ClosureColor* closure, const OSL::Color3& w );
*/