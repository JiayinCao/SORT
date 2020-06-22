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

#include "closures.h"
#include "osl_utils.h"
#include "core/memory.h"
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
#include "core/mesh.h"

using namespace Tsl_Namespace;

SORT_FORCEINLINE bool is_tsl_color_black(float3 color) {
    return color.x == 0.0f && color.y == 0.0f && color.y == 0.0f;
}

// it is always assumed the value is valid
SORT_FORCEINLINE float& tsl_color_channel(float3 color, int i) {
    return (i == 0) ? color.x : ((i == 1) ? color.y : color.z);
}

#define DEFINE_CLOSUREID(T)  ClosureID T::closure_id = INVALID_CLOSURE_ID

// These data structure is not supposed to be seen by other parts of the renderer
namespace {
     constexpr unsigned int MAX_CLOSURE_CNT = 128;

     struct Closure_Base {
         struct BuiltinClosures {
             const char* name;
             int id;
         };

         Closure_Base() = default;
         virtual ~Closure_Base() = default;
     };

     struct Surface_Closure_Base : public Closure_Base {
         /*
         virtual Spectrum EvaluateOpacity(const ClosureComponent* comp, const OSL::Color3& w) const {
             return w;
         }
         */

         virtual void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const = 0;
     };

//     struct Volume_Closure_Base : public Closure_Base {
//         virtual void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material, const Mesh* mesh) const = 0;
//         virtual void Evaluate(const ClosureComponent* comp, const OSL::Color3& w, MediumSample& ms) const {}
//     };

     static std::vector<std::unique_ptr<Surface_Closure_Base>>   g_surface_closures(MAX_CLOSURE_CNT);
//     static std::vector<std::unique_ptr<Volume_Closure_Base>>    g_volume_closures(VOLUME_CLOSURE_CNT);

     inline static Surface_Closure_Base* getSurfaceClosureBase(unsigned int closure_id) {
         sAssert(closure_id >= 0 && closure_id < MAX_CLOSURE_CNT, MATERIAL);
         sAssert(IS_PTR_VALID(g_surface_closures[closure_id]), MATERIAL);

         return g_surface_closures[closure_id].get();
     }

//     inline static Volume_Closure_Base* getVolumeClosureBase(unsigned int closure_id) {
//         closure_id -= VOLUME_CLOSURE_BASE;

//         sAssert(closure_id >= 0 && closure_id < VOLUME_CLOSURE_CNT, VOLUME);
//         sAssert(IS_PTR_VALID(g_volume_closures[closure_id]), VOLUME);

//         return g_volume_closures[closure_id].get();
//     }

     struct Surface_Closure_Lambert : public Surface_Closure_Base {
         static ClosureID    closure_id;

         static const char* GetName(){
             return "lambert";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeLambert::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             ClosureTypeLambert* bxdf_param = (ClosureTypeLambert*)param;
             se.AddBxdf(SORT_MALLOC(Lambert)(*bxdf_param, w));
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_Lambert);

     struct Surface_Closure_OrenNayar : public Surface_Closure_Base {
         static ClosureID    closure_id;

         static const char* GetName(){
             return "oren_nayar";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeOrenNayar::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             ClosureTypeOrenNayar* bxdf_param = (ClosureTypeOrenNayar*)param;
             se.AddBxdf(SORT_MALLOC(OrenNayar)(*bxdf_param, w));
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_OrenNayar);

     struct Surface_Closure_Disney : public Surface_Closure_Base {
         static ClosureID    closure_id;

