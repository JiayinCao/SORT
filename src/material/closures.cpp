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

#include <OSL/oslclosure.h>
#include <OSL/genclosure.h>
#include "closures.h"
#include "osl_utils.h"
#include "core/memory.h"
#include "scatteringevent/bsdf/lambert.h"
#include "scatteringevent/bsdf/lambert.h"
#include "scatteringevent/bsdf/orennayar.h"
#include "scatteringevent/bsdf/disney.h"
#include "scatteringevent/bsdf/microfacet.h"
#include "scatteringevent/bsdf/ashikhmanshirley.h"
#include "scatteringevent/bsdf/phong.h"
#include "scatteringevent/bsdf/dielectric.h"
#include "scatteringevent/bsdf/hair.h"
#include "scatteringevent/bsdf/fourierbxdf.h"
#include "scatteringevent/bsdf/merl.h"
#include "scatteringevent/bsdf/coat.h"
#include "scatteringevent/bsdf/doublesided.h"
#include "scatteringevent/bsdf/distributionbrdf.h"
#include "scatteringevent/bsdf/fabric.h"
#include "scatteringevent/bsdf/transparent.h"
#include "scatteringevent/scatteringevent.h"
#include "scatteringevent/bssrdf/bssrdf.h"
#include "medium/medium.h"
#include "medium/absorption.h"
#include "medium/homogeneous.h"
#include "medium/heterogeneous.h"
#include "material/material.h"

using namespace OSL;

// These data structure is not supposed to be seen by other parts of the renderer
namespace {
    constexpr int MAXPARAMS = 32;

    struct Closure_Base {
        struct BuiltinClosures {
            const char* name;
            int id;
            ClosureParam params[MAXPARAMS];
        };

        Closure_Base() = default;
        virtual ~Closure_Base() = default;
    };

    struct Surface_Closure_Base : public Closure_Base {
        virtual Spectrum EvaluateOpacity(const ClosureComponent* comp, const OSL::Color3& w) const {
            return w;
        }

        virtual void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const = 0;
    };

