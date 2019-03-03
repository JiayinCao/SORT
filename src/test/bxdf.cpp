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

#include <thread>
#include <mutex>
#include "unittest_common.h"
#include "thirdparty/gtest/gtest.h"
#include "sampler/sample.h"
#include "spectrum/spectrum.h"
#include "core/thread.h"
#include "core/samplemethod.h"
#include "bsdf/bsdf.h"
#include "bsdf/lambert.h"
#include "bsdf/orennayar.h"
#include "bsdf/phong.h"
#include "bsdf/ashikhmanshirley.h"
#include "bsdf/disney.h"
#include "bsdf/microfacet.h"
#include "bsdf/dielectric.h"
#include "bsdf/hair.h"

// A physically based BRDF should obey the rule of reciprocity
void checkReciprocity(const Bxdf* bxdf) {
    spinlock_mutex mutex;
    ParrallRun<8,128>( [&]() {
        const Vector wi = UniformSampleSphere(sort_canonical(), sort_canonical());
        const Vector wo = UniformSampleSphere(sort_canonical(), sort_canonical());

        const auto f0 = bxdf->F(wo, wi) * AbsCosTheta(wo);
        const auto f1 = bxdf->F(wi, wo) * AbsCosTheta(wi);

        std::lock_guard<spinlock_mutex> lock(mutex);
        ASSERT_NEAR(f0.GetR(), f1.GetR(), 0.001f);
        ASSERT_NEAR(f0.GetG(), f1.GetG(), 0.001f);
        ASSERT_NEAR(f0.GetB(), f1.GetB(), 0.001f);
    });
}

// A physically based BRDF/BTDF should not reflect more energy than it receives
void checkEnergyConservation(const Bxdf* bxdf) {
    Spectrum total = ParrallReduction<Spectrum, 8, 1024 * 1024 * 8>( [&](){
        Vector wi;
        float pdf = 0.0f;
        Spectrum r = bxdf->Sample_F(DIR_UP, wi, BsdfSample(true), &pdf);
        return pdf > 0.0f ? r / pdf : 0.0f;
    } );
    EXPECT_LE(total.GetR(), 1.01f);
    EXPECT_LE(total.GetG(), 1.01f);
    EXPECT_LE(total.GetB(), 1.01f);
}

// Check whether the pdf evaluated from sample_f matches the one from Pdf
// The exact algorithm is mentioned in my blog, except that the following algorithm also evaluates BTDF
// https://agraphicsguy.wordpress.com/2018/03/09/how-does-pbrt-verify-bxdf/
void checkPdf( const Bxdf* bxdf ){
    Vector wo(sort_canonical() * 2.0f - 1.0f, sort_canonical() * 2.0f - 1.0f, sort_canonical() * 2.0f - 1.0f);
    wo.Normalize();

    if( CosTheta( wo ) < 0.0f )
        wo = -wo;

    // Check whether pdf and spectrum value from Sample_F matches the Pdf and F functions
    spinlock_mutex mutex;
    ParrallRun<8,128>( [&]() {
        float pdf = 0.0f;
        Vector wi;
        const auto f0 = bxdf->Sample_F( wo , wi , BsdfSample(true) , &pdf );
        const float calculated_pdf = bxdf->Pdf( wo , wi );
        const auto f1 = bxdf->F( wo , wi );

        std::lock_guard<spinlock_mutex> lock(mutex);
        EXPECT_NEAR(pdf, calculated_pdf, 0.001f);
        EXPECT_TRUE( !isnan(pdf) );
        EXPECT_GE( pdf , 0.0f );
        EXPECT_NEAR(f0.GetR(), f1.GetR(), 0.001f);
        EXPECT_NEAR(f0.GetG(), f1.GetG(), 0.001f);
        EXPECT_NEAR(f0.GetB(), f1.GetB(), 0.001f);
    });

    // Check whether pdf adds together is less to 1.0
    // The sum won't converge to 1.0 because there are cases where importance sampling method will generated rays under the surface,
    // leading to 'invalid' sampling, which is simply dropped by setting pdf to 0.0.
    {
        double total = ParrallReduction<double, 8, 1024 * 1024 * 2>( [&](){
            Vector wi = UniformSampleSphere(sort_canonical(), sort_canonical());
            float pdf = UniformSpherePdf();
            return pdf > 0.0f ? bxdf->Pdf(wo, wi) / pdf : 0.0f;
        } );
        EXPECT_LE(total, 1.01f); // 1% error is tolerated
    }

    // Check whether the pdf actually matches the way rays are sampled
    {
        double total = ParrallReduction<double, 8>( [&](){
            Vector wi;
            float pdf;
            bxdf->Sample_F(wo, wi, BsdfSample(true), &pdf);
            return pdf != 0.0f ? 1.0f / pdf : 0.0f;
        } );
        EXPECT_NEAR(total, TWO_PI, 0.03f); // 0.3% error is tolerated
    }
}