         static const char* GetName(){
             return "disney";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeDisney::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto weight = w;
             const auto sample_weight = ( weight.x + weight.y + weight.z ) / 3.0f;
             auto& params = *(ClosureTypeDisney*)param;
             auto& mfp = params.scatterDistance;

             // Ignore SSS if necessary
             if (SE_NONE != (se.GetFlag() & SE_REPLACE_BSSRDF))
                 mfp = Tsl_Namespace::make_float3(0.0f, 0.0f, 0.0f);

             RGBSpectrum sssBaseColor = params.baseColor;

 #ifdef SSS_REPLACE_WITH_LAMBERT
             constexpr float delta = 0.0001f;
             auto bssrdf_channel_weight = 0.0f;
             auto total_channel_weight = 0.0f;
             auto addExtraLambert = false;
             auto baseColor = params.baseColor;
             for (int i = 0; i < SPECTRUM_SAMPLE; ++i) {
                 auto& _baseColor = tsl_color_channel(baseColor, i);
                 auto& _mfp = tsl_color_channel(mfp, i);

                 total_channel_weight += _baseColor;

                 // If the reflectance is zero or the mean free path is too small, switch back to lambert.
                 if (_baseColor == 0.0f) {
                     _mfp = 0.0f;
                     continue;
                 }

                 // if the mean free distance is too small, replace it with lambert.
                 if (_mfp < delta) {
                     _mfp = 0.0f;
                     sssBaseColor[i] = 0.0f;
                     addExtraLambert = true;
                 }
                 else {
                     _baseColor = 0.0f;
                     bssrdf_channel_weight += sssBaseColor[i];
                 }
             }

             const auto bssrdf_pdf = bssrdf_channel_weight / total_channel_weight;
 #else
             constexpr auto bssrdf_pdf = 1.0f;
 #endif
             if (!( 0.0f == mfp.x && 0.0f == mfp.y && 0.0f == mfp.z )) {
                 const auto bxdf_sampling_weight = DisneyBRDF::Evaluate_Sampling_Weight( params );

                 if( bxdf_sampling_weight > 0.0f )
                     se.AddBxdf(SORT_MALLOC(DisneyBRDF)(params, weight, bxdf_sampling_weight * sample_weight));

                 const auto diffuseWeight = (Spectrum)( weight * (1.0f - params.metallic) * (1.0 - params.specTrans) );
                 if (!sssBaseColor.IsBlack() && bxdf_sampling_weight < 1.0f && !diffuseWeight.IsBlack() )
                     se.AddBssrdf( SORT_MALLOC(DisneyBssrdf)(&se.GetInteraction(), sssBaseColor, params.scatterDistance, diffuseWeight , ( 1.0f - bxdf_sampling_weight ) * sample_weight * bssrdf_pdf ) );

 #ifdef SSS_REPLACE_WITH_LAMBERT
                 if (addExtraLambert && !is_tsl_color_black(baseColor))
                     se.AddBxdf(SORT_MALLOC(Lambert)(baseColor, diffuseWeight, ( 1.0f - bxdf_sampling_weight ) * sample_weight * ( 1.0f - bssrdf_pdf ) , params.normal));
 #endif
             }else{
                 se.AddBxdf(SORT_MALLOC(DisneyBRDF)(params, weight, sample_weight));
             }
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_Disney);

//     struct Surface_Closure_MicrofacetReflection : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_MICROFACET_REFLECTION;

//         static const char* GetName(){
//             return "microfacetReflection";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_STRING_PARAM(MicroFacetReflection::Params, dist),
//                 CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, eta),
//                 CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, absorption),
//                 CLOSURE_FLOAT_PARAM(MicroFacetReflection::Params, roughnessU),
//                 CLOSURE_FLOAT_PARAM(MicroFacetReflection::Params, roughnessV),
//                 CLOSURE_COLOR_PARAM(MicroFacetReflection::Params, baseColor),
//                 CLOSURE_VECTOR_PARAM(MicroFacetReflection::Params, n),
//                 CLOSURE_FINISH_PARAM(MicroFacetReflection::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<MicroFacetReflection::Params>();
//             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
//         }
//     };

//     struct Surface_Closure_MicrofacetRefraction : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_MICROFACET_REFRACTION;

//         static const char* GetName(){
//             return "microfacetRefraction";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_STRING_PARAM(MicroFacetRefraction::Params, dist),
//                 CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, etaI),
//                 CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, etaT),
//                 CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, roughnessU),
//                 CLOSURE_FLOAT_PARAM(MicroFacetRefraction::Params, roughnessV),
//                 CLOSURE_COLOR_PARAM(MicroFacetRefraction::Params, transmittance),
//                 CLOSURE_VECTOR_PARAM(MicroFacetRefraction::Params, n),
//                 CLOSURE_FINISH_PARAM(MicroFacetRefraction::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<MicroFacetRefraction::Params>();
//             se.AddBxdf(SORT_MALLOC(MicroFacetRefraction)(params, w * comp->w));
//         }
//     };

     struct Surface_Closure_AshikhmanShirley : public Surface_Closure_Base {
         static ClosureID closure_id;

