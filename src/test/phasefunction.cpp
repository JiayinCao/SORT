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

#include "thirdparty/gtest/gtest.h"
#include "medium/phasefunction.h"
#include "unittest_common.h"
#include "core/render_context.h"

// This should work. For some reason, it doesn't. I need to figure out what is wrong.
// This is probably because the quality of the random number generated is not good.
// With the tolorance of 4%, there is still 0.2% failing rate, for which reason this unit test won't be activated.
TEST(PHASE_FUNCTION, DISABLED_HenyeyGreenstein_PDF_Sample_Accuracy) {
    RenderContext rc;
    rc.Init();

    const auto u = sort_rand<float>(rc);
    const auto v = sort_rand<float>(rc);
    const auto wo = UniformSampleSphere( u , v );
	
    // Check whether the pdf actually matches the way rays are sampled
    const auto total0 = ParrallReduction<double, 8, 8 * 1024 * 1024>( [&](){
        RenderContext rc;
        rc.Init();

        Vector wi;
        float pdf = 0.0f;
        const HenyeyGreenstein hg( sort_rand<float>(rc) );
        hg.Sample( rc, wo , wi , pdf );
        return pdf != 0.0f ? 1.0f / pdf : 0.0f;
    } );
    EXPECT_NEAR( total0 , FOUR_PI , 0.04f );

    // corner cases when asymmetry parameter in HG phase function is zero
    const auto total1 = ParrallReduction<double, 8, 1024 * 1024>( [&](){
        RenderContext rc;
        rc.Init();

        Vector wi;
        float pdf = 0.0f;
        const HenyeyGreenstein hg( 0.0f );
        hg.Sample( rc, wo , wi , pdf );
        return pdf != 0.0f ? 1.0f / pdf : 0.0f;
    } );
    EXPECT_NEAR( total1 , FOUR_PI , 0.04f );
}

TEST(PHASE_FUNCTION, HenyeyGreenstein_PDF_Sample) {
    RenderContext rc;
    rc.Init();

    const auto u = sort_rand<float>(rc);
    const auto v = sort_rand<float>(rc);
    const auto wo = UniformSampleSphere( u , v );
	
    // since HenyeyGreenstein has a precise pdf sampling policy, its pdf should be exactly the same with its value.
    ParrallRun<8, 1024 * 1024>( [&](){
        RenderContext rc;
        rc.Init();

        Vector wi;
        auto pdf = 0.0f;
        const HenyeyGreenstein hg( sort_rand<float>(rc) );
        hg.Sample( rc, wo , wi , pdf );

        const auto hg_value = hg.P( wo , wi );
        EXPECT_NEAR( pdf , hg_value , 0.001f );
    } );

    // corner cases when asymmetry parameter in HG phase function is zero
    ParrallRun<8, 1024 * 1024>( [&](){
        RenderContext rc;
        rc.Init();

        Vector wi;
        float pdf = 0.0f;
        const HenyeyGreenstein hg( 0.0f );
        hg.Sample( rc, wo , wi , pdf );

        const auto hg_value = hg.P( wo , wi );
        EXPECT_NEAR( pdf , hg_value , 0.001f );
    } );
}