void checkAll( const Bxdf* bxdf , bool cPdf = true , bool cReciprocity = true , bool cEnergyConservation = true ){
    if(cPdf) 
        checkPdf( bxdf );
    if(cReciprocity) 
        checkReciprocity( bxdf );
    if (cEnergyConservation)
        checkEnergyConservation(bxdf);
}

TEST (BXDF, Labmert) { 
    static const Spectrum R(1.0f);
    Lambert lambert( R , R , DIR_UP );
    checkAll( &lambert );
}

TEST(BXDF, LabmertTransmittion) {
    static const Spectrum R(1.0f);
    LambertTransmission lambert(R, R, DIR_UP);
    checkAll(&lambert);
}

TEST(BXDF, OrenNayar) {
    static const Spectrum R(1.0f);
    OrenNayar orenNayar(R, sort_canonical(), R, DIR_UP);
    checkAll(&orenNayar);
}

TEST(BXDF, Phong) {
    static const Spectrum R(1.0f);
    const float ratio = sort_canonical();
    Phong phong( R * ratio , R * ( 1.0f - ratio ) , sort_canonical(), R, DIR_UP);
    checkAll(&phong);
}

#if 0
// Sometimes it doesn't always pass, need investigation.
TEST(BXDF, AshikhmanShirley) {
    static const Spectrum R(1.0f);
    AshikhmanShirley as( R , sort_canonical() , sort_canonical() , sort_canonical() , R , DIR_UP );
    checkAll(&as);
}
#endif

#if 0
// https://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf
// Disney BRDF is not strictly energy conserving, please refer the above link for further detail ( chapter 5.1 ).
TEST(BXDF, Disney) {
    static const Spectrum R(1.0f);
    DisneyBRDF disney( R , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , R , DIR_UP );
    checkAll(&disney);
}
#endif

TEST(BXDF, MicroFacetReflection) {
    static const Spectrum R(1.0f);
    const FresnelConductor fresnel( 1.0f , 1.5f );
    const GGX ggx(0.5f, 0.5f);
    MicroFacetReflection mf( R , &fresnel , &ggx , R , DIR_UP );
    checkAll(&mf);
}

TEST(BXDF, MicroFacetRefraction) {
    static const Spectrum R(1.0f);
    const FresnelConductor fresnel( 1.0f , 1.5f );
    const GGX ggx( sort_canonical() , sort_canonical() );
    MicroFacetRefraction mr( R , &ggx , sort_canonical() , sort_canonical() , R , DIR_UP );
    checkAll( &mr , false , false , true );
}

TEST(BXDF, Dielectric) {
    static const Spectrum R(1.0f);
    const FresnelConductor fresnel( 1.0f , 1.5f );
    const GGX ggx( sort_canonical() , sort_canonical() );
    Dielectric dielectric( R , R , &ggx , sort_canonical() , sort_canonical() , R , DIR_UP );
    checkAll( &dielectric , false , false , true );
}

#if 0
TEST(BXDF, Hair) {
    Spectrum sigma_a = 0.f;
    Spectrum fullWeight = 1.0f;

    Vector3f wo = UniformSampleSphere( sort_canonical() , sort_canonical() );
    for (float beta_m = .1; beta_m < 1; beta_m += .2) {
        for (float beta_n = .1; beta_n < 1; beta_n += .2) {
            // Estimate reflected uniform incident radiance from hair
            Spectrum sum = 0.f;
            int count = 500000;
            Hair hair( sigma_a, beta_m, beta_n, 1.55f, fullWeight);
            for (int i = 0; i < count; ++i) {
                Vector3f wi = UniformSampleSphere( sort_canonical() , sort_canonical() );
                sum += hair.f(wo, wi);
            }

            float avg = sum.GetIntensity() / (count * UniformSpherePdf());
            EXPECT_NEAR(avg, 1.0f, 0.05f);
        }
    }

    for (float beta_m = .1; beta_m < 1; beta_m += .2) {
        for (float beta_n = .1; beta_n < 1; beta_n += .2) {
            Hair hair( sigma_a, beta_m, beta_n, 1.55f, fullWeight);
            checkAll( &hair , true , false , true );
        }
    }
}
#endif