         static const char* GetName(){
             return "ashikhman_shirley";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeAshikhmanShirley::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(ClosureTypeAshikhmanShirley*)param;
             se.AddBxdf(SORT_MALLOC(AshikhmanShirley)(params, w));
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_AshikhmanShirley);

//     struct Surface_Closure_Phong : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_PHONG;

//         static const char* GetName(){
//             return "phong";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_COLOR_PARAM(Phong::Params, diffuse),
//                 CLOSURE_COLOR_PARAM(Phong::Params, specular),
//                 CLOSURE_FLOAT_PARAM(Phong::Params, specularPower),
//                 CLOSURE_VECTOR_PARAM(Phong::Params, n),
//                 CLOSURE_FINISH_PARAM(Phong::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<Phong::Params>();
//             se.AddBxdf(SORT_MALLOC(Phong)(params, w * comp->w));
//         }
//     };

     struct Surface_Closure_LambertTransmission : public Surface_Closure_Base {
         static ClosureID    closure_id;

         static const char* GetName(){
             return "lambert_transmission";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeLambertTransmission::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             ClosureTypeLambertTransmission* bxdf_param = (ClosureTypeLambertTransmission*)param;
             se.AddBxdf(SORT_MALLOC(LambertTransmission)(*bxdf_param, w));
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_LambertTransmission);

     struct Surface_Closure_Mirror : public Surface_Closure_Base {
         static ClosureID    closure_id;

         static const char* GetName(){
             return "mirror";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeMirror::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             ClosureTypeMirror* bxdf_param = (ClosureTypeMirror*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(*bxdf_param, w));
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_Mirror);

     struct Surface_Closure_Dielectric : public Surface_Closure_Base {
         static ClosureID    closure_id;

         static const char* GetName(){
             return "dieletric";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeDielectric::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             ClosureTypeDielectric* bxdf_param = (ClosureTypeDielectric*)param;
             se.AddBxdf(SORT_MALLOC(Dielectric)(*bxdf_param, w));
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_Dielectric);

//     struct Surface_Closure_MicrofacetReflectionDielectric : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_MICROFACET_REFLECTION_DIELETRIC;

//         static const char* GetName(){
//             return "microfacetReflectionDieletric";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_STRING_PARAM(MicroFacetReflection::ParamsDieletric, dist),
//                 CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, iorI),
//                 CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, iorT),
//                 CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, roughnessU),
//                 CLOSURE_FLOAT_PARAM(MicroFacetReflection::ParamsDieletric, roughnessV),
//                 CLOSURE_COLOR_PARAM(MicroFacetReflection::ParamsDieletric, baseColor),
//                 CLOSURE_VECTOR_PARAM(MicroFacetReflection::ParamsDieletric, n),
//                 CLOSURE_FINISH_PARAM(MicroFacetReflection::ParamsDieletric)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<MicroFacetReflection::ParamsDieletric>();
//             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
//         }
//     };

//     struct Surface_Closure_Hair : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_HAIR;

//         static const char* GetName(){
//             return "hair";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_COLOR_PARAM(Hair::Params, sigma),
//                 CLOSURE_FLOAT_PARAM(Hair::Params, longtitudinalRoughness),
//                 CLOSURE_FLOAT_PARAM(Hair::Params, azimuthalRoughness),
//                 CLOSURE_FLOAT_PARAM(Hair::Params, ior),
//                 CLOSURE_FINISH_PARAM(Hair::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<Hair::Params>();
//             se.AddBxdf(SORT_MALLOC(Hair)(params, w * comp->w));
//         }
//     };

//     struct Surface_Closure_FourierBRDF : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_FOURIER_BDRF;

//         static const char* GetName(){
//             return "fourierBRDF";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_INT_PARAM(FourierBxdf::Params, resIdx),
//                 CLOSURE_VECTOR_PARAM(FourierBxdf::Params, n),
//                 CLOSURE_FINISH_PARAM(FourierBxdf::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<FourierBxdf::Params>();
//             se.AddBxdf(SORT_MALLOC(FourierBxdf)(params, w * comp->w));
//         }
//     };

//     struct Surface_Closure_MERL : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_MERL_BRDF;

//         static const char* GetName(){
//             return "merlBRDF";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_INT_PARAM(Merl::Params, resIdx),
//                 CLOSURE_VECTOR_PARAM(Merl::Params, n),
//                 CLOSURE_FINISH_PARAM(Merl::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<Merl::Params>();
//             se.AddBxdf(SORT_MALLOC(Merl)(params, w * comp->w));
//         }
//     };

//     struct Surface_Closure_Coat : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_COAT;

//         static const char* GetName(){
//             return "coat";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_CLOSURE_PARAM(Coat::Params, closure),
//                 CLOSURE_FLOAT_PARAM(Coat::Params, roughness),
//                 CLOSURE_FLOAT_PARAM(Coat::Params, ior),
//                 CLOSURE_COLOR_PARAM(Coat::Params, sigma),
//                 CLOSURE_VECTOR_PARAM(Coat::Params, n),
//                 CLOSURE_FINISH_PARAM(Coat::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<Coat::Params>();
//             ScatteringEvent* bottom = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
//             ProcessSurfaceClosure(params.closure, Color3(1.0f), *bottom);
//             se.AddBxdf(SORT_MALLOC(Coat)(params, w, bottom));
//         }
//     };