    struct Volume_Closure_Base : public Closure_Base {
        virtual void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material) const = 0;
        virtual void Evaluate(const ClosureComponent* comp, const OSL::Color3& w, MediumSample& ms) const {}
    };

    static std::vector<std::unique_ptr<Surface_Closure_Base>>   g_surface_closures(SURFACE_CLOSURE_CNT);
    static std::vector<std::unique_ptr<Volume_Closure_Base>>    g_volume_closures(VOLUME_CLOSURE_CNT);

    inline static Surface_Closure_Base* getSurfaceClosureBase(unsigned int closure_id) {
        sAssert(closure_id >= 0 && closure_id < SURFACE_CLOSURE_CNT, MATERIAL);
        sAssert(g_surface_closures[closure_id] != nullptr, MATERIAL);

        return g_surface_closures[closure_id].get();
    }

    inline static Volume_Closure_Base* getVolumeClosureBase(unsigned int closure_id) {
        closure_id -= VOLUME_CLOSURE_BASE;

        sAssert(closure_id >= 0 && closure_id < VOLUME_CLOSURE_CNT, VOLUME);
        sAssert(g_volume_closures[closure_id] != nullptr, VOLUME);

        return g_volume_closures[closure_id].get();
    }

    struct Surface_Closure_Lambert : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_LAMBERT;

        static const char* GetName(){
            return "lambert";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID, {
                CLOSURE_COLOR_PARAM(Lambert::Params, baseColor),
                CLOSURE_VECTOR_PARAM(Lambert::Params, n),
                CLOSURE_FINISH_PARAM(Lambert::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w , ScatteringEvent& se) const override {
            const auto& params = *comp->as<Lambert::Params>();
            se.AddBxdf(SORT_MALLOC(Lambert)(params, w * comp->w));
        }
    };

    struct Surface_Closure_OrenNayar : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_OREN_NAYAR;

        static const char* GetName(){
            return "orenNayar";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID, {
                CLOSURE_COLOR_PARAM(OrenNayar::Params, baseColor),
                CLOSURE_FLOAT_PARAM(OrenNayar::Params, sigma),
                CLOSURE_VECTOR_PARAM(OrenNayar::Params, n),
                CLOSURE_FINISH_PARAM(OrenNayar::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w , ScatteringEvent& se) const override {
            const auto& params = *comp->as<OrenNayar::Params>();
            se.AddBxdf(SORT_MALLOC(OrenNayar)(params, w * comp->w));
        }
    };

    struct Surface_Closure_Disney : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_DISNEY;

        static const char* GetName(){
            return "disney";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, metallic),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, specular),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, specularTint),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, roughness),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, anisotropic),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, sheen),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, sheenTint),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, clearcoat),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, clearcoatGloss),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, specTrans),
                CLOSURE_COLOR_PARAM(DisneyBRDF::Params, scatterDistance),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, flatness),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, diffTrans),
                CLOSURE_INT_PARAM(DisneyBRDF::Params, thinSurface),
                CLOSURE_COLOR_PARAM(DisneyBRDF::Params, baseColor),
                CLOSURE_VECTOR_PARAM(DisneyBRDF::Params, n),
                CLOSURE_FINISH_PARAM(DisneyBRDF::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto weight = comp->w * w;
            const auto sample_weight = ( weight[0] + weight[1] + weight[2] ) / 3.0f;
            auto params = *comp->as<DisneyBRDF::Params>();
            auto& mfp = params.scatterDistance;

            // Ignore SSS if necessary
            if ( SE_NONE != ( se.GetFlag() & SE_REPLACE_BSSRDF ) )
                mfp = OSL::Vec3(0.0f);

            RGBSpectrum sssBaseColor = params.baseColor;

#ifdef SSS_REPLACE_WITH_LAMBERT
            constexpr float delta = 0.0001f;
            auto bssrdf_channel_weight = 0.0f;
            auto total_channel_weight = 0.0f;
            auto addExtraLambert = false;
            auto baseColor = params.baseColor;
            for (int i = 0; i < SPECTRUM_SAMPLE; ++i) {
                total_channel_weight += params.baseColor[i];

                // If the reflectance is zero or the mean free path is too small, switch back to lambert.
                if (params.baseColor[i] == 0.0f) {
                    mfp[i] = 0.0f;
                    continue;
                }

                // if the mean free distance is too small, replace it with lambert.
                if (mfp[i] < delta) {
                    mfp[i] = 0.0f;
                    sssBaseColor[i] = 0.0f;
                    addExtraLambert = true;
                }
                else {
                    baseColor[i] = 0.0f;
                    bssrdf_channel_weight += sssBaseColor[i];
                }
            }

            const auto bssrdf_pdf = bssrdf_channel_weight / total_channel_weight;
#else
            constexpr auto bssrdf_pdf = 1.0f;
#endif
            if (!( 0.0f == mfp[0] && 0.0f == mfp[1] && 0.0f == mfp[2] )) {
                const auto bxdf_sampling_weight = DisneyBRDF::Evaluate_Sampling_Weight( params );

                if( bxdf_sampling_weight > 0.0f )
                    se.AddBxdf(SORT_MALLOC(DisneyBRDF)(params, weight, bxdf_sampling_weight * sample_weight));

                const auto diffuseWeight = (Spectrum)( weight * (1.0f - params.metallic) * (1.0 - params.specTrans) );
                if (!sssBaseColor.IsBlack() && bxdf_sampling_weight < 1.0f && !diffuseWeight.IsBlack() )
                    se.AddBssrdf( SORT_MALLOC(DisneyBssrdf)(&se.GetInteraction(), sssBaseColor, params.scatterDistance, diffuseWeight , ( 1.0f - bxdf_sampling_weight ) * sample_weight * bssrdf_pdf ) );

#ifdef SSS_REPLACE_WITH_LAMBERT
                if (addExtraLambert && !baseColor.IsBlack())
                    se.AddBxdf(SORT_MALLOC(Lambert)(baseColor, diffuseWeight, ( 1.0f - bxdf_sampling_weight ) * sample_weight * ( 1.0f - bssrdf_pdf ) , params.n));
#endif
            }else{
                se.AddBxdf(SORT_MALLOC(DisneyBRDF)(params, weight));
            }
        }
    };

    struct Surface_Closure_MicrofacetReflection : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_MICROFACET_REFLECTION;

        static const char* GetName(){
            return "microfacetReflection";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_STRING_PARAM(MicroFacetReflection::Params, dist),
                CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, eta),
                CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, absorption),
                CLOSURE_FLOAT_PARAM(MicroFacetReflection::Params, roughnessU),
                CLOSURE_FLOAT_PARAM(MicroFacetReflection::Params, roughnessV),
                CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, baseColor),
                CLOSURE_VECTOR_PARAM(MicroFacetReflection::Params, n),
                CLOSURE_FINISH_PARAM(MicroFacetReflection::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<MicroFacetReflection::Params>();
            se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
        }
    };

    struct Surface_Closure_MicrofacetRefraction : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_MICROFACET_REFRACTION;

        static const char* GetName(){
            return "microfacetRefraction";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_STRING_PARAM(MicroFacetRefraction::Params, dist),
                CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, etaI),
                CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, etaT),
                CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, roughnessU),
                CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, roughnessV),
                CLOSURE_COLOR_PARAM(MicroFacetRefraction::Params, transmittance),
                CLOSURE_VECTOR_PARAM(MicroFacetRefraction::Params, n),
                CLOSURE_FINISH_PARAM(MicroFacetRefraction::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<MicroFacetRefraction::Params>();
            se.AddBxdf(SORT_MALLOC(MicroFacetRefraction)(params, w * comp->w));
        }
    };

    struct Surface_Closure_AshikhmanShirley : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_ASHIKHMANSHIRLEY;

        static const char* GetName(){
            return "ashikhmanShirley";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_FLOAT_PARAM(AshikhmanShirley::Params, specular),
                CLOSURE_FLOAT_PARAM(AshikhmanShirley::Params, roughnessU),
                CLOSURE_FLOAT_PARAM(AshikhmanShirley::Params, roughnessV),
                CLOSURE_COLOR_PARAM(AshikhmanShirley::Params, baseColor),
                CLOSURE_VECTOR_PARAM(AshikhmanShirley::Params, n),
                CLOSURE_FINISH_PARAM(AshikhmanShirley::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<AshikhmanShirley::Params>();
            se.AddBxdf(SORT_MALLOC(AshikhmanShirley)(params, w * comp->w));
        }
    };

    struct Surface_Closure_Phong : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_PHONG;

        static const char* GetName(){
            return "phong";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(Phong::Params, diffuse),
                CLOSURE_COLOR_PARAM(Phong::Params, specular),
                CLOSURE_FLOAT_PARAM(Phong::Params, specularPower),
                CLOSURE_VECTOR_PARAM(Phong::Params, n),
                CLOSURE_FINISH_PARAM(Phong::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<Phong::Params>();
            se.AddBxdf(SORT_MALLOC(Phong)(params, w * comp->w));
        }
    };

    struct Surface_Closure_LambertTransmission : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_LAMBERT_TRANSMITTANCE;

        static const char* GetName(){
            return "lambertTransmission";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(LambertTransmission::Params, transmittance),
                CLOSURE_VECTOR_PARAM(LambertTransmission::Params, n),
                CLOSURE_FINISH_PARAM(LambertTransmission::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<LambertTransmission::Params>();
            se.AddBxdf(SORT_MALLOC(LambertTransmission)(params, w * comp->w));
        }
    };

    struct Surface_Closure_Mirror : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_MIRROR;

        static const char* GetName(){
            return "mirror";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(MicroFacetReflection::MirrorParams, baseColor),
                CLOSURE_VECTOR_PARAM(MicroFacetReflection::MirrorParams, n),
                CLOSURE_FINISH_PARAM(MicroFacetReflection::MirrorParams)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<MicroFacetReflection::MirrorParams>();
            se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
        }
    };

    struct Surface_Closure_Dielectric : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_DIELETRIC;

        static const char* GetName(){
            return "dieletric";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(Dielectric::Params, reflectance),
                CLOSURE_COLOR_PARAM(Dielectric::Params, transmittance),
                CLOSURE_FLOAT_PARAM(Dielectric::Params, roughnessU),
                CLOSURE_FLOAT_PARAM(Dielectric::Params, roughnessV),
                CLOSURE_VECTOR_PARAM(Dielectric::Params, n),
                CLOSURE_FINISH_PARAM(Dielectric::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<Dielectric::Params>();
            se.AddBxdf(SORT_MALLOC(Dielectric)(params, w * comp->w));
        }
    };

    struct Surface_Closure_MicrofacetReflectionDielectric : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_MICROFACET_REFLECTION_DIELETRIC;

        static const char* GetName(){
            return "microfacetReflectionDieletric";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_STRING_PARAM(MicroFacetReflection::ParamsDieletric, dist),
                CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, iorI),
                CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, iorT),
                CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, roughnessU),
                CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, roughnessV),
                CLOSURE_COLOR_PARAM(MicroFacetReflection::ParamsDieletric, baseColor),
                CLOSURE_VECTOR_PARAM(MicroFacetReflection::ParamsDieletric, n),
                CLOSURE_FINISH_PARAM(MicroFacetReflection::ParamsDieletric)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<MicroFacetReflection::ParamsDieletric>();
            se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
        }
    };

    struct Surface_Closure_Hair : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_HAIR;

        static const char* GetName(){
            return "hair";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(Hair::Params, sigma),
                CLOSURE_FLOAT_PARAM(Hair::Params, longtitudinalRoughness),
                CLOSURE_FLOAT_PARAM(Hair::Params, azimuthalRoughness),
                CLOSURE_FLOAT_PARAM(Hair::Params, ior),
                CLOSURE_FINISH_PARAM(Hair::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<Hair::Params>();
            se.AddBxdf(SORT_MALLOC(Hair)(params, w * comp->w));
        }
    };

    struct Surface_Closure_FourierBRDF : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_FOURIER_BDRF;

        static const char* GetName(){
            return "fourierBRDF";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_INT_PARAM(FourierBxdf::Params, resIdx),
                CLOSURE_VECTOR_PARAM(FourierBxdf::Params, n),
                CLOSURE_FINISH_PARAM(FourierBxdf::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<FourierBxdf::Params>();
            se.AddBxdf(SORT_MALLOC(FourierBxdf)(params, w * comp->w));
        }
    };

    struct Surface_Closure_MERL : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_MERL_BRDF;

        static const char* GetName(){
            return "merlBRDF";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_INT_PARAM(Merl::Params, resIdx),
                CLOSURE_VECTOR_PARAM(Merl::Params, n),
                CLOSURE_FINISH_PARAM(Merl::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<Merl::Params>();
            se.AddBxdf(SORT_MALLOC(Merl)(params, w * comp->w));
        }
    };

    struct Surface_Closure_Coat : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_COAT;

        static const char* GetName(){
            return "coat";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_CLOSURE_PARAM(Coat::Params, closure),
                CLOSURE_FLOAT_PARAM(Coat::Params, roughness),
                CLOSURE_FLOAT_PARAM(Coat::Params, ior),
                CLOSURE_COLOR_PARAM(Coat::Params, sigma),
                CLOSURE_VECTOR_PARAM(Coat::Params, n),
                CLOSURE_FINISH_PARAM(Coat::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<Coat::Params>();
            ScatteringEvent* bottom = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
            ProcessSurfaceClosure(params.closure, Color3(1.0f), *bottom);
            se.AddBxdf(SORT_MALLOC(Coat)(params, w, bottom));
        }
    };

    struct Surface_Closure_DoubleSided : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_DOUBLESIDED;

        static const char* GetName(){
            return "doubleSided";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_CLOSURE_PARAM(DoubleSided::Params, bxdf0),
                CLOSURE_CLOSURE_PARAM(DoubleSided::Params, bxdf1),
                CLOSURE_FINISH_PARAM(DoubleSided::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<DoubleSided::Params>();
            ScatteringEvent* se0 = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
            ScatteringEvent* se1 = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
            ProcessSurfaceClosure(params.bxdf0, Color3(1.0f), *se0);
            ProcessSurfaceClosure(params.bxdf1, Color3(1.0f), *se1);
            se.AddBxdf(SORT_MALLOC(DoubleSided)(se0, se1, w));
        }
    };

    struct Surface_Closure_DistributionBRDF : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_DISTRIBUTIONBRDF;

        static const char* GetName(){
            return "distributionBRDF";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID, {
                CLOSURE_COLOR_PARAM(DistributionBRDF::Params, baseColor),
                CLOSURE_FLOAT_PARAM(DistributionBRDF::Params, roughness),
                CLOSURE_FLOAT_PARAM(DistributionBRDF::Params, specular),
                CLOSURE_FLOAT_PARAM(DistributionBRDF::Params, specularTint),
                CLOSURE_VECTOR_PARAM(DistributionBRDF::Params, n),
                CLOSURE_FINISH_PARAM(DistributionBRDF::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<DistributionBRDF::Params>();
            se.AddBxdf(SORT_MALLOC(DistributionBRDF)(params, w * comp->w));
        }
    };

    struct Surface_Closure_Fabric : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_FABRIC;

        static const char* GetName() {
            return "fabric";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(Fabric::Params, baseColor),
                CLOSURE_FLOAT_PARAM(Fabric::Params, roughness),
                CLOSURE_VECTOR_PARAM(Fabric::Params, n),
                CLOSURE_FINISH_PARAM(Fabric::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<Fabric::Params>();
            se.AddBxdf(SORT_MALLOC(Fabric)(params, w * comp->w));
        }
    };

    struct Surface_Closure_SSS : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_SSS;

        static const char* GetName() {
            return "subsurfaceScattering";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(DisneyBssrdf::Params, baseColor),
                CLOSURE_VECTOR_PARAM(DisneyBssrdf::Params, scatterDistance),
                CLOSURE_VECTOR_PARAM(DisneyBssrdf::Params, n),
                CLOSURE_FINISH_PARAM(DisneyBssrdf::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<DisneyBssrdf::Params>();
            if( isBlack(params.baseColor) )
                return;

            const auto weight = w * comp->w;

            if (SE_NONE == (se.GetFlag() & SE_REPLACE_BSSRDF)){
#ifdef SSS_REPLACE_WITH_LAMBERT
                auto sssBaseColor = params.baseColor;
                const auto pdf_weight = (weight[0] + weight[1] + weight[2]) / 3.0f;

                constexpr float delta = 0.0001f;
                auto bssrdf_channel_weight = 0.0f;
                auto total_channel_weight = 0.0f;
                auto mfp = params.scatterDistance;
                auto addExtraLambert = false;
                auto baseColor = params.baseColor;
                for (int i = 0; i < SPECTRUM_SAMPLE; ++i) {
                    total_channel_weight += params.baseColor[i];

                    // If the reflectance is zero or the mean free path is too small, switch back to lambert.
                    if (params.baseColor[i] == 0.0f) {
                        mfp[i] = 0.0f;
                        continue;
                    }

                    // if the mean free distance is too small, replace it with lambert.
                    if (mfp[i] < delta) {
                        mfp[i] = 0.0f;
                        sssBaseColor[i] = 0.0f;
                        addExtraLambert = true;
                    }
                    else {
                        baseColor[i] = 0.0f;
                        bssrdf_channel_weight += sssBaseColor[i];
                    }
                }
                
                const auto bssrdf_pdf = bssrdf_channel_weight / total_channel_weight;
                if (!mfp.IsBlack() && !sssBaseColor.IsBlack())
                    se.AddBssrdf(SORT_MALLOC(DisneyBssrdf)(&se.GetInteraction(), sssBaseColor, mfp, weight, pdf_weight * bssrdf_pdf ));

                if (addExtraLambert && !baseColor.IsBlack())
                    se.AddBxdf(SORT_MALLOC(Lambert)(baseColor, weight, pdf_weight * ( 1.0f - bssrdf_pdf ), params.n));
#else
                se.AddBssrdf(SORT_MALLOC(DisneyBssrdf)(&se.GetInteraction(), params.baseColor, params.scatterDistance, weight ));
#endif
            }else{
                se.AddBxdf(SORT_MALLOC(Lambert)(params.baseColor, weight , params.n));
            }
        }
    };

    struct Surface_Closure_Transparent : public Surface_Closure_Base {
        static constexpr int    ClosureID = SURFACE_CLOSURE_TRANSPARENT;

        static const char* GetName() {
            return "transparent";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(Transparent::Params, attenuation),
                CLOSURE_FINISH_PARAM(Transparent::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
            const auto& params = *comp->as<Transparent::Params>();
            se.AddBxdf(SORT_MALLOC(Transparent)(params, w * comp->w));
        }

        Spectrum EvaluateOpacity( const ClosureComponent* comp, const OSL::Color3& w ) const override{
            const auto& params = *comp->as<Transparent::Params>();
            return 1.0f - Spectrum(params.attenuation);
        }
    };

    struct Volume_Closure_Absorption : public Volume_Closure_Base {
        static constexpr int    ClosureID = VOLUME_CLOSURE_ABSORPTION;

        static const char* GetName() {
            return "medium_absorption";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(AbsorptionMedium::Params, baseColor),
                CLOSURE_FLOAT_PARAM(AbsorptionMedium::Params, absorption),
                CLOSURE_FINISH_PARAM(AbsorptionMedium::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material) const override{
            if (SE_ENTERING == flag) {
                const auto& params = *comp->as<AbsorptionMedium::Params>();
                ms.AddMedium(SORT_MALLOC(AbsorptionMedium)(params, material));
            } else if( SE_LEAVING == flag ){
                ms.RemoveMedium(material->GetUniqueID());
            }
        }
    };

    struct Volume_Closure_Homogeneous : public Volume_Closure_Base {
        static constexpr int    ClosureID = VOLUME_CLOSURE_HOMOGENEOUS;

        static const char* GetName() {
            return "medium_homogeneous";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(HomogeneousMedium::Params, baseColor),
                CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, emission),
                CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, absorption),
                CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, scattering),
                CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, anisotropy),
                CLOSURE_FINISH_PARAM(HomogeneousMedium::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material ) const override {
            if (SE_ENTERING == flag) {
                const auto& params = *comp->as<HomogeneousMedium::Params>();
                ms.AddMedium(SORT_MALLOC(HomogeneousMedium)(params, material));
            } else if (SE_LEAVING == flag) {
                ms.RemoveMedium(material->GetUniqueID());
            }
        }
    };

    struct Volume_Closure_Heterogeneous : public Volume_Closure_Base {
        static constexpr int    ClosureID = VOLUME_CLOSURE_HETEROGENOUS;

        static const char* GetName() {
            return "medium_heterogeneous";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_COLOR_PARAM(HeterogenousMedium::Params, baseColor),
                CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, emission),
                CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, absorption),
                CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, scattering),
                CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, anisotropy),
                CLOSURE_FINISH_PARAM(HeterogenousMedium::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material) const override {
            if (SE_ENTERING == flag)
                ms.AddMedium(SORT_MALLOC(HeterogenousMedium)(material));
            else if (SE_LEAVING == flag)
                ms.RemoveMedium(material->GetUniqueID());
        }

        void Evaluate(const ClosureComponent* comp, const OSL::Color3& w, MediumSample& ms) const override{
            const auto& params = *comp->as<HeterogenousMedium::Params>();
            ms.absorption = params.absorption;
            ms.scattering = params.scattering;
            ms.extinction = ms.absorption + ms.scattering;
            ms.anisotropy = params.anisotropy;
            ms.emission = params.emission;
            ms.basecolor = params.baseColor;
        }
    };
}

