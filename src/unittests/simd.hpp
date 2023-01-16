/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <math.h>
#include "core/define.h"
#include "thirdparty/gtest/gtest.h"
#include "simd/simd_wrapper.h"
#include "unittest_common.h"
#include "job/fiber.h"

using namespace unittest;

#ifdef SIMD_8WAY_IMPLEMENTATION
    #ifdef SORT_X64_TARGET
        #define SIMD_TEST       SIMD_AVX
    #else
        #error "Undefined SIMD"
    #endif
#endif

#ifdef SIMD_4WAY_IMPLEMENTATION
    #ifdef SORT_X64_TARGET
        #define SIMD_TEST       SIMD_SSE
    #elif defined(SORT_ARM64_TARGET)
        #define SIMD_TEST       SIMD_NEON
    #else
        #error "Undefined SIMD"
    #endif
#endif

#if defined( SIMD_4WAY_IMPLEMENTATION ) || defined( SIMD_8WAY_IMPLEMENTATION )

static constexpr float nan_unsigned = 0xffc00000;
static constexpr float nan_float = *((float*)(&nan_unsigned));

TEST(SIMD_TEST, simd_set_ps1) {
    constexpr float c_data = 2.0f;
    const auto simd_data = simd_set_ps1( c_data );
    for( int i = 0 ; i < SIMD_CHANNEL ; ++i )
        EXPECT_EQ( simd_data[i] , c_data );
}

TEST(SIMD_TEST, simd_set_ps) {
    float data[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        data[i] = 2.0f * i;

    const auto simd_data = simd_set_ps( data );
    for( int i = 0 ; i < SIMD_CHANNEL ; ++i )
        EXPECT_EQ( simd_data[i] , data[i] );
}

TEST(SIMD_TEST, simd_set_mask) {
    bool data[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        data[i] = ( i % 2 ) ? true : false;

    const auto simd_data = simd_set_mask( data );
    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const float f = simd_data[i];
        EXPECT_EQ( *((unsigned*)(&f)) , data[i] ? 0xffffffff : 0x0 );
    }
}

TEST(SIMD_TEST, simd_add_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_add_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i )
        EXPECT_EQ( simd_data[i] , data0[i] + data1[i] );
}

TEST(SIMD_TEST, simd_sub_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_sub_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i )
        EXPECT_EQ( simd_data[i] , data0[i] - data1[i] );
}

TEST(SIMD_TEST, simd_mul_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_mul_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i )
        EXPECT_EQ( simd_data[i] , data0[i] * data1[i] );
}

TEST(SIMD_TEST, simd_div_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i + 1.0f;
        data1[i] = i * i;
    }

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_div_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] / data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_sqr_ps) {
    float data[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        data[i] = 2.0f * i;

    const auto simd_data = simd_set_ps( data );
    const auto simd_data_sqr = simd_sqr_ps( simd_data );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i )
        EXPECT_EQ( simd_data_sqr[i] , data[i] * data[i] );
}

TEST(SIMD_TEST, simd_sqrt_ps) {
    float data[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        data[i] = 2.0f * i;

    // this is intentional to test 'bad' values
    data[0] = -1.0f;

    const auto simd_data = simd_set_ps( data );
    const auto simd_data_sqrt = simd_sqrt_ps( simd_data );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data_sqrt[i];
        const auto f1 = sqrt(data[i]);

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_rcp_ps) {
    float data[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        data[i] = 2.0f * i;

    const auto simd_data = simd_set_ps( data );
    const auto simd_data_sqrt = simd_rcp_ps( simd_data );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data_sqrt[i];
        const auto f1 = 1.0f / data[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_mad_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL] , data2[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
        data2[i] = i * i * i;
    }

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data2 = simd_set_ps( data2 );
    const auto simd_data = simd_mad_ps( simd_data0 , simd_data1 , simd_data2 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] * data1[i] + data2[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_pick_ps) {
    bool data0[SIMD_CHANNEL];
    float data1[SIMD_CHANNEL] , data2[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = ( ( i % 2 ) == 0 ) ? true : false;
        data1[i] = i * i;
        data2[i] = i * i * i;
    }
    data2[0] = nan_float;

    const auto simd_data0 = simd_set_mask( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data2 = simd_set_ps( data2 );
    const auto simd_data = simd_pick_ps( simd_data0 , simd_data1 , simd_data2 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] ? data1[i] : data2[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_cmpeq_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_cmpeq_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] == data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = f1 ? 0xffffffff : 0x0;

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_cmpneq_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    // check the warning in simd_wrapper.h, this line won't survive
    // data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_cmpneq_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] != data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = f1 ? 0xffffffff : 0x0;

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_cmple_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_cmple_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] <= data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = f1 ? 0xffffffff : 0x0;

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_cmplt_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_cmplt_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] < data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = f1 ? 0xffffffff : 0x0;

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_cmpge_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_cmpge_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] >= data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = f1 ? 0xffffffff : 0x0;

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_cmpgt_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_cmpgt_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] > data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = f1 ? 0xffffffff : 0x0;

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_min_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_min_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] > data1[i] ? data1[i] : data0[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_max_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_max_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i] < data1[i] ? data1[i] : data0[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));

        EXPECT_EQ( u0 , u1 );
    }
}

