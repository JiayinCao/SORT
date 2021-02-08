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

// This file wraps most of TSL related interface

#pragma once

#include <tsl_version.h>
#include <tsl_system.h>
#include <string>
#include "core/define.h"
#include "math/vector3.h"
#include "spectrum/spectrum.h"
#include "closures.h"
#include "scatteringevent/scatteringevent.h"

class ScatteringEvent;
struct SurfaceInteraction;
class MediumStack;
struct MediumInteraction;
class Mesh;
struct RenderContext;

// In an ideal world, I should have used different memory layout for different type of shaders.
// The following fields are obviously not valid in certain cases, like there is no normal in 
// a volume shader since there is no intersection, the same goes true the other way.
//
// TSL already supports per-shader tsl global. However, SORT decides to use consitent tsl global
// because it supports groupping nodes in Blender. With nodes with different tsl global memory 
// layout grouped together, it will result in compilation error.
// This is the main reason I didn't choose to use the feature.
DECLARE_TSLGLOBAL_BEGIN(TslGlobal)
DECLARE_TSLGLOBAL_VAR(Tsl_float3, uvw)
DECLARE_TSLGLOBAL_VAR(Tsl_float3, position)     // this is world space position
DECLARE_TSLGLOBAL_VAR(Tsl_float3, normal)       // this is world space normal
DECLARE_TSLGLOBAL_VAR(Tsl_float3, gnormal)      // this is world space geometric normal
DECLARE_TSLGLOBAL_VAR(Tsl_float3, I)            // this is world space input direction
DECLARE_TSLGLOBAL_VAR(Tsl_float, density)       // volume density
DECLARE_TSLGLOBAL_END()

//! @brief  Get Shading context.
std::shared_ptr<Tsl_Namespace::ShadingContext> GetShadingContext();

//! @brief  Execute Jited shader code.
void ExecuteSurfaceShader(Tsl_Namespace::ShaderInstance* shader, ScatteringEvent& se, RenderContext& rc);

//! @brief  Execute a shader and populate the medium stack
//!
//! @param  shader      The tsl shader to be evaluated.
//! @param  mi          The interaction inside the medium.
//! @param  ms          The medium stack to hold the results.
//! @param  flag        A flag indicates whether to add or remove the medium.
//! @param  material    The material that spawns the medium.
void ExecuteVolumeShader(Tsl_Namespace::ShaderInstance* shader, const MediumInteraction& mi , MediumStack& ms, const SE_Interaction flag, const MaterialBase* material, RenderContext& rc);

//! @brief  Evaluate the properties at a specific position in the volume.
//!
//! @param  shader      The tsl shader to be executed.
//! @param  mi          The interaction inside the volume.
//! @param  ms          The medium sample to be returned.
void EvaluateVolumeSample(Tsl_Namespace::ShaderInstance* shader, const MediumInteraction& mi, MediumSample& ms);

//! @brief  Evaluate the transparency of the intersection.
//!
//! @param  shader          The tsl shader to be evaluated.
//! @param  intersection    The intersection of interest.
Spectrum EvaluateTransparency(Tsl_Namespace::ShaderInstance* shader, const SurfaceInteraction& intersection);

//! @brief  Create thread contexts
void CreateTSLThreadContexts();

//! @brief  Destroy thread contexts
void DestroyTSLThreadContexts();