template< typename T >
static void registerSurfaceClosure(OSL::ShadingSystem* shadingsys) {
    T::Register(shadingsys);
    g_surface_closures[T::ClosureID] = std::make_unique<T>();
}

template< typename T >
static void registerVolumeClosure(OSL::ShadingSystem* shadingsys) {
    T::Register(shadingsys);
    g_volume_closures[T::ClosureID - VOLUME_CLOSURE_BASE] = std::make_unique<T>();
}

void RegisterClosures(OSL::ShadingSystem* shadingsys) {
    registerSurfaceClosure<Surface_Closure_Lambert>(shadingsys);
    registerSurfaceClosure<Surface_Closure_OrenNayar>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Disney>(shadingsys);
    registerSurfaceClosure<Surface_Closure_MicrofacetReflection>(shadingsys);
    registerSurfaceClosure<Surface_Closure_MicrofacetRefraction>(shadingsys);
    registerSurfaceClosure<Surface_Closure_AshikhmanShirley>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Phong>(shadingsys);
    registerSurfaceClosure<Surface_Closure_LambertTransmission>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Mirror>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Dielectric>(shadingsys);
    registerSurfaceClosure<Surface_Closure_MicrofacetReflectionDielectric>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Hair>(shadingsys);
    registerSurfaceClosure<Surface_Closure_MERL>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Coat>(shadingsys);
    registerSurfaceClosure<Surface_Closure_DoubleSided>(shadingsys);
    registerSurfaceClosure<Surface_Closure_FourierBRDF>(shadingsys);
    registerSurfaceClosure<Surface_Closure_DistributionBRDF>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Fabric>(shadingsys);
    registerSurfaceClosure<Surface_Closure_SSS>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Transparent>(shadingsys);

    registerVolumeClosure<Volume_Closure_Absorption>(shadingsys);
    registerVolumeClosure<Volume_Closure_Homogeneous>(shadingsys);
    registerVolumeClosure<Volume_Closure_Heterogeneous>(shadingsys);
}

