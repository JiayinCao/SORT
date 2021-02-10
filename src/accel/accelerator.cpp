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

#include "accelerator.h"
#include "core/primitive.h"

SORT_STATS_DEFINE_COUNTER(sRayCount)
SORT_STATS_DEFINE_COUNTER(sShadowRayCount)
SORT_STATS_DEFINE_COUNTER(sIntersectionTest)

#ifdef ENABLE_TRANSPARENT_SHADOW
bool Accelerator::GetAttenuation( Ray& ray , Spectrum& attenuation , RenderContext& rc , MediumStack* ms ) const {
    SurfaceInteraction intersection;
    intersection.query_shadow = true;
    if (!GetIntersect(rc, ray, intersection)) {
        if (ms)
            attenuation *= ms->Tr(ray, ray.m_fMax, rc);
        return false;
    }

    // primitive being null is a special coding meaning the ray is blocked by an opaque primitive.
    if( IS_PTR_INVALID( intersection.primitive ) ){
        attenuation = 0.0f;
        return true;
    }

    // get the material of the intersected primitive
    const MaterialBase* material = intersection.primitive->GetMaterial();
    sAssert( IS_PTR_VALID( material ) , SPATIAL_ACCELERATOR );

    // evaluate the transparency first in case it is fully opaque.
    attenuation = material->EvaluateTransparency(rc, intersection);

    // consider beam transmittance during ray traversal if medium is presented.
    if (ms && !attenuation.IsBlack() ) {
        attenuation *= ms->Tr(ray, intersection.t, rc);

        const auto theta_wi = dot(ray.m_Dir, intersection.gnormal);
        const auto theta_wo = -theta_wi;
        const auto interaction_flag = update_interaction_flag(theta_wi, theta_wo);

        // at this point, we know for sure the ray pass through the surface.
        MediumInteraction mi;
        mi.intersect = intersection.intersect;
        mi.mesh = intersection.primitive->GetMesh();
        material->UpdateMediumStack(mi, interaction_flag, *ms, rc);
    }

    ray.m_Ori = intersection.intersect;
    ray.m_fMin = 0.001f;              // avoid self collision again.
    ray.m_fMax -= intersection.t;

    return true;
}
#endif

bool Accelerator::UpdateMediumStack( Ray& ray , MediumStack& ms , RenderContext& rc, const bool reversed ) const{
	SurfaceInteraction intersection;

#ifdef ENABLE_TRANSPARENT_SHADOW
	intersection.query_shadow = false;
#endif

	if (!GetIntersect(rc, ray, intersection))
		return false;

    // make sure there is primitive intersected
    sAssert(IS_PTR_VALID(intersection.primitive), SPATIAL_ACCELERATOR);

	// get the material of the intersected primitive
	const MaterialBase* material = intersection.primitive->GetMaterial();
	sAssert( IS_PTR_VALID( material ) , SPATIAL_ACCELERATOR );

#if 0
	const auto theta_wi = dot(ray.m_Dir, intersection.gnormal);
	const auto theta_wo = -theta_wi;
	auto interaction_flag = update_interaction_flag(theta_wi, theta_wo);

    if (reversed) {
        if (SE_LEAVING == interaction_flag)
            interaction_flag = SE_ENTERING;
        else if (SE_ENTERING == interaction_flag)
            interaction_flag = SE_LEAVING;
    }
#else
    // The following logic is exactly the same with the above one.
    const auto theta_wi = dot(ray.m_Dir, intersection.gnormal);
    const auto interaction_flag = ((theta_wi > 0.0f) != (reversed)) ? SE_LEAVING : SE_ENTERING;
#endif

	// at this point, we know for sure the ray pass through the surface.
	MediumInteraction mi;
	mi.intersect = intersection.intersect;
    mi.mesh = intersection.primitive->GetMesh();
	material->UpdateMediumStack(mi, interaction_flag, ms, rc);

	ray.m_Ori = intersection.intersect;
	ray.m_fMin = 0.001f;              // avoid self collision again.
	ray.m_fMax -= intersection.t;

	return true;
}