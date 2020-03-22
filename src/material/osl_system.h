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

// This file wraps most of OSL related interface

#pragma once

#include <OSL/oslexec.h>
#include <string>
#include "core/define.h"
#include "math/vector3.h"
#include "spectrum/spectrum.h"
#include "closures.h"
#include "scatteringevent/scatteringevent.h"

#if defined(SORT_IN_WINDOWS)
    #define STDOSL_PATH     "..\\src\\stdosl.h"
#else
    #define STDOSL_PATH     "../src/stdosl.h"
#endif

class ScatteringEvent;
struct SurfaceInteraction;
class MediumStack;
struct MediumInteraction;
class Mesh;

struct ShadingContextWrapper {
public:
    OSL::ShadingContext* GetShadingContext(OSL::ShadingSystem* shadingsys);
    void DestroyContext(OSL::ShadingSystem* shadingsys);

private:
    OSL::PerThreadInfo              *thread_info = nullptr;
    OSL::ShadingContext             *ctx = nullptr;
};

//! @brief  Begin building shader
OSL::ShaderGroupRef BeginShaderGroup( const std::string& group_name );
bool EndShaderGroup();

//! @brief  Optimize shader
void OptimizeShader(OSL::ShaderGroup* group);

//! @brief  Build a shader from source code
bool BuildShader( const std::string& shader_source, const std::string& shader_name, const std::string& shader_layer , const std::string& shader_group_name = "" );

//! @brief  Connect parameters between shaders
bool ConnectShader( const std::string& source_shader , const std::string& source_param , const std::string& target_shader , const std::string& target_param );

//! @brief  Execute a shader and populate the scattering event
//!
//! @param  shader      The osl shader to be evaluated.
//! @param  se          The resulting scattering event.
void ExecuteSurfaceShader( OSL::ShaderGroup* shader , ScatteringEvent& se );

//! @brief  Execute a shader and populate the medium stack
//!
//! @param  shader      The osl shader to be evaluated.
//! @param  mi          The interaction inside the medium.
//! @param  ms          The medium stack to hold the results.
//! @param  flag        A flag indicates whether to add or remove the medium.
//! @param  material    The material that spawns the medium.
void ExecuteVolumeShader( OSL::ShaderGroup* shader, const MediumInteraction& mi , MediumStack& se, const SE_Interaction flag, const MaterialBase* material);

//! @brief  Evaluate the properties at a specific position in the volume.
//!
//! @param  shader      The osl shader to be executed.
//! @param  mi          The interaction inside the volume.
//! @param  ms          The medium sample to be returned.
void EvaluateVolumeSample( OSL::ShaderGroup* shader, const MediumInteraction& mi, MediumSample& ms);

//! @brief  Evaluate the transparency of the intersection.
//!
//! @param  shader          The osl shader to be evaluated.
//! @param  intersection    The intersection of interest.
Spectrum EvaluateTransparency( OSL::ShaderGroup* shader , const SurfaceInteraction& intersection );

//! @brief  Create thread contexts
void CreateOSLThreadContexts();

//! @brief  Destroy thread contexts
void DestroyOSLThreadContexts();