     struct Surface_Closure_DoubleSided : public Surface_Closure_Base {
         static ClosureID closure_id;

         static const char* GetName(){
             return "double_sided";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeDoubleSided::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3 & w, ScatteringEvent & se) const override {
             const auto& params = *(const ClosureTypeDoubleSided*)param;
             ScatteringEvent* se0 = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
             ScatteringEvent* se1 = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
             ProcessSurfaceClosure((const ClosureTreeNodeBase*)params.closure0, make_float3(1.0f, 1.0f, 1.0f), *se0);
             ProcessSurfaceClosure((const ClosureTreeNodeBase*)params.closure1, make_float3(1.0f, 1.0f, 1.0f), *se1);
             se.AddBxdf(SORT_MALLOC(DoubleSided)(se0, se1, w));
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_DoubleSided);

//     struct Surface_Closure_DistributionBRDF : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_DISTRIBUTIONBRDF;

//         static const char* GetName(){
//             return "distributionBRDF";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID, {
//                 CLOSURE_COLOR_PARAM(DistributionBRDF::Params, baseColor),
//                 CLOSURE_FLOAT_PARAM(DistributionBRDF::Params, roughness),
//                 CLOSURE_FLOAT_PARAM(DistributionBRDF::Params, specular),
//                 CLOSURE_FLOAT_PARAM(DistributionBRDF::Params, specularTint),
//                 CLOSURE_VECTOR_PARAM(DistributionBRDF::Params, n),
//                 CLOSURE_FINISH_PARAM(DistributionBRDF::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<DistributionBRDF::Params>();
//             se.AddBxdf(SORT_MALLOC(DistributionBRDF)(params, w * comp->w));
//         }
//     };

//     struct Surface_Closure_Fabric : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_FABRIC;

//         static const char* GetName() {
//             return "fabric";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_COLOR_PARAM(Fabric::Params, baseColor),
//                 CLOSURE_FLOAT_PARAM(Fabric::Params, roughness),
//                 CLOSURE_VECTOR_PARAM(Fabric::Params, n),
//                 CLOSURE_FINISH_PARAM(Fabric::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<Fabric::Params>();
//             se.AddBxdf(SORT_MALLOC(Fabric)(params, w * comp->w));
//         }
//     };

     struct Surface_Closure_SSS : public Surface_Closure_Base {
         static ClosureID closure_id;

         static const char* GetName() {
             return "subsurface_scattering";
         }

