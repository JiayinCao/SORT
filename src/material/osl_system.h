/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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
#include "closures.h"

#if defined(SORT_IN_WINDOWS)
    #define STDOSL_PATH     "..\\src\\stdosl.h"
#else
    #define STDOSL_PATH     "../src/stdosl.h"
#endif

struct ShadingContextWrapper {
public:
    OSL::ShadingContext* GetShadingContext(OSL::ShadingSystem* shadingsys);
    void DestroyContext(OSL::ShadingSystem* shadingsys);

private:
    OSL::PerThreadInfo              *thread_info = nullptr;
    OSL::ShadingContext             *ctx = nullptr;
};

// begin building shader
OSL::ShaderGroupRef BeginShaderGroup( const std::string& group_name );
bool EndShaderGroup();

// Optimize shader
void OptimizeShader(OSL::ShaderGroup* group);

// Build a shader from source code
bool BuildShader( const std::string& shader_source, const std::string& shader_name, const std::string& shader_layer , const std::string& shader_group_name = "" );

// Connect parameters between shaders
bool ConnectShader( const std::string& source_shader , const std::string& source_param , const std::string& target_shader , const std::string& target_param );

// Execute a shader and populate the bsdf
void ExecuteShader( class Bsdf* bsdf , class Bssrdf*& bssrdf , const class Intersection& intersection , OSL::ShaderGroup* shader );

// Create thread contexts
void CreateOSLThreadContexts();

// Destroy thread contexts
void DestroyOSLThreadContexts();