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

#include <thread>
#include <mutex>
#include "unittest_common.h"
#include "thirdparty/gtest/gtest.h"
#include "sampler/sample.h"
#include "spectrum/spectrum.h"
#include "core/thread.h"
#include "core/samplemethod.h"
#include "scatteringevent/bsdf/lambert.h"
#include "scatteringevent/bsdf/orennayar.h"
#include "scatteringevent/bsdf/phong.h"
#include "scatteringevent/bsdf/ashikhmanshirley.h"
#include "scatteringevent/bsdf/disney.h"
#include "scatteringevent/bsdf/microfacet.h"
#include "scatteringevent/bsdf/dielectric.h"
#include "scatteringevent/bsdf/hair.h"
#include "scatteringevent/bsdf/fabric.h"

// A physically based BRDF should obey the rule of reciprocity
void checkReciprocity(const Bxdf* bxdf) {
    spinlock_mutex mutex;
    ParrallRun<8,128>( [&]() {
        const Vector wi = UniformSampleSphere(sort_canonical(), sort_canonical());
        const Vector wo = UniformSampleSphere(sort_canonical(), sort_canonical());

        const auto f0 = bxdf->F(wo, wi) * absCosTheta(wo);
        const auto f1 = bxdf->F(wi, wo) * absCosTheta(wi);

        std::lock_guard<spinlock_mutex> lock(mutex);
        ASSERT_NEAR(f0.r, f1.r, 0.001f);
        ASSERT_NEAR(f0.g, f1.g, 0.001f);
        ASSERT_NEAR(f0.b, f1.b, 0.001f);
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
    EXPECT_LE(total.r, 1.03f);
    EXPECT_LE(total.g, 1.03f);
    EXPECT_LE(total.b, 1.03f);
}

// Check whether the pdf evaluated from sample_f matches the one from Pdf
// The exact algorithm is mentioned in my blog, except that the following algorithm also evaluates BTDF
// https://agraphicsguy.wordpress.com/2018/03/09/how-does-pbrt-verify-bxdf/
void checkPdf( const Bxdf* bxdf ){
    Vector wo = UniformSampleHemisphere( sort_canonical() , sort_canonical() );

    // Check whether pdf and spectrum value from Sample_F matches the Pdf and F functions
    spinlock_mutex mutex;
    ParrallRun<8,128>( [&]() {
        float pdf = 0.0f;
        Vector wi;
        const auto f0 = bxdf->Sample_F( wo , wi , BsdfSample(true) , &pdf );
        const float calculated_pdf = bxdf->Pdf( wo , wi );
        const auto f1 = bxdf->F( wo , wi );

        if( calculated_pdf == 0.0f )
            return;

        std::lock_guard<spinlock_mutex> lock(mutex);
        EXPECT_LE( fabs( pdf / calculated_pdf - 1.0f ) , 0.01f );
        EXPECT_TRUE( !IsNan(pdf) );
        EXPECT_GE( pdf , 0.0f );
        EXPECT_NEAR(f0.r, f1.r, 0.001f);
        EXPECT_NEAR(f0.g, f1.g, 0.001f);
        EXPECT_NEAR(f0.b, f1.b, 0.001f);
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
        EXPECT_LE(total, 1.03f); // 3% error is tolerated
    }

    // Check whether the pdf actually matches the way rays are sampled
    {
        double total = ParrallReduction<double, 8>( [&](){
            Vector wi;
            float pdf;
            bxdf->Sample_F(wo, wi, BsdfSample(true), &pdf);
            return pdf != 0.0f ? 1.0f / pdf : 0.0f;
        } );
        EXPECT_LE( fabs( total - TWO_PI ) , 0.03f );
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
    Lambert lambert( WHITE_SPECTRUM , WHITE_SPECTRUM , DIR_UP );
    checkAll( &lambert );
}

TEST(BXDF, LabmertTransmittion) {
    LambertTransmission lambert(WHITE_SPECTRUM, WHITE_SPECTRUM, DIR_UP);
    checkAll(&lambert);
}

TEST(BXDF, OrenNayar) {
    OrenNayar orenNayar(WHITE_SPECTRUM, sort_canonical(), FULL_WEIGHT, DIR_UP);
    checkAll(&orenNayar);
}

TEST(BXDF, Phong) {
    const float ratio = sort_canonical();
    Phong phong( WHITE_SPECTRUM * ratio , WHITE_SPECTRUM * ( 1.0f - ratio ) , sort_canonical(), FULL_WEIGHT , DIR_UP);
    checkAll(&phong, false);
}

// Sometimes it doesn't always pass, need investigation.
TEST(BXDF, DISABLED_AshikhmanShirley) {
    AshikhmanShirley as( WHITE_SPECTRUM , sort_canonical() , sort_canonical() , sort_canonical() , FULL_WEIGHT , DIR_UP );
    checkAll(&as);
}

// https://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf
// Disney BRDF is not strictly energy conserving, please refer the above link for further detail ( chapter 5.1 ).
TEST(BXDF, DISABLED_Disney) {
    DisneyBRDF disney( WHITE_SPECTRUM , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() ,
                       sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() , sort_canonical() ,
                       sort_canonical() , sort_canonical() , 0 , FULL_WEIGHT , DIR_UP );
    checkAll(&disney);
}

TEST(BXDF, DISABLED_MicroFacetReflection) {
    const FresnelConductor fresnel( 1.0f , 1.5f );
    const GGX ggx(0.5f, 0.5f);
    MicroFacetReflection mf( WHITE_SPECTRUM , &fresnel , &ggx , FULL_WEIGHT , DIR_UP );
    checkAll(&mf);
}

TEST(BXDF, MicroFacetRefraction) {
    const FresnelConductor fresnel( 1.0f , 1.5f );
    const GGX ggx( sort_canonical() , sort_canonical() );
    MicroFacetRefraction mr( WHITE_SPECTRUM , &ggx , sort_canonical() , sort_canonical() , FULL_WEIGHT , DIR_UP );
    checkAll( &mr , false , false , true );
}

TEST(BXDF, Dielectric) {
    const FresnelConductor fresnel( 1.0f , 1.5f );
    const GGX ggx( sort_canonical() , sort_canonical() );
    Dielectric dielectric( WHITE_SPECTRUM , WHITE_SPECTRUM , &ggx , sort_canonical() , sort_canonical() , FULL_WEIGHT , DIR_UP );
    checkAll( &dielectric , false , false , true );
}

TEST(BXDF, DISABLED_DreamWork_Fabric) {
    auto test_fabric = []( const float roughness ){
        Fabric fabric( WHITE_SPECTRUM , roughness , FULL_WEIGHT , DIR_UP );
        checkAll( &fabric );
    };

    test_fabric( 0.0f );
    test_fabric( 0.5f );
    test_fabric( 1.0f );
}

TEST(BXDF, DISABLED_HairFurnace) {
    Spectrum sigma_a = 0.0f;

    Vector3f wo = UniformSampleHemisphere(sort_canonical(), sort_canonical());
    for (float beta_m = 0.0f; beta_m <= 1.0f; beta_m += 0.2f) {
        for (float beta_n = 0.0f; beta_n <= 1.0f; beta_n += 0.2f) {
            // Estimate reflected uniform incident radiance from hair
            auto sum = 0.f;
            constexpr int CNT = 1024 * 256;
            Hair hair(sigma_a, beta_m, beta_n, 1.55f, FULL_WEIGHT);
            sum += ParrallReduction<float, 8, CNT>([&]() {
                Vector3f wi = UniformSampleSphere(sort_canonical(), sort_canonical());
                EXPECT_GE(hair.f(wo, wi).GetIntensity(), 0.00f);
                return hair.f(wo, wi).GetIntensity() / UniformSpherePdf();
            });

            EXPECT_LE(sum, 1.05f);
            EXPECT_GE(sum, 0.95f);
        }
    }
}

// Since hair has its exact way to importance sample its bxdf, the evaluated bxdf and pdf should be exactly the same.
TEST(BXDF, HairPDFConsistant) {
    static const Spectrum sigma_a = 0.f;

    // Since the PDF of hair BXDF matches exactly with its BXDF value itself, there is a special PDF verification process for hair.
    auto checkPDF = [] ( const Bxdf* bxdf ){
        const auto wo = UniformSampleHemisphere( sort_canonical() , sort_canonical() );

        spinlock_mutex mutex0;
        ParrallRun<8,128>( [&]() {
            Vector wi;
            float pdf;
            auto f = bxdf->Sample_F(wo, wi, BsdfSample(true), &pdf);

            std::lock_guard<spinlock_mutex> lock(mutex0);
            if( pdf > 0.0f )
                EXPECT_LE( fabs( f.GetIntensity() / pdf - 1.0f ) , 0.01f );
        });
    };

    for (float beta_m = 0.1f; beta_m < 1.0f; beta_m += 0.5f) {
        for (float beta_n = 0.1f; beta_n < 1.0f; beta_n += 0.5f) {
            Hair hair( sigma_a, beta_m, beta_n, 1.55f, FULL_WEIGHT);
            checkPDF( &hair );
        }
    }
}

TEST(BXDF, DISABLED_HairStandardChecking) {
    static Spectrum sigma_a = 0.f;
    for (float beta_m = 0.1f; beta_m < 1.0f; beta_m += 0.5f) {
        for (float beta_n = 0.1f; beta_n < 1.0f; beta_n += 0.5f) {
            Hair hair(sigma_a, beta_m, beta_n, 1.55f, FULL_WEIGHT);
            checkAll(&hair , true , false , true );
        }
    }
}

// This is generally not a very unstable way to test the BRDF. Disabled by default.
TEST(BXDF, DISABLED_HairSamplingConsistance) {
    static Spectrum sigma_a = 0.f;
    // Since the PDF of hair BXDF matches exactly with its BXDF value itself, there is a special PDF verification process for hair.
    auto checkPDF = [] ( const Bxdf* bxdf ){
        constexpr int CNT = 1024 * 64;
        const auto Li = []( const Vector& w ) -> Spectrum { return w.y * w.y ; };

        Vector wo = UniformSampleHemisphere( sort_canonical() , sort_canonical() );
        spinlock_mutex mutex1;
        Spectrum uni , imp;
        ParrallRun<8, CNT>( [&](){
            Vector wi0;
            float pdf;
            auto f0 = bxdf->Sample_F( wo , wi0 , BsdfSample() , &pdf );
            f0 = pdf > 0.0f ? f0 * Li( wi0 ) / pdf : Spectrum(0.0f);
            const auto wi1 = UniformSampleSphere( sort_canonical() , sort_canonical() );
            const auto f1 = bxdf->F( wo , wi1 ) * Li( wi1 ) / UniformSpherePdf();

            std::lock_guard<spinlock_mutex> lock(mutex1);
            if( pdf > 0.0f )
                imp += f0;
            uni += f1;
        } );
        const auto ratio = uni.GetIntensity() / imp.GetIntensity();
        if( fabs( ratio - 1.0f ) > 0.05f ){
            std::cout<<uni.r << "\t"<<uni.g << "\t"<< uni.b<<std::endl;
            std::cout<<imp.r << "\t"<<imp.g << "\t"<< imp.b<<std::endl;
        }
        EXPECT_LE( fabs( ratio - 1.0f ) , 0.05f );
    };

    for (float beta_m = 0.1f; beta_m < 1.0f; beta_m += 0.5f) {
        for (float beta_n = 0.1f; beta_n < 1.0f; beta_n += 0.5f) {
            Hair hair( sigma_a, beta_m, beta_n, 1.55f, FULL_WEIGHT);
            checkPDF( &hair );
        }
    }
}
