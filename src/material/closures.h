/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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

#include <tsl_system.h>
#include <tsl_args.h>
#include "spectrum/spectrum.h"
#include "medium/medium.h"

struct RenderContext;
class ScatteringEvent;
class MediumStack;
class MaterialBase;
class Mesh;

//! @brief  Register all closures supported by SORT.
void RegisterClosures();

//! @brief  Process the closure tree result and populate the BSDF.
//!
//! @param  closure         The closure tree in the tsl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @param  se              The result scattering event.
void ProcessSurfaceClosure(const Tsl_Namespace::ClosureTreeNodeBase* closure, const Tsl_Namespace::float3& w, ScatteringEvent& se, RenderContext& rc);

//! @brief  Process the closure tree result and populate the MediumStack.
//!
//! @param  closure         The closure tree in the tsl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @param  mediumStack     The medium stack container that holds the result.
//! @param  flag            A flag indicates whether to add or remove the medium.
//! @param    material        The material that spawns the medium.
//! @param  mesh            The mesh that wraps the volume.
void ProcessVolumeClosure(const Tsl_Namespace::ClosureTreeNodeBase* closure, const Tsl_Namespace::float3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material, const Mesh* mesh, RenderContext& rc);

//! @brief  Evaluate the properties of the volume at specific position in a volume.
//!
//! Neither of absorption volurm, nor the homogeneous volume need this function. This is purely for heteregenous medium.
//!
//! @param  closure         The closure tree in the tsl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @param  ms              The medium sample to be returned.
void EvaluateVolumeSample(const Tsl_Namespace::ClosureTreeNodeBase* closure, const Tsl_Namespace::float3& w, MediumSample& ms);

//! @brief  Evaluate how opaque the surface is in the TSL shader.
//!
//! @param  closure         The closure tree in the tsl shader.
//! @param  w               The weight of this closure tree, this also counts the weight inherits from the higher level tree nodes.
//! @return                 The opacity of the intersection.
Spectrum ProcessOpacity(const Tsl_Namespace::ClosureTreeNodeBase* closure, const Tsl_Namespace::float3& w);
