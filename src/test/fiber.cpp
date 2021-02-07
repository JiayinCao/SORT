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
#include "task/fiber.h"

#ifdef SORT_IN_WINDOWS

void fiber_empty_func(void* king) { }

void create_destroy_fiber() {
    Fiber fiber;
    FiberInitDesc desc;
    desc.m_stack_size = 1024;
    desc.m_fiber_func = fiber_empty_func;
    auto ret = create_fiber(desc, fiber);
    EXPECT_TRUE(ret);

    ret = create_fiber(desc, fiber);
    EXPECT_FALSE(ret);

    ret = delete_fiber(fiber);
    EXPECT_TRUE(ret);

    ret = delete_fiber(fiber);
    EXPECT_FALSE(ret);
}

void takeover_fiber(void* fiber) {
    // we are in control now, why don't we just do something and return it to the main fiber.
    create_destroy_fiber();

    // give back the control
    switch_to_fiber(*(Fiber*)fiber);
}

static std::string g_test_str = "";
void append_string(void* fiber) {
    static int k = 12;
    g_test_str += std::to_string(k++);

    // give back the control
    switch_to_fiber(*(Fiber*)fiber);
}

TEST(FIBER, CreateAndDelete) {
    create_destroy_fiber();
}

TEST(FIBER, TakeOwnerShip) {
    // convert this thread to a fiber so that we can transfer the ownership to the fiber we just created.
    Fiber main_fiber;
    fiber_from_thread(main_fiber);

    // create another fiber
    Fiber fiber;
    FiberInitDesc desc;
    desc.m_stack_size = 1024;
    desc.m_fiber_func = takeover_fiber;
    desc.m_fiber_arg = &main_fiber;
    auto ret = create_fiber(desc, fiber);
    EXPECT_TRUE(ret);

    // switch to the created fiber
    switch_to_fiber(fiber);

    // restore the thread
    thread_from_fiber();

    // delete the fiber
    ret = delete_fiber(fiber);
    EXPECT_TRUE(ret);
}

TEST(FIBER, MultipleFibers) {
    // convert this thread to a fiber so that we can transfer the ownership to the fiber we just created.
    Fiber main_fiber;
    fiber_from_thread(main_fiber);

    // create another fiber
    Fiber fiber0;
    FiberInitDesc desc0;
    desc0.m_stack_size = 1024;
    desc0.m_fiber_func = append_string;
    desc0.m_fiber_arg = &main_fiber;
    auto ret = create_fiber(desc0, fiber0);
    EXPECT_TRUE(ret);

    // create a third fiber
    Fiber fiber1;
    FiberInitDesc desc1;
    desc1.m_stack_size = 1024;
    desc1.m_fiber_func = append_string;
    desc1.m_fiber_arg = &fiber0;
    ret = create_fiber(desc1, fiber1);
    EXPECT_TRUE(ret);

    // switch to the created fiber
    switch_to_fiber(fiber1);

    // restore the thread
    thread_from_fiber();

    const auto str = g_test_str.c_str();
    EXPECT_EQ(str[0], '1');
    EXPECT_EQ(str[1], '2');
    EXPECT_EQ(str[2], '1');
    EXPECT_EQ(str[3], '3');
}

#endif