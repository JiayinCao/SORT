/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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
#include "bsdf/bsdf.h"
#include "sampler/sample.h"
#include "spectrum/spectrum.h"
#include "bsdf/microfacet.h"
#include "bsdf/disney.h"
#include <thread>
#include "utility/samplemethod.h"

// Check PDF evaluation 
void checkDist( const MicroFacetDistribution* dist ){
    const int TN = 8;   // thread number
    double total[TN] = { 0.0f };
    std::thread threads[TN];
    for (int i = 0; i < TN; ++i) {
        threads[i] = std::thread([&](int tid) {
            const long long N = 1024 * 1024 * 8;
            double local = 0.0f;
            for (long long i = 0; i < N; ++i) {
                auto h = dist->sample_f(BsdfSample(true));
                auto pdf = dist->Pdf(h);
                local += pdf != 0.0f ? 1.0f / pdf : 0.0f;
            }
            total[tid] += (double)local / (double)(N * TN);
        }, i);
    }
    for (int i = 0; i < TN; ++i)
        threads[i].join();
    double final_total = 0.0f;
    for (int i = 0; i < TN; ++i)
        final_total += total[i];
    EXPECT_NEAR(final_total, TWO_PI, 0.03f); // 0.5% error is tolerated
}

// Check Pdf
void checkPdf( const MicroFacetDistribution* dist ){
    const int TN = 8;   // thread number
    double total[TN] = { 0.0f };
    std::thread threads[TN];
    for (int i = 0; i < TN; ++i) {
        threads[i] = std::thread([&](int tid) {
            const long long N = 1024 * 1024 * 2;
            double local = 0.0f;
            for (long long i = 0; i < N; ++i) {
                const Vector h = UniformSampleHemisphere(sort_canonical(), sort_canonical());
                const float pdf = UniformHemispherePdf();
                if (pdf > 0.0f)
                    local += dist->Pdf(h) / pdf;
            }
            total[tid] += (double)local / (double)(N * TN);
        }, i);
    }
    for (int i = 0; i < TN; ++i)
        threads[i].join();
    double final_total = 0.0f;
    for (int i = 0; i < TN; ++i)
        final_total += total[i];
    EXPECT_NEAR(final_total, 1.00f, 0.01f ); // 1% error is tolerated
}

void checkAll( const MicroFacetDistribution* dist ){
    checkPdf( dist );
    checkDist( dist );
}

TEST(DISTRIBUTION, GGX) {
    const GGX ggx(0.5f,0.5f);
    checkAll(&ggx);
}

TEST(DISTRIBUTION, Beckmann) {
    const Beckmann beckmann(0.5f,0.5f);
    checkAll(&beckmann);
}

TEST(DISTRIBUTION, Blinn) {
    const Blinn blinn(0.5f,0.5f);
    checkAll(&blinn);
}

TEST(DISTRIBUTION, ClearcoatGGX) {
    const ClearcoatGGX cggx(0.5f);
    checkAll(&cggx);
}
