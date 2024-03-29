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
#include "job/fiber.h"

using namespace unittest;

// Convert the current thread to a fiber
TEST(Fiber, ConvertCurrentThreadToFiber) {
    auto thread_fiber = createFiberFromThread();
    EXPECT_TRUE(thread_fiber != nullptr);
}

// Create a new fiber
TEST(Fiber, CreateNewFiber) {
    auto new_fiber = createFiber(4096);
    EXPECT_TRUE(new_fiber != nullptr);

#if SORT_FIBER_ASM_IMPLEMENTATION
    EXPECT_NE(new_fiber->m_stack_ptr, nullptr);
    EXPECT_EQ(new_fiber->m_target_func, nullptr);
#endif

    new_fiber = createFiber(0, [](){});
    EXPECT_TRUE(new_fiber == nullptr);
}

// Swap fibers from one to another
TEST(Fiber, SwapFiber) {
    std::unique_ptr<Fiber> fiber0, fiber1, fiber2, thread_fiber;
    std::string str = "a";
    float flt = 1.0f;

    // capture the current thread so that we can swap
    thread_fiber = createFiberFromThread();

    fiber0 = createFiber(4096, 
        [&](){
            float local_flt = 1.f;

            EXPECT_EQ(str, "a");
            EXPECT_EQ(flt, 1.f);
            str += "b";
            flt += 1.f;

            local_flt += 2.f;
            switchFiber(fiber0.get(), fiber1.get());

            EXPECT_EQ(local_flt, 3.f);
            switchFiber(fiber0.get(), thread_fiber.get());
        }
    );

    fiber1 = createFiber(4096, 
        [&](){
            EXPECT_EQ(str, "ab");
            EXPECT_EQ(flt, 2.f);
            str += "c";
            flt += 2.f;
            switchFiber(fiber1.get(), fiber2.get());
        }
    );

    fiber2 = createFiber(4096, 
        [&](){
            EXPECT_EQ(str, "abc");
            EXPECT_EQ(flt, 4.f);
            str += "d";
            flt += 4.f;
            switchFiber(fiber2.get(), thread_fiber.get());
        }
    );

    // swap to that fiber
    switchFiber(thread_fiber.get(), fiber0.get());

    EXPECT_EQ(str, "abcd");
    EXPECT_EQ(flt, 8.f);

    // swap to the first fiber again
    switchFiber(thread_fiber.get(), fiber0.get());
}

// Swap within one fiber multiple times to make sure it consumes
TEST(Fiber, MultiSwapFiber) {
    std::unique_ptr<Fiber> fiber, thread_fiber;
    std::string str = "a";

    // capture the current thread so that we can swap
    thread_fiber = createFiberFromThread();

    fiber = createFiber(4096, 
        [&](){
            int k = 0;
            std::string localcopy = str;
            const static std::string s[] = { "b", "c", "d" };

            while(k < 3){
                EXPECT_EQ(str, localcopy);
                str += s[k];
                localcopy += s[k];
                switchFiber(fiber.get(), thread_fiber.get());

                ++k;
            }
        }
    );

    // swap back to the fiber three times
    int k = 0;
    while(k++ < 3)
        switchFiber(thread_fiber.get(), fiber.get());

    EXPECT_EQ(str, "abcd");
}

// Multiple threads with each thread having a fiber running on itself.
TEST(Fiber, MultiThreadedFibers) {
    std::thread threads[3];

    for(int i = 0; i < 3; ++i){
        threads[i] = std::thread([](){
                std::unique_ptr<Fiber> fiber, thread_fiber;
                std::string str = "a";

                // capture the current thread so that we can swap
                thread_fiber = createFiberFromThread();

                fiber = createFiber(4096, 
                    [&](){
                        int k = 0;
                        std::string localcopy = str;
                        const static std::string s[] = { "b", "c", "d" };

                        while(k < 3){
                            EXPECT_EQ(str, localcopy);
                            str += s[k];
                            localcopy += s[k];
                            switchFiber(fiber.get(), thread_fiber.get());

                            ++k;
                        }
                    }
                );

                // swap back to the fiber three times
                int k = 0;
                while(k++ < 3)
                    switchFiber(thread_fiber.get(), fiber.get());

                EXPECT_EQ(str, "abcd");
            }
        );
    }

    for(int i = 0; i < 3; ++i){
        threads[i].join();
    }
}

// Fiber that can be shared across threads, not at the same time
TEST(Fiber, CrossThreadFibers) {
    std::string str = "a";

    std::thread thread0, thread1;
    std::unique_ptr<Fiber> thread0_fiber, thread1_fiber, current_thread_fiber, new_fiber;

    current_thread_fiber = createFiberFromThread();
    
    new_fiber = createFiber(4096, 
        [&](){
            EXPECT_EQ(str, "a");
            str += "b";
            switchFiber(new_fiber.get(), current_thread_fiber.get());

            EXPECT_EQ(str, "abc");
            str += "d";
            switchFiber(new_fiber.get(), thread0_fiber.get());

            EXPECT_EQ(str, "abcde");
            str += "f";
            switchFiber(new_fiber.get(), thread1_fiber.get());
        }
    );
    switchFiber(current_thread_fiber.get(), new_fiber.get());
    EXPECT_EQ(str, "ab");

    thread0 = std::thread([&](){
        thread0_fiber = createFiberFromThread();
        str += "c";
        switchFiber(thread0_fiber.get(), new_fiber.get());
    });
    thread0.join();
    EXPECT_EQ(str, "abcd");

    thread1 = std::thread([&](){
        thread1_fiber = createFiberFromThread();
        str += "e";
        switchFiber(thread1_fiber.get(), new_fiber.get());
    });
    thread1.join();
    EXPECT_EQ(str, "abcdef");
}