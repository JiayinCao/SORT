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
#include "tsl_utils.h"
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

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeEmpty)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeEmpty, float, dummy)    // this is purely because tsl doesn't support empty closure structure
DECLARE_CLOSURE_TYPE_END(ClosureTypeEmpty)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeEmpty)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeEmpty, float, dummy)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeEmpty)

#define DEFINE_CLOSURETYPE(T,name)  static ClosureID closure_id; \
                                    static const char* GetName(){ return name; } \
                                    static void Register() { closure_id = T::RegisterClosure(GetName(), ShadingSystem::get_instance()); }
                                    
#define DEFINE_CLOSUREID(T)         ClosureID T::closure_id = INVALID_CLOSURE_ID

#define ALL_SURFACE_CLOSURE_ACTION \
        SURFACE_CLOSURE_ACTION(Surface_Closure_Empty)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Lambert)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_OrenNayar)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Disney)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_LambertTransmission)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Mirror)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Dielectric)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_AshikhmanShirley)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_SSS)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_DoubleSided)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Transparent)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Phong)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_DistributionBRDF)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Fabric)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_MicrofacetReflectionGGX)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_MicrofacetReflectionBlinn)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_MicrofacetReflectionBeckmann)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_MicrofacetReflectionDielectric)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_MicrofacetRefractionGGX)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_MicrofacetRefractionBlinn)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_MicrofacetRefractionBeckmann)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Hair)\
        SURFACE_CLOSURE_ACTION(Surface_Closure_Coat)

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
         //! @brief     This is the interface for creating the processing the closure and fill the scattering event.
         //!
         //! @param param       Closure parameter.
         //! @param w           Weight of the current closure.
         //! @param se          This is the output of the function, the scattering event.
         virtual void Process(const ClosureParamPtr param, const float3& w, ScatteringEvent& se) const = 0;

         //! @brief     Evaluate the opacity of the closure.
         //!
         //! @param param       Closure parameter.
         //! @param w           Weight of the current closure.
         //! @return            The opacity of the closure.
         virtual Spectrum EvaluateOpacity(const ClosureParamPtr comp, const float3& w) const {
             return w;
         }
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

     struct Surface_Closure_Empty: public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeEmpty, "empty_closure")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             // nothing inside the closure needs to be processed.
         }
     };

     struct Surface_Closure_Lambert : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeLambert, "lambert")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             ClosureTypeLambert* bxdf_param = (ClosureTypeLambert*)param;
             se.AddBxdf(SORT_MALLOC(Lambert)(*bxdf_param, w));
         }
     };

     struct Surface_Closure_OrenNayar : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeOrenNayar, "oren_nayar")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             ClosureTypeOrenNayar* bxdf_param = (ClosureTypeOrenNayar*)param;
             se.AddBxdf(SORT_MALLOC(OrenNayar)(*bxdf_param, w));
         }
     };

     struct Surface_Closure_Disney : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeDisney, "disney")

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

     struct Surface_Closure_MicrofacetReflectionGGX : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMicrofacetReflectionGGX, "microfacet_reflection_ggx")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeMicrofacetReflectionGGX*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w));
         }
     };

     struct Surface_Closure_MicrofacetReflectionBlinn: public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMicrofacetReflectionBlinn, "microfacet_reflection_blinn")

        void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeMicrofacetReflectionBlinn*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w));
         }
     };

     struct Surface_Closure_MicrofacetReflectionBeckmann : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMicrofacetReflectionBeckmann, "microfacet_reflection_beckmann")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeMicrofacetReflectionBeckmann*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w));
         }
     };

     struct Surface_Closure_MicrofacetRefractionGGX : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMicrofacetReflectionBeckmann, "microfacet_refraction_ggx")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeMicrofacetRefractionGGX*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetRefraction)(params, w));
         }
     };

     struct Surface_Closure_MicrofacetRefractionBlinn : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMicrofacetReflectionBeckmann, "microfacet_refraction_blinn")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeMicrofacetRefractionBlinn*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetRefraction)(params, w));
         }
     };

     struct Surface_Closure_MicrofacetRefractionBeckmann : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMicrofacetReflectionBeckmann, "microfacet_refraction_beckmann")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeMicrofacetRefractionBeckmann*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetRefraction)(params, w));
         }
     };

     struct Surface_Closure_AshikhmanShirley : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeAshikhmanShirley, "ashikhman_shirley")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(ClosureTypeAshikhmanShirley*)param;
             se.AddBxdf(SORT_MALLOC(AshikhmanShirley)(params, w));
         }
     };

     struct Surface_Closure_Phong : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypePhong, "phong")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(ClosureTypePhong*)param;
             se.AddBxdf(SORT_MALLOC(Phong)(params, w));
         }
     };

     struct Surface_Closure_LambertTransmission : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeLambertTransmission, "lambert_transmission")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeLambertTransmission*)param;
             se.AddBxdf(SORT_MALLOC(LambertTransmission)(params, w));
         }
     };

     struct Surface_Closure_Mirror : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMirror, "mirror")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(ClosureTypeMirror*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w));
         }
     };

     struct Surface_Closure_Dielectric : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeDielectric, "dieletric")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(ClosureTypeDielectric*)param;
             se.AddBxdf(SORT_MALLOC(Dielectric)(params, w));
         }
     };

     struct Surface_Closure_MicrofacetReflectionDielectric : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeMicrofacetReflectionDielectric, "microfacet_dielectric")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3 & w, ScatteringEvent & se) const override {
             const auto& params = *(ClosureTypeMicrofacetReflectionDielectric*)param;
             se.AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w));
         }
     };

     struct Surface_Closure_Hair : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeHair, "hair")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeHair*)param;
             se.AddBxdf(SORT_MALLOC(Hair)(params, w));
         }
     };

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

     struct Surface_Closure_Coat : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeCoat, "coat")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeCoat*)param;
             ScatteringEvent* bottom = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
             ProcessSurfaceClosure((const ClosureTreeNodeBase*)params.closure, make_float3(1.0f, 1.0f, 1.0f), *bottom);
             se.AddBxdf(SORT_MALLOC(Coat)(params, w, bottom));
         }
     };

     struct Surface_Closure_DoubleSided : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeDoubleSided, "double_sided")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3 & w, ScatteringEvent & se) const override {
             const auto& params = *(const ClosureTypeDoubleSided*)param;
             ScatteringEvent* se0 = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
             ScatteringEvent* se1 = SORT_MALLOC(ScatteringEvent)(se.GetInteraction(), SE_Flag( SE_EVALUATE_ALL | SE_SUB_EVENT | SE_REPLACE_BSSRDF ) );
             ProcessSurfaceClosure((const ClosureTreeNodeBase*)params.closure0, make_float3(1.0f, 1.0f, 1.0f), *se0);
             ProcessSurfaceClosure((const ClosureTreeNodeBase*)params.closure1, make_float3(1.0f, 1.0f, 1.0f), *se1);
             se.AddBxdf(SORT_MALLOC(DoubleSided)(se0, se1, w));
         }
     };

     struct Surface_Closure_DistributionBRDF : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeDistributionBRDF, "distribution_brdf")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeDistributionBRDF*)param;
             se.AddBxdf(SORT_MALLOC(DistributionBRDF)(params, w));
         }
     };

     struct Surface_Closure_Fabric : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeFabric, "fabric")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeFabric*)param;
             se.AddBxdf(SORT_MALLOC(Fabric)(params, w));
         }
     };

     struct Surface_Closure_SSS : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeSSS, "subsurface_scattering")

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

     struct Surface_Closure_Transparent : public Surface_Closure_Base {
         DEFINE_CLOSURETYPE(ClosureTypeTransparent, "transparent")

         void Process(const Tsl_Namespace::ClosureParamPtr param, const Tsl_Namespace::float3& w, ScatteringEvent& se) const override {
             const auto& params = *(const ClosureTypeTransparent*)param;
             se.AddBxdf(SORT_MALLOC(Transparent)(params, w));
         }

         Spectrum EvaluateOpacity(const ClosureParamPtr param, const float3& w) const override{
             // do I need to take the weight into account?
             const auto& params = *(const ClosureTypeTransparent*)param;
             return 1.0f - Spectrum(params.attenuation);
         }
     };

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

