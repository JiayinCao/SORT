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
        virtual void Process(const ClosureComponent* comp, const OSL::Color3& w , ScatteringEvent& se ) const = 0;

        virtual Spectrum EvaluateOpacity( const ClosureComponent* comp, const OSL::Color3& w ) const{
            return w;
        }
    };

    static std::vector<std::unique_ptr<Closure_Base>>   g_closures(CLOSURE_CNT);

    struct Closure_Lambert : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_LAMBERT;

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

    struct Closure_OrenNayar : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_OREN_NAYAR;

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

    struct Closure_Disney : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_DISNEY;

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
                const auto bxdf_sampling_weight = DisneyBRDF::Evaluate_PDF( params );

                if( bxdf_sampling_weight > 0.0f )
                    se.AddBxdf(SORT_MALLOC(DisneyBRDF)(params, weight, bxdf_sampling_weight * sample_weight));

                const auto diffuseWeight = (Spectrum)( weight * (1.0f - params.metallic) * (1.0 - params.specTrans) );
                if (!sssBaseColor.IsBlack() && bxdf_sampling_weight < 1.0f && !diffuseWeight.IsBlack() )
                    se.AddBssrdf( SORT_MALLOC(DisneyBssrdf)(&se.GetIntersection(), sssBaseColor, params.scatterDistance, diffuseWeight , ( 1.0f - bxdf_sampling_weight ) * sample_weight * bssrdf_pdf ) );

#ifdef SSS_REPLACE_WITH_LAMBERT
                if (addExtraLambert && !baseColor.IsBlack())
                    se.AddBxdf(SORT_MALLOC(Lambert)(baseColor, diffuseWeight, ( 1.0f - bxdf_sampling_weight ) * sample_weight * ( 1.0f - bssrdf_pdf ) , params.n));
#endif
            }else{
                se.AddBxdf(SORT_MALLOC(DisneyBRDF)(params, weight));
            }
        }
    };

    struct Closure_MicrofacetReflection : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_MICROFACET_REFLECTION;

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

    struct Closure_MicrofacetRefraction : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_MICROFACET_REFRACTION;

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

    struct Closure_AshikhmanShirley : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_ASHIKHMANSHIRLEY;

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

    struct Closure_Phong : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_PHONG;

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

    struct Closure_LambertTransmission : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_LAMBERT_TRANSMITTANCE;

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

    struct Closure_Mirror : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_MIRROR;

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

    struct Closure_Dielectric : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_DIELETRIC;

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

    struct Closure_MicrofacetReflectionDielectric : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_MICROFACET_REFLECTION_DIELETRIC;

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

    struct Closure_Hair : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_HAIR;

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

    struct Closure_FourierBRDF : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_FOURIER_BDRF;

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

    struct Closure_MERL : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_MERL_BRDF;

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

    struct Closure_Coat : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_COAT;

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
            ScatteringEvent* bottom = SORT_MALLOC(ScatteringEvent)(se.GetIntersection(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
            ProcessClosure(params.closure, Color3(1.0f), *bottom);
            se.AddBxdf(SORT_MALLOC(Coat)(params, w, bottom));
        }
    };

    struct Closure_DoubleSided : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_DOUBLESIDED;

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
            ScatteringEvent* se0 = SORT_MALLOC(ScatteringEvent)(se.GetIntersection(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
            ScatteringEvent* se1 = SORT_MALLOC(ScatteringEvent)(se.GetIntersection(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
            ProcessClosure(params.bxdf0, Color3(1.0f), *se0);
            ProcessClosure(params.bxdf1, Color3(1.0f), *se1);
            se.AddBxdf(SORT_MALLOC(DoubleSided)(se0, se1, w));
        }
    };

    struct Closure_DistributionBRDF : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_DISTRIBUTIONBRDF;

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

    struct Closure_Fabric : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_FABRIC;

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

    struct Closure_SSS : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_SSS;

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
                    se.AddBssrdf(SORT_MALLOC(DisneyBssrdf)(&se.GetIntersection(), sssBaseColor, mfp, weight, pdf_weight * bssrdf_pdf ));

                if (addExtraLambert && !baseColor.IsBlack())
                    se.AddBxdf(SORT_MALLOC(Lambert)(baseColor, weight, pdf_weight * ( 1.0f - bssrdf_pdf ), params.n));
#else
                se.AddBssrdf(SORT_MALLOC(DisneyBssrdf)(&se.GetIntersection(), params.baseColor, params.scatterDistance, weight ));
#endif
            }else{
                se.AddBxdf(SORT_MALLOC(Lambert)(params.baseColor, weight , params.n));
            }
        }
    };

    struct Closure_Transparent : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_TRANSPARENT;

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
            return 0.0f;
        }
    };
}

template< typename T >
void registerClosure(OSL::ShadingSystem* shadingsys) {
    T::Register(shadingsys);

    // Unit test will register a closure multiple times
    // sAssertMsg(g_closures[T::ClosureID] == nullptr, MATERIAL, "Closure slot duplicated!");

    g_closures[T::ClosureID] = std::make_unique<T>();
}

void RegisterClosures(OSL::ShadingSystem* shadingsys) {
    registerClosure<Closure_Lambert>(shadingsys);
    registerClosure<Closure_OrenNayar>(shadingsys);
    registerClosure<Closure_Disney>(shadingsys);
    registerClosure<Closure_MicrofacetReflection>(shadingsys);
    registerClosure<Closure_MicrofacetRefraction>(shadingsys);
    registerClosure<Closure_AshikhmanShirley>(shadingsys);
    registerClosure<Closure_Phong>(shadingsys);
    registerClosure<Closure_LambertTransmission>(shadingsys);
    registerClosure<Closure_Mirror>(shadingsys);
    registerClosure<Closure_Dielectric>(shadingsys);
    registerClosure<Closure_MicrofacetReflectionDielectric>(shadingsys);
    registerClosure<Closure_Hair>(shadingsys);
    registerClosure<Closure_MERL>(shadingsys);
    registerClosure<Closure_Coat>(shadingsys);
    registerClosure<Closure_DoubleSided>(shadingsys);
    registerClosure<Closure_FourierBRDF>(shadingsys);
    registerClosure<Closure_DistributionBRDF>(shadingsys);
    registerClosure<Closure_Fabric>(shadingsys);
    registerClosure<Closure_SSS>(shadingsys);
    registerClosure<Closure_Transparent>(shadingsys);
}

void ProcessClosure(const OSL::ClosureColor* closure, const OSL::Color3& w , ScatteringEvent& se ){
    if (!closure)
        return;

    switch (closure->id) {
        case ClosureColor::MUL: {
            Color3 cw = w * closure->as_mul()->weight;
            ProcessClosure(closure->as_mul()->closure, cw , se);
            break;
        }
        case ClosureColor::ADD: {
            ProcessClosure(closure->as_add()->closureA, w , se);
            ProcessClosure(closure->as_add()->closureB, w , se);
            break;
        }
        default: {
            const ClosureComponent* comp = closure->as_comp();
            sAssert(comp->id >= 0 && comp->id < CLOSURE_CNT, MATERIAL);
            sAssert(g_closures[comp->id] != nullptr, MATERIAL);
            g_closures[comp->id]->Process(comp, w * comp->w , se);
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
            sAssert(comp->id >= 0 && comp->id < CLOSURE_CNT, MATERIAL);
            sAssert(g_closures[comp->id] != nullptr, MATERIAL);
            occlusion += g_closures[comp->id]->EvaluateOpacity(comp, w * comp->w);
        }
    }
    return occlusion;
}