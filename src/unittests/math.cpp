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
#include "math/exp.h"
#include "unittest_common.h"

using namespace unittest;

SORT_FORCEINLINE void exp_accuracy_test( const double x ){
    const double e0 = exp( x );
    const double e1 = exp_compile( x );
    EXPECT_NEAR( e0 , e1 , 0.01f * e0 );
};

TEST(MATH, EXP_ACCURACY) {
    exp_accuracy_test( 1.0 );
    exp_accuracy_test( 1.5 );
    exp_accuracy_test( 2.0 );
    exp_accuracy_test( 4.0 );
    exp_accuracy_test( 128.0 );
    exp_accuracy_test( 256.0 );
}

TEST(MATH, EXP_ZERO_INPUT) {
    exp_accuracy_test( 0.0 );
}

TEST(MATH, EXP_NEGATIVE) {
    exp_accuracy_test( -1.0 );
    exp_accuracy_test( -1.5 );
    exp_accuracy_test( -2.0 );
    exp_accuracy_test( -4.0 );
    exp_accuracy_test( -128.0 );
    exp_accuracy_test( -256.0 );
}