void ProcessSurfaceClosure(const OSL::ClosureColor* closure, const OSL::Color3& w , ScatteringEvent& se ){
    if (!closure)
        return;

    switch (closure->id) {
        case ClosureColor::MUL: {
            Color3 cw = w * closure->as_mul()->weight;
            ProcessSurfaceClosure(closure->as_mul()->closure, cw , se);
            break;
        }
        case ClosureColor::ADD: {
            ProcessSurfaceClosure(closure->as_add()->closureA, w , se);
            ProcessSurfaceClosure(closure->as_add()->closureB, w , se);
            break;
        }
        default: {
            const ClosureComponent* comp = closure->as_comp();
            getSurfaceClosureBase(comp->id)->Process(comp, w * comp->w , se);
        }
    }
}

void ProcessVolumeClosure(const OSL::ClosureColor* closure, const OSL::Color3& w, MediumStack& mediumStack, const SE_Interaction flag, const MaterialBase* material) {
    if (!closure)
        return;

    switch (closure->id) {
        case ClosureColor::MUL:
        case ClosureColor::ADD:
            // no support for blending or addding volume for now.
            sAssert(false, VOLUME);
            break;
        default: {
            const ClosureComponent* comp = closure->as_comp();
            getVolumeClosureBase(comp->id)->Process(comp, w * comp->w, mediumStack, flag, material);
        }
    }
}

