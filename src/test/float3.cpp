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
#include "math/float3.h"

TEST(Basic_Data_Structure_float3, ChannelChecking) {
    const float3  f( 1.0f , 2.0f , 3.0f );
    EXPECT_EQ( f[0] , 1.0f );
    EXPECT_EQ( f[1] , 2.0f );
    EXPECT_EQ( f[2] , 3.0f );
    EXPECT_EQ( f.x , 1.0f );
    EXPECT_EQ( f.y , 2.0f );
    EXPECT_EQ( f.z , 3.0f );
    EXPECT_EQ( f.r , 1.0f );
    EXPECT_EQ( f.g , 2.0f );
    EXPECT_EQ( f.b , 3.0f );
    EXPECT_EQ( f.w , 0.0f );
}

TEST(Basic_Data_Structure_float3, BasicOperation) {
    const float3  f0( 1.0f , 2.0f , 3.0f );
    const float3  f1( 5.0f , 7.0f , 10.0f );
    const auto f = f0 + f1;

    EXPECT_EQ( f[0] , 6.0f );
    EXPECT_EQ( f[1] , 9.0f );
    EXPECT_EQ( f[2] , 13.0f );
}