         static void Register(ShadingSystem* shadingsys) {
             closure_id = ClosureTypeSSS::RegisterClosure(GetName(), *shadingsys);
         }

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3 & w, ScatteringEvent & se) const override {
             const auto& params = *(ClosureTypeSSS*)param;
             if(is_tsl_color_black(params.base_color))
                 return;

             const auto weight = w;

             if (SE_NONE == (se.GetFlag() & SE_REPLACE_BSSRDF)){
 #ifdef SSS_REPLACE_WITH_LAMBERT
                 auto sssBaseColor = params.base_color;
                 const auto pdf_weight = (weight.x + weight.y + weight.z) / 3.0f;

                 constexpr float delta = 0.0001f;
                 auto bssrdf_channel_weight = 0.0f;
                 auto total_channel_weight = 0.0f;
                 auto mfp = params.scatter_distance;
                 auto addExtraLambert = false;
                 auto baseColor = params.base_color;
                 for (int i = 0; i < SPECTRUM_SAMPLE; ++i) {
                     auto& base_color_channel_i = tsl_color_channel(baseColor, i);
                     auto& sss_base_color_channel_i = tsl_color_channel(sssBaseColor, i);
                     auto& mfp_channel_i = tsl_color_channel(mfp, i);
                     total_channel_weight += sss_base_color_channel_i;

                     // If the reflectance is zero or the mean free path is too small, switch back to lambert.
                     if (sss_base_color_channel_i == 0.0f) {
                         mfp_channel_i = 0.0f;
                         continue;
                     }

                     // if the mean free distance is too small, replace it with lambert.
                     if (mfp_channel_i < delta) {
                         mfp_channel_i = 0.0f;
                         sss_base_color_channel_i = 0.0f;
                         addExtraLambert = true;
                     }
                     else {
                         base_color_channel_i = 0.0f;
                         bssrdf_channel_weight += sss_base_color_channel_i;
                     }
                 }

                 const auto bssrdf_pdf = bssrdf_channel_weight / total_channel_weight;
                 if (!is_tsl_color_black(mfp) && !is_tsl_color_black(sssBaseColor))
                     se.AddBssrdf(SORT_MALLOC(DisneyBssrdf)(&se.GetInteraction(), sssBaseColor, mfp, weight, pdf_weight * bssrdf_pdf ));

                 if (addExtraLambert && !is_tsl_color_black(baseColor))
                     se.AddBxdf(SORT_MALLOC(Lambert)(baseColor, weight, pdf_weight * ( 1.0f - bssrdf_pdf ), params.normal));
 #else
                 se.AddBssrdf(SORT_MALLOC(DisneyBssrdf)(&se.GetInteraction(), params, weight ));
 #endif
             }else{
                 se.AddBxdf(SORT_MALLOC(Lambert)(params.base_color, weight , params.normal));
             }
         }
     };
     DEFINE_CLOSUREID(Surface_Closure_SSS);

//     struct Surface_Closure_Transparent : public Surface_Closure_Base {
//         static constexpr int    ClosureID = SURFACE_CLOSURE_TRANSPARENT;

//         static const char* GetName() {
//             return "transparent";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_COLOR_PARAM(Transparent::Params, attenuation),
//                 CLOSURE_FINISH_PARAM(Transparent::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, ScatteringEvent& se) const override {
//             const auto& params = *comp->as<Transparent::Params>();
//             se.AddBxdf(SORT_MALLOC(Transparent)(params, w * comp->w));
//         }

//         Spectrum EvaluateOpacity( const ClosureComponent* comp, const OSL::Color3& w ) const override{
//             const auto& params = *comp->as<Transparent::Params>();
//             return 1.0f - Spectrum(params.attenuation);
//         }
//     };

//     struct Volume_Closure_Absorption : public Volume_Closure_Base {
//         static constexpr int    ClosureID = VOLUME_CLOSURE_ABSORPTION;

//         static const char* GetName() {
//             return "medium_absorption";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_COLOR_PARAM(AbsorptionMedium::Params, baseColor),
//                 CLOSURE_FLOAT_PARAM(AbsorptionMedium::Params, absorption),
//                 CLOSURE_FINISH_PARAM(AbsorptionMedium::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material, const Mesh* mesh) const override{
//             if (SE_ENTERING == flag) {
//                 const auto& params = *comp->as<AbsorptionMedium::Params>();
//                 ms.AddMedium(SORT_MALLOC(AbsorptionMedium)(params, material));
//             } else if( SE_LEAVING == flag ){
//                 ms.RemoveMedium(material->GetUniqueID());
//             }
//         }
//     };

//     struct Volume_Closure_Homogeneous : public Volume_Closure_Base {
//         static constexpr int    ClosureID = VOLUME_CLOSURE_HOMOGENEOUS;

//         static const char* GetName() {
//             return "medium_homogeneous";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_COLOR_PARAM(HomogeneousMedium::Params, baseColor),
//                 CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, emission),
//                 CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, absorption),
//                 CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, scattering),
//                 CLOSURE_FLOAT_PARAM(HomogeneousMedium::Params, anisotropy),
//                 CLOSURE_FINISH_PARAM(HomogeneousMedium::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material, const Mesh* mesh) const override {
//             if (SE_ENTERING == flag) {
//                 const auto& params = *comp->as<HomogeneousMedium::Params>();
//                 ms.AddMedium(SORT_MALLOC(HomogeneousMedium)(params, material));
//             } else if (SE_LEAVING == flag) {
//                 ms.RemoveMedium(material->GetUniqueID());
//             }
//         }
//     };

//     struct Volume_Closure_Heterogeneous : public Volume_Closure_Base {
//         static constexpr int    ClosureID = VOLUME_CLOSURE_HETEROGENOUS;

//         static const char* GetName() {
//             return "medium_heterogeneous";
//         }

//         static void Register(ShadingSystem* shadingsys) {
//             BuiltinClosures closure = { GetName(), ClosureID,{
//                 CLOSURE_COLOR_PARAM(HeterogenousMedium::Params, baseColor),
//                 CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, emission),
//                 CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, absorption),
//                 CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, scattering),
//                 CLOSURE_FLOAT_PARAM(HeterogenousMedium::Params, anisotropy),
//                 CLOSURE_FINISH_PARAM(HeterogenousMedium::Params)
//             } };
//             shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
//         }

//         void Process(const ClosureComponent* comp, const OSL::Color3& w, MediumStack& ms, const SE_Interaction flag, const MaterialBase* material, const Mesh* mesh) const override {
//             if (SE_ENTERING == flag)
//                 ms.AddMedium(SORT_MALLOC(HeterogenousMedium)(material, mesh));
//             else if (SE_LEAVING == flag)
//                 ms.RemoveMedium(material->GetUniqueID());
//         }

//         void Evaluate(const ClosureComponent* comp, const OSL::Color3& w, MediumSample& ms) const override{
//             const auto& params = *comp->as<HeterogenousMedium::Params>();
//             ms.absorption = fmax(0.0f, params.absorption);
//             ms.scattering = fmax(0.0f, params.scattering);
//             ms.extinction = fmax(0.0f, ms.absorption + ms.scattering);
//             ms.anisotropy = params.anisotropy;
//             ms.emission = fmax(0.0f, params.emission);
//             ms.basecolor = params.baseColor;
//         }
//     };
}