TEST(SIMD_TEST, simd_and_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_and_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i];
        const auto f2 = data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));
        const auto u2 = *((unsigned*)(&f2));

        EXPECT_EQ( u0 , u1 & u2 );
    }
}

TEST(SIMD_TEST, simd_or_ps) {
    float data0[SIMD_CHANNEL] , data1[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        data0[i] = 2.0f * i;
        data1[i] = i * i;
    }
    data0[0] = data1[0] = nan_float;

    const auto simd_data0 = simd_set_ps( data0 );
    const auto simd_data1 = simd_set_ps( data1 );
    const auto simd_data = simd_or_ps( simd_data0 , simd_data1 );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto f0 = simd_data[i];
        const auto f1 = data0[i];
        const auto f2 = data1[i];

        const auto u0 = *((unsigned*)(&f0));
        const auto u1 = *((unsigned*)(&f1));
        const auto u2 = *((unsigned*)(&f2));

        EXPECT_EQ( u0 , u1 | u2 );
    }
}

TEST(SIMD_TEST, simd_movemask_ps) {
    float data[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        data[i] = ( i % 2 ) ? 2.0f * i : -1.0f * i;

    const auto simd_data = simd_set_ps( data );
    const auto simd_data_mask = simd_movemask_ps( simd_data );

    for( int i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const bool b0 = ( simd_data_mask >> i ) & 0x01;
        const bool b1 = ( *((unsigned*)(&data[i])) >> 31 ) & 0x01;

        EXPECT_EQ( b0 , b1 );
    }
}

TEST(SIMD_TEST, simd_minreduction_ps) {
    float data[SIMD_CHANNEL];
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        data[i] = 2.0f * ( (float)i - 1.5f ) * ( (float)i - 1.5f );

    const auto simd_data = simd_set_ps( data );
    const auto reduction = simd_minreduction_ps( simd_data );
    const auto correct_reduction = simd_set_ps1( 0.5f );

    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
        EXPECT_EQ( reduction[0] , correct_reduction[0] );
}

// make sure the simd operations won't get affected by fiber suspention
TEST(SIMD_TEST, fiber_split) {
    std::unique_ptr<Fiber> fiber, thread_fiber;

    // capture the current thread so that we can swap
    thread_fiber = createFiberFromThread();

    fiber = createFiber(4096, 
        [&](){
            float data[SIMD_CHANNEL];
            for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
                data[i] = 2.0f * ( (float)i - 1.5f ) * ( (float)i - 1.5f );

            const auto simd_data = simd_set_ps( data );
            const auto reduction = simd_minreduction_ps( simd_data );
            
            switchFiber(fiber.get(), thread_fiber.get());

            const auto correct_reduction = simd_set_ps1( 0.5f );
            for( auto i = 0 ; i < SIMD_CHANNEL ; ++i )
                EXPECT_EQ( reduction[0] , correct_reduction[0] );

            switchFiber(fiber.get(), thread_fiber.get());
        }
    );

    // swap to that fiber
    switchFiber(thread_fiber.get(), fiber.get());

    // resume the fiber to verify the simd operations
    switchFiber(thread_fiber.get(), fiber.get());
}

#endif