#define SURFACE_CLOSURE_ACTION(T) DEFINE_CLOSUREID(T);
     ALL_SURFACE_CLOSURE_ACTION
#undef SURFACE_CLOSURE_ACTION
}

 template< typename T >
 static void registerSurfaceClosure() {
     T::Register();
     g_surface_closures[T::closure_id] = std::make_unique<T>();
 }

// template< typename T >
// static void registerVolumeClosure(OSL::ShadingSystem* shadingsys) {
//     T::Register(shadingsys);
//     g_volume_closures[T::ClosureID - VOLUME_CLOSURE_BASE] = std::make_unique<T>();
// }

void RegisterClosures() {
#define SURFACE_CLOSURE_ACTION(T) registerSurfaceClosure<T>();
    ALL_SURFACE_CLOSURE_ACTION
#undef SURFACE_CLOSURE_ACTION

    /*
    registerSurfaceClosure<Surface_Closure_MERL>(shadingsys);
    registerSurfaceClosure<Surface_Closure_FourierBRDF>(shadingsys);
    
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

 Spectrum ProcessOpacity(const ClosureTreeNodeBase* closure, const float3& w ){
     if (!closure)
         return 0.0f;

     Spectrum occlusion = 0.0f;

     switch (closure->m_id) {
         case Tsl_Namespace::CLOSURE_ADD:
             {
                 const ClosureTreeNodeAdd* closure_add = (const ClosureTreeNodeAdd*)closure;
                 occlusion += Spectrum(ProcessOpacity(closure_add->m_closure0, w));
                 occlusion += Spectrum(ProcessOpacity(closure_add->m_closure1, w));
             }
             break;
         case Tsl_Namespace::CLOSURE_MUL:
             {
                 const ClosureTreeNodeMul* closure_mul = (const ClosureTreeNodeMul*)closure;
                 const float3 weight = make_float3(w.x * closure_mul->m_weight, w.y * closure_mul->m_weight, w.z * closure_mul->m_weight);
                 occlusion += Spectrum(ProcessOpacity(closure_mul->m_closure, weight));
             }
             break;
         default:
             auto closure_base = getSurfaceClosureBase(closure->m_id);
             occlusion += Spectrum(closure_base->EvaluateOpacity(closure->m_params, w));
             break;
     }

     return occlusion;
 }