 template< typename T >
 static void registerSurfaceClosure(Tsl_Namespace::ShadingSystem* shadingsys) {
     T::Register(shadingsys);
     g_surface_closures[T::closure_id] = std::make_unique<T>();
 }

// template< typename T >
// static void registerVolumeClosure(OSL::ShadingSystem* shadingsys) {
//     T::Register(shadingsys);
//     g_volume_closures[T::ClosureID - VOLUME_CLOSURE_BASE] = std::make_unique<T>();
// }

void RegisterClosures(Tsl_Namespace::ShadingSystem* shadingsys) {
    registerSurfaceClosure<Surface_Closure_Lambert>(shadingsys);
    registerSurfaceClosure<Surface_Closure_OrenNayar>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Disney>(shadingsys);
    registerSurfaceClosure<Surface_Closure_LambertTransmission>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Mirror>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Dielectric>(shadingsys);
    registerSurfaceClosure<Surface_Closure_AshikhmanShirley>(shadingsys);
    registerSurfaceClosure<Surface_Closure_SSS>(shadingsys);
    registerSurfaceClosure<Surface_Closure_DoubleSided>(shadingsys);
    /*registerSurfaceClosure<Surface_Closure_MicrofacetReflection>(shadingsys);
    registerSurfaceClosure<Surface_Closure_MicrofacetRefraction>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Phong>(shadingsys);
    registerSurfaceClosure<Surface_Closure_MicrofacetReflectionDielectric>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Hair>(shadingsys);
    registerSurfaceClosure<Surface_Closure_MERL>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Coat>(shadingsys);
    registerSurfaceClosure<Surface_Closure_FourierBRDF>(shadingsys);
    registerSurfaceClosure<Surface_Closure_DistributionBRDF>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Fabric>(shadingsys);
    registerSurfaceClosure<Surface_Closure_Transparent>(shadingsys);

    registerVolumeClosure<Volume_Closure_Absorption>(shadingsys);
    registerVolumeClosure<Volume_Closure_Homogeneous>(shadingsys);
    registerVolumeClosure<Volume_Closure_Heterogeneous>(shadingsys);*/
}

void ProcessSurfaceClosure(const ClosureTreeNodeBase* closure, const float3& w, ScatteringEvent& se) {
    if (!closure)
        return;

    switch (closure->m_id) {
        case Tsl_Namespace::CLOSURE_ADD:
            {
                const ClosureTreeNodeAdd* closure_add = (const ClosureTreeNodeAdd*)closure;
                ProcessSurfaceClosure(closure_add->m_closure0, w, se);
                ProcessSurfaceClosure(closure_add->m_closure1, w, se);
            }
            break;
        case Tsl_Namespace::CLOSURE_MUL:
            {
                const ClosureTreeNodeMul* closure_mul = (const ClosureTreeNodeMul*)closure;
                const float3 weight = make_float3(w.x * closure_mul->m_weight, w.y * closure_mul->m_weight, w.z * closure_mul->m_weight);
                ProcessSurfaceClosure(closure_mul->m_closure, weight, se);
            }
            break;
        default:
            auto closure_base = getSurfaceClosureBase(closure->m_id);
            closure_base->Process(closure->m_params, w, se);
            break;
    }
}

