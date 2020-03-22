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

#include "integratormethod.h"
#include "math/ray.h"
#include "math/interaction.h"
#include "scatteringevent/scatteringevent.h"
#include "scatteringevent/bsdf/bxdf_utils.h"
#include "core/primitive.h"
#include "material/material.h"
#include "light/light.h"
#include "medium/phasefunction.h"

SORT_FORCEINLINE float MisFactor( float f, float g ){
    return (f*f) / (f*f + g*g);
}

Spectrum    EvaluateDirect( const ScatteringEvent& se , const Ray& r , const Scene& scene , const Light* light , const LightSample& ls ,const BsdfSample& bs ){
    const auto& ip = se.GetInteraction();
    Spectrum radiance;
    Visibility visibility(scene);
    float light_pdf;
    float bsdf_pdf;
    const auto wo = -r.m_Dir;
    Vector wi;
    const auto li = light->sample_l( ip.intersect , &ls , wi , 0 , &light_pdf , 0 , 0 , visibility );
    if( light_pdf > 0.0f && !li.IsBlack() ){
        Spectrum f = se.Evaluate_BSDF( wo , wi );

#ifndef ENABLE_TRANSPARENT_SHADOW
        if( !f.IsBlack() && visibility.IsVisible() ){
            if( light->IsDelta() ){
                radiance += li * f / light_pdf;
            }else{
                bsdf_pdf = se.Pdf_BSDF( wo , wi );
                const auto weight = MisFactor( light_pdf , bsdf_pdf );
                radiance = li * f * weight / light_pdf;
            }
        }
#else
		if( !f.IsBlack() ){
			const auto attenuation = visibility.GetAttenuation();
			if (!attenuation.IsBlack()) {
				if (light->IsDelta()) {
					radiance += attenuation * li * f / light_pdf;
				} else {
					bsdf_pdf = se.Pdf_BSDF(wo, wi);
					const auto weight = MisFactor(light_pdf, bsdf_pdf);
					radiance = attenuation * li * f * weight / light_pdf;
				}
			}
		}
#endif
    }

    if( !light->IsDelta() ){
        const auto f = se.Sample_BSDF( wo , wi , bs , bsdf_pdf );
        if( !f.IsBlack() && bsdf_pdf != 0.0f ){
            const auto light_pdf = light->Pdf( ip.intersect , wi );
            if( light_pdf <= 0.0f )
                return radiance;
            const auto weight = MisFactor( bsdf_pdf , light_pdf );

            Spectrum li;
            SurfaceInteraction _ip;
            if( false == light->Le( Ray( ip.intersect , wi ) , &_ip , li ) )
                return radiance;

            visibility.ray = Ray( ip.intersect , wi , 0 , 0.001f , _ip.t - 0.001f );
#ifndef ENABLE_TRANSPARENT_SHADOW
            if( !li.IsBlack() && visibility.IsVisible() )
                radiance += li * f * weight / bsdf_pdf;
#else
			if( !li.IsBlack() ){
				const auto attenuation = visibility.GetAttenuation();
				if (!attenuation.IsBlack())
					radiance += attenuation * li * f * weight / bsdf_pdf;
			}
#endif
        }
    }

    return radiance;
}

Spectrum    EvaluateDirect(const ScatteringEvent& se, const Ray& r, const Scene& scene, const Light* light, const LightSample& ls, const BsdfSample& bs, const MaterialBase* material , const MediumStack& ms ) {
    const auto& ip = se.GetInteraction();
    Spectrum radiance;
    Visibility visibility(scene);
    float light_pdf;
    float bsdf_pdf;
    const auto wo = -r.m_Dir;
    Vector wi;
    const auto li = light->sample_l(ip.intersect, &ls, wi, 0, &light_pdf, 0, 0, visibility);
    if (light_pdf > 0.0f && !li.IsBlack()) {
        Spectrum f = se.Evaluate_BSDF(wo, wi);

#ifndef ENABLE_TRANSPARENT_SHADOW
        if (!f.IsBlack() && visibility.IsVisible()) {
            if (light->IsDelta()) {
                radiance += li * f / light_pdf;
            } else {
                bsdf_pdf = se.Pdf_BSDF(wo, wi);
                const auto weight = MisFactor(light_pdf, bsdf_pdf);
                radiance = li * f * weight / light_pdf;
            }
        }
#else
        // as long as the ray is passing through the surface, it is necessary to update the medium stack.
        // make sure a copy, instead of the original data is updated to avoid data pollution.
        MediumStack ms_copy = ms;
        const auto interaction_flag = update_interaction_flag(dot(wi, se.GetInteraction().gnormal), dot(wo, se.GetInteraction().gnormal));
        if (SE_Interaction::SE_REFLECTION != interaction_flag) {
            MediumInteraction mi;
            mi.intersect = se.GetInteraction().intersect;
            material->UpdateMediumStack(mi, interaction_flag, ms_copy);
        }

        if (!f.IsBlack()) {
            const auto attenuation = visibility.GetAttenuation( &ms_copy );
            if (!attenuation.IsBlack()) {
                if (light->IsDelta()) {
                    radiance += attenuation * li * f / light_pdf;
                }
                else {
                    bsdf_pdf = se.Pdf_BSDF(wo, wi);
                    const auto weight = MisFactor(light_pdf, bsdf_pdf);
                    radiance = attenuation * li * f * weight / light_pdf;
                }
            }
        }
#endif
    }

    if (!light->IsDelta()) {
        const auto f = se.Sample_BSDF(wo, wi, bs, bsdf_pdf);
        if (!f.IsBlack() && bsdf_pdf != 0.0f) {
            float light_pdf;
            light_pdf = light->Pdf(ip.intersect, wi);
            if (light_pdf <= 0.0f)
                return radiance;
            const auto weight = MisFactor(bsdf_pdf, light_pdf);

            Spectrum li;
            SurfaceInteraction _ip;
            if (false == light->Le(Ray(ip.intersect, wi), &_ip, li))
                return radiance;

            // Make sure the ray starts from the surface instead of the light because the state of medium stack is known at the surface intersection,
            // while the medium state at the light is totaly unknown. The medium state will be evaluated during shadow ray traversal.
            visibility.ray = Ray(ip.intersect, wi, 0, 0.001f, _ip.t - 0.001f);
#ifndef ENABLE_TRANSPARENT_SHADOW
            if (!li.IsBlack() && visibility.IsVisible())
                radiance += li * f * weight / bsdf_pdf;
#else
            // as long as the ray is passing through the surface, it is necessary to update the medium stack.
            // make sure a copy, instead of the original data is updated to avoid data pollution.
            MediumStack ms_copy = ms;
            const auto interaction_flag = update_interaction_flag(dot(wi, se.GetInteraction().gnormal), dot(wo, se.GetInteraction().gnormal));
            if (SE_Interaction::SE_REFLECTION != interaction_flag) {
                MediumInteraction mi;
                mi.intersect = se.GetInteraction().intersect;
                material->UpdateMediumStack(mi, interaction_flag, ms_copy);
            }

            if (!li.IsBlack()) {
                const auto attenuation = visibility.GetAttenuation(&ms_copy);
                if (!attenuation.IsBlack())
                    radiance += attenuation * li * f * weight / bsdf_pdf;
            }
#endif
        }
    }

    return radiance;
}

Spectrum    EvaluateDirect(const Point& ip, const PhaseFunction* ph, const Vector& wo, const Scene& scene, const Light* light, MediumStack ms) {
    Spectrum radiance;
    Visibility visibility(scene);
    float light_pdf;
    Vector wi;
    const LightSample ls(true);
    const auto li = light->sample_l(ip, &ls, wi, 0, &light_pdf, 0, nullptr, visibility);
    if (light_pdf > 0.0f && !li.IsBlack() ) {
        const auto f = ph->P(wo, wi);
        if (f > 0.0f) {
#ifdef ENABLE_TRANSPARENT_SHADOW
            const auto attenuation = visibility.GetAttenuation(&ms);
            if (!attenuation.IsBlack())
                radiance = attenuation * li * f / light_pdf;
#else
            const auto occluded = visibility.IsVisible();
            if (!occluded)
                radiance = li * f / light_pdf;
#endif
        }
    }

    return radiance;
}

// This is only used by SSS for now, since it is a smooth BRDF, there is no need to do MIS.
Spectrum SampleOneLight( const ScatteringEvent& se , const Ray& r, const SurfaceInteraction& inter, const Scene& scene, const MaterialBase* material, const MediumStack& ms) {
    // Uniformly choose a light, this may not be the optimal solution in case of more lights, need more research in this topic later.
    float light_pick_pdf = 0.0f;
    const auto light = scene.SampleLight( sort_canonical() , &light_pick_pdf );
    if(IS_PTR_INVALID(light))
        return 0.0f;

    Spectrum radiance;
    Visibility visibility(scene);
    const auto wo = -r.m_Dir;
    Vector wi;
    LightSample ls(true);
    auto light_pdf = 0.0f;
    const auto li = light->sample_l( inter.intersect , &ls , wi , 0 , &light_pdf , 0 , 0 , visibility );
    if( light_pdf > 0.0f && !li.IsBlack() ){
        Spectrum f = se.Evaluate_BSDF( wo , wi );

#ifndef ENABLE_TRANSPARENT_SHADOW
        if( !f.IsBlack() && visibility.IsVisible() )
            radiance += li * f / light_pdf / light_pick_pdf;
#else
        // as long as the ray is passing through the surface, it is necessary to update the medium stack.
        // make sure a copy, instead of the original data is updated to avoid data pollution.
        MediumStack ms_copy = ms;
        const auto interaction_flag = update_interaction_flag(dot(wi, se.GetInteraction().gnormal), dot(wo, se.GetInteraction().gnormal));
        if (SE_Interaction::SE_REFLECTION != interaction_flag) {
            MediumInteraction mi;
            mi.intersect = se.GetInteraction().intersect;
            material->UpdateMediumStack(mi, interaction_flag, ms_copy);
        }

		if( !f.IsBlack() ){
			const auto attenuation = visibility.GetAttenuation( &ms_copy );
			if( !attenuation.IsBlack() )
				radiance += attenuation * li * f / light_pdf / light_pick_pdf;
        }
#endif
    }
    return radiance;
}

Spectrum    EvaluateDirect( const Ray& r , const Scene& scene , const Light* light , const SurfaceInteraction& ip ,
                            const LightSample& ls ,const BsdfSample& bs , bool replaceSSS ){
    ScatteringEvent se( ip , replaceSSS ? SE_EVALUATE_ALL_NO_SSS : SE_EVALUATE_ALL );
    ip.primitive->GetMaterial()->UpdateScatteringEvent( se );
    return EvaluateDirect( se , r , scene , light , ls , bs );
}