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

#include "image_evaluation.h"
#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/waitgroup.h"

void ImageEvaluation::StartRunning(int argc, char** argv, IStreamBase& stream) {

    // this is really just to make sure it compiles and runs well on all platforms
    // this will be deleted immediately after I verify it.
    {
        // Create a marl scheduler using the 4 hardware threads.
        // Bind this scheduler to the main thread so we can call marl::schedule()
        marl::Scheduler::Config cfg;
        cfg.setWorkerThreadCount(4);

        marl::Scheduler scheduler(cfg);
        scheduler.bind();
        defer(scheduler.unbind());  // Automatically unbind before returning.

        constexpr int numTasks = 10;

        // Create an event that is manually reset.
        marl::Event sayHello(marl::Event::Mode::Manual);

        // Create a WaitGroup with an initial count of numTasks.
        marl::WaitGroup saidHello(numTasks);

        // Schedule some tasks to run asynchronously.
        for (int i = 0; i < numTasks; i++) {
            // Each task will run on one of the 4 worker threads.
            marl::schedule([=] {  // All marl primitives are capture-by-value.
                // Decrement the WaitGroup counter when the task has finished.
                defer(saidHello.done());

                printf("Task %d waiting to say hello...\n", i);

                // Blocking in a task?
                // The scheduler will find something else for this thread to do.
                sayHello.wait();

                printf("Hello from task %d!\n", i);
                });
        }

        sayHello.signal();  // Unblock all the tasks.

        saidHello.wait();  // Wait for all tasks to complete.

        printf("All tasks said hello.\n");

        // All tasks are guaranteed to complete before the scheduler is destructed.
    }
}

int ImageEvaluation::WaitForWorkToBeDone() {
    return 0;
}