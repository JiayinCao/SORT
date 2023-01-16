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

#include "thirdparty/gtest/gtest.h"
#include "unittest_common.h"
#include "job/scheduler.h"

using namespace unittest;

//! @brief  Helper class that setup the bind and unbind for the class
struct SchedulerWrapper {
    Scheduler scheduler;

    SchedulerWrapper() {
        scheduler.Bind();
    }

    ~SchedulerWrapper() {
        scheduler.Unbind();
    }
};

// This test should at least pass without inifitely waiting
TEST(SchedulerTest, EmptyScheduling) {
    SchedulerWrapper sw;

    sw.scheduler.Begin();
    sw.scheduler.Stop();
}

// This test should at least pass without inifitely waiting
TEST(SchedulerTest, SingleTaskScheduling) {
    SchedulerWrapper sw;

    int k = 0;

    sw.scheduler.Begin();
    schedule_parallel([&]() {
        EXPECT_EQ(k, 0);
        ++k;
    });
    sw.scheduler.Stop();

    EXPECT_EQ(k, 1);
}