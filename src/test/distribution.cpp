/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "thirdparty/gtest/gtest.h"
#include "unittest_common.h"
#include "sampler/sample.h"
#include "spectrum/spectrum.h"
#include "scatteringevent/bsdf/microfacet.h"
#include "scatteringevent/bsdf/disney.h"
#include <thread>
#include "core/samplemethod.h"

// Check PDF evaluation
void checkDist( const MicroFacetDistribution* dist ){
    double final_total = ParrallReduction<double, 8>( [&](){
            auto h = dist->sample_f(BsdfSample(true));
            auto pdf = dist->Pdf(h);
            return pdf != 0.0f ? 1.0f / pdf : 0.0f;
        } );
    EXPECT_NEAR(final_total, TWO_PI, 0.03f); // 0.5% error is tolerated
}

// Check Pdf
void checkPdf( const MicroFacetDistribution* dist ){
    double final_total = ParrallReduction<double, 8>( [&](){
            const Vector h = UniformSampleHemisphere(sort_canonical(), sort_canonical());
            const float pdf = UniformHemispherePdf();
            return pdf > 0.0f ? dist->Pdf(h) / pdf : 0.0f;
        } );
    EXPECT_LE(final_total, 1.01f); // 1% error is tolerated
}

void checkAll( const MicroFacetDistribution* dist ){
    checkPdf( dist );
    checkDist( dist );
}

// Somehow, this unit test always fails. Need to investigate.
TEST(DISTRIBUTION, DISABLED_GGX) {
    const GGX ggx(0.5f,0.5f);
    checkAll(&ggx);
}

// Somehow, this unit test always fails. Need to investigate.
TEST(DISTRIBUTION, DISABLED_Beckmann) {
    const Beckmann beckmann(0.5f,0.5f);
    checkAll(&beckmann);
}

// Somehow, this unit test always fails. Need to investigate.
TEST(DISTRIBUTION, DISABLED_Blinn) {
    const Blinn blinn(0.5f,0.5f);
    checkAll(&blinn);
}

#if 0
// Somehow, this unit test always fails. Need to investigate.
TEST(DISTRIBUTION, ClearcoatGGX) {
    const ClearcoatGGX cggx(0.5f);
    checkAll(&cggx);
}
#endif
