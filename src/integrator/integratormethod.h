/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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

#include "integrator.h"

struct    SurfaceInteraction;
class    Light;
class   MediumStack;
struct  RenderContext;

// evaluate direct lighting
Spectrum    EvaluateDirect(const ScatteringEvent& se, const Ray& r, const Scene& scene, const Light* light, const LightSample& ls, const BsdfSample& bs, const MaterialBase* material, const MediumStack& ms, RenderContext& rc);
Spectrum    EvaluateDirect(const ScatteringEvent& se, const Ray& r, const Scene& scene, const Light* light, const LightSample& ls, const BsdfSample& bs, RenderContext& rc);

Spectrum    EvaluateDirect(const Point& ip, const PhaseFunction* ph, const Vector& wo, const Scene& scene, const Light* light, MediumStack ms, RenderContext& rc);

// uniformly evaluate direct illumination from one light
Spectrum    SampleOneLight( const ScatteringEvent& se , const Ray& r, const SurfaceInteraction& inter, const Scene& scene, const MaterialBase* material, const MediumStack& ms, RenderContext& rc);

// helper function to evaluate light contribution
Spectrum    EvaluateDirect( const Ray& r , const Scene& scene , const Light* light , const SurfaceInteraction& ip ,
                            const LightSample& ls , const BsdfSample& bs , RenderContext& rc, bool replaceSSS = false );