 //void ProcessSurfaceClosure(const OSL::ClosureColor* closure, const OSL::Color3& w , ScatteringEvent& se ){
 //    if (!closure)
 //        return;

 //    switch (closure->id) {
 //        case ClosureColor::MUL: {
 //            Color3 cw = w * closure->as_mul()->weight;
 //            ProcessSurfaceClosure(closure->as_mul()->closure, cw , se);
 //            break;
 //        }
 //        case ClosureColor::ADD: {
 //            ProcessSurfaceClosure(closure->as_add()->closureA, w , se);
 //            ProcessSurfaceClosure(closure->as_add()->closureB, w , se);
 //            break;
 //        }
 //        default: {
 //            const ClosureComponent* comp = closure->as_comp();
 //            getSurfaceClosureBase(comp->id)->Process(comp, w * comp->w , se);
 //        }
 //    }
 //}

// void ProcessVolumeClosure(const OSL::ClosureColor* closure, const OSL::Color3& w, MediumStack& mediumStack, const SE_Interaction flag, const MaterialBase* material, const Mesh* mesh) {
//     if (!closure)
//         return;

//     switch (closure->id) {
//         case ClosureColor::MUL:
//         case ClosureColor::ADD:
//             // no support for blending or addding volume for now.
//             sAssert(false, VOLUME);
//             break;
//         default: {
//             const ClosureComponent* comp = closure->as_comp();
//             getVolumeClosureBase(comp->id)->Process(comp, w * comp->w, mediumStack, flag, material, mesh);
//         }
//     }
// }

// void ProcessVolumeClosure(const OSL::ClosureColor* closure, const OSL::Color3& w, MediumSample& ms){
//     if (!closure)
//         return;

//     switch (closure->id) {
//         case ClosureColor::MUL:
//         case ClosureColor::ADD:
//             // no support for blending or addding volume for now.
//             sAssert(false, VOLUME);
//             break;
//         default: {
//             const ClosureComponent* comp = closure->as_comp();
//             getVolumeClosureBase(comp->id)->Evaluate(comp, w * comp->w, ms);
//         }
//     }
// }

// Spectrum ProcessOpacity(const OSL::ClosureColor* closure, const OSL::Color3& w ){
//     if (!closure)
//         return 0.0f;

//     Spectrum occlusion = 0.0f;
//     switch (closure->id) {
//         case ClosureColor::MUL: {
//             const auto cw = w * closure->as_mul()->weight;
//             occlusion += ProcessOpacity(closure->as_mul()->closure, cw );
//             break;
//         }
//         case ClosureColor::ADD: {
//             occlusion += ProcessOpacity(closure->as_add()->closureA, w );
//             occlusion += ProcessOpacity(closure->as_add()->closureB, w );
//             break;
//         }
//         default: {
//             const ClosureComponent* comp = closure->as_comp();
//             occlusion += getSurfaceClosureBase(comp->id)->EvaluateOpacity(comp, w * comp->w);
//         }
//     }
//     return occlusion;
// }