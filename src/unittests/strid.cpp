/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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
#include "core/strid.h"
#include "unittest_common.h"

using namespace unittest;

TEST(StringID, BasicChecking) {
    // Make sure sid generated in different ways result in the same value
    StringID    sid0( "hello world" );
    EXPECT_EQ( sid0 , SID( "hello world" ) );

    // Make sure sid generated the same ways result in the same value.
    StringID    sid1( "hello world" );
    EXPECT_EQ( sid1 , sid0 );

    // See if there is collision, passing this test mostly like means nothing,
    // but failing to pass the test is clearly wrong.
    StringID    sid2( "Hello world" );
    EXPECT_NE( sid0 , sid2 );
    EXPECT_NE( sid0 , SID( "Hello world") );
}

TEST(StringID, Invalid_StringID) {
    // Empty string should result in INVALID_SID
    EXPECT_EQ( SID("") , INVALID_SID );
}