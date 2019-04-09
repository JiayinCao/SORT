/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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
#include "core/memory.h"
#include "bsdf/lambert.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "bsdf/orennayar.h"
#include "bsdf/disney.h"
#include "bsdf/microfacet.h"
#include "bsdf/ashikhmanshirley.h"
#include "bsdf/phong.h"
#include "bsdf/dielectric.h"
#include "bsdf/hair.h"
#include "bsdf/fourierbxdf.h"
#include "bsdf/merl.h"
#include "bsdf/coat.h"
#include "bsdf/doublesided.h"

using namespace OSL;

// Process closures recursively
void ProcessClosure(Bsdf* bsdf, const OSL::ClosureColor* closure, const OSL::Color3& w);

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
        virtual void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) = 0;
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<Lambert::Params>();
            bsdf->AddBxdf(SORT_MALLOC(Lambert)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<OrenNayar::Params>();
            bsdf->AddBxdf(SORT_MALLOC(OrenNayar)(params, w * comp->w));
        }
    };

    struct Closure_Disney : public Closure_Base {
        static constexpr int    ClosureID = CLOSURE_DISNEY;

        static const char* GetName(){
            return "disney";
        }

        static void Register(ShadingSystem* shadingsys) {
            BuiltinClosures closure = { GetName(), ClosureID,{
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, subsurface),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, metallic),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, specular),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, specularTint),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, roughness),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, anisotropic),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, sheen),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, sheenTint),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, clearcoat),
                CLOSURE_FLOAT_PARAM(DisneyBRDF::Params, clearcoatGloss),
                CLOSURE_COLOR_PARAM(DisneyBRDF::Params, baseColor),
                CLOSURE_VECTOR_PARAM(DisneyBRDF::Params, n),
                CLOSURE_FINISH_PARAM(DisneyBRDF::Params)
            } };
            shadingsys->register_closure(closure.name, closure.id, closure.params, nullptr, nullptr);
        }

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<DisneyBRDF::Params>();
            bsdf->AddBxdf(SORT_MALLOC(DisneyBRDF)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<MicroFacetReflection::Params>();
            bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<MicroFacetRefraction::Params>();
            bsdf->AddBxdf(SORT_MALLOC(MicroFacetRefraction)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<AshikhmanShirley::Params>();
            bsdf->AddBxdf(SORT_MALLOC(AshikhmanShirley)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<Phong::Params>();
            bsdf->AddBxdf(SORT_MALLOC(Phong)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<LambertTransmission::Params>();
            bsdf->AddBxdf(SORT_MALLOC(LambertTransmission)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<MicroFacetReflection::MirrorParams>();
            bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<Dielectric::Params>();
            bsdf->AddBxdf(SORT_MALLOC(Dielectric)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<MicroFacetReflection::ParamsDieletric>();
            bsdf->AddBxdf(SORT_MALLOC(MicroFacetReflection)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<Hair::Params>();
            bsdf->AddBxdf(SORT_MALLOC(Hair)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<FourierBxdf::Params>();
            bsdf->AddBxdf(SORT_MALLOC(FourierBxdf)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<Merl::Params>();
            bsdf->AddBxdf(SORT_MALLOC(Merl)(params, w * comp->w));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<Coat::Params>();
            Bsdf* bottom = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
            ProcessClosure(bottom, params.closure, Color3(1.0f));
            bsdf->AddBxdf(SORT_MALLOC(Coat)(params, w, bottom));
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

        void Process(Bsdf* bsdf, const ClosureComponent* comp, const OSL::Color3& w) override {
            const auto& params = *comp->as<DoubleSided::Params>();
            Bsdf* bxdf0 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
            Bsdf* bxdf1 = SORT_MALLOC(Bsdf)(bsdf->GetIntersection(), true);
            ProcessClosure(bxdf0, params.bxdf0, Color3(1.0f));
            ProcessClosure(bxdf1, params.bxdf1, Color3(1.0f));
            bsdf->AddBxdf(SORT_MALLOC(DoubleSided)(bxdf0, bxdf1, w));
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
}

void ProcessClosure(Bsdf* bsdf, const OSL::ClosureColor* closure, const OSL::Color3& w) {
    if (!closure)
        return;
    switch (closure->id) {
        case ClosureColor::MUL: {
            Color3 cw = w * closure->as_mul()->weight;
            ProcessClosure(bsdf, closure->as_mul()->closure, cw);
            break;
        }
        case ClosureColor::ADD: {
            ProcessClosure(bsdf, closure->as_add()->closureA, w);
            ProcessClosure(bsdf, closure->as_add()->closureB, w);
            break;
        }
        default: {
            const ClosureComponent* comp = closure->as_comp();
            sAssert(comp->id >= 0 && comp->id < CLOSURE_CNT, MATERIAL);
            sAssert(g_closures[comp->id] != nullptr, MATERIAL);
            g_closures[comp->id]->Process(bsdf, comp, w * comp->w);
        }
    }
}