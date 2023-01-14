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

#include "core/define.h"
#include "thirdparty/gtest/gtest.h"
#include "core/memory.h"
#include "unittest_common.h"

using namespace unittest;

TEST(Memory, AlignedAllocation) {
    int i = 0;
    while( i++ < 10 ){
        for( auto k = 1 ; k <= 2 ; ++k ){
            // alignment
            const auto alignment = k * 16;

            // allocate the aligned memory
            auto* ret = malloc_aligned( 1024 , alignment );

            // the memory should be aligned
            EXPECT_EQ( ((uintptr_t)ret) % alignment , (uintptr_t)0 );

            // free the memory
            free_aligned( ret );
        }
    }
}

TEST(Memory, AlignedAllocation_ZeroInput) {
    // allocate the aligned memory
    auto* ret = malloc_aligned( 0 , 32 );

    // there should be no memory allocated.
    EXPECT_EQ( (void*)ret , (void*)nullptr );

    // this line should do nothing.
    free_aligned( ret );
}