void ProcessVolumeClosure(const OSL::ClosureColor* closure, const OSL::Color3& w, MediumSample& ms){
    if (!closure)
        return;

    switch (closure->id) {
        case ClosureColor::MUL:
        case ClosureColor::ADD:
            // no support for blending or addding volume for now.
            sAssert(false, VOLUME);
            break;
        default: {
            const ClosureComponent* comp = closure->as_comp();
            getVolumeClosureBase(comp->id)->Evaluate(comp, w * comp->w, ms);
        }
    }
}

Spectrum ProcessOpacity(const OSL::ClosureColor* closure, const OSL::Color3& w ){
    if (!closure)
        return 0.0f;

    Spectrum occlusion = 0.0f;
    switch (closure->id) {
        case ClosureColor::MUL: {
            const auto cw = w * closure->as_mul()->weight;
            occlusion += ProcessOpacity(closure->as_mul()->closure, cw );
            break;
        }
        case ClosureColor::ADD: {
            occlusion += ProcessOpacity(closure->as_add()->closureA, w );
            occlusion += ProcessOpacity(closure->as_add()->closureB, w );
            break;
        }
        default: {
            const ClosureComponent* comp = closure->as_comp();
            occlusion += getSurfaceClosureBase(comp->id)->EvaluateOpacity(comp, w * comp->w);
        }
    }
    return occlusion;
}