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

#include <algorithm>
#include "scheduler.h"

//! @brief  Threaded based data for slave worker context.
//! 
//! This is the best way that I can think of to store the data mainly for accessibility
//! reason. Code doesn't need to acqauire the instance of scheduler to find the current
//! fiber.
//! And this thread local storage is indeed for per-thread information, rather than 
//! per fiber information. So it is totally fine to use it here.
struct SlaveWorkerContext {
    /**< Currently executing fiber. */
    Fiber* current_fiber = nullptr;

    /**< Background fiber. */
    Fiber* background_fiber = nullptr;

    /**< Thread fiber. */
    Fiber* thread_fiber = nullptr;
};
thread_local SlaveWorkerContext   g_slaveworker_context;

void Scheduler::SlaveWorker::InitializeSlaveWorker(Scheduler* scheduler) {
    // The first thing each thread would do is to conver the current thread into a fiber
    thread_fiber = createFiberFromThread();

    // Create the back ground fiber that pulls task all the time
    background_fiber = createFiber(scheduler->m_config.slave_fiber_stack_size, 
        [this, scheduler](){
            while(auto task = scheduler->pullTask()){
                auto tc = scheduler->acquireTaskContext();
                tc->task = task.get();

                // switch to the fiber for execution
                scheduler->switchToFiber(tc->fiber.get());

                // if the tc is in idle status, release the lock
                if(tc->status == TaskContext::TCStatus::Idle)
                    scheduler->m_tc_pool_mutex.unlock();
            }
            scheduler->switchToFiber(g_slaveworker_context.thread_fiber);
        }
    );

    // Keep track of the current executing fiber
    g_slaveworker_context.current_fiber = thread_fiber.get();
    g_slaveworker_context.background_fiber = background_fiber.get();
    g_slaveworker_context.thread_fiber = thread_fiber.get();

    // switch to a fiber for tasks
    scheduler->switchToFiber(background_fiber.get());
}

SchedulerConfig::SchedulerConfig() {
    // As SORT's job system uses cooperative threading, there is no need to allocate
    // more threads than what is available on hardware.
    slave_thread_cnt = std::thread::hardware_concurrency();

    // By default each slave thread fiber has 4k memory.
    slave_fiber_stack_size = 4 * 1024;
}

Scheduler::~Scheduler() {
    sAssertMsg(this != s_bound_scheduler, SCHEDULER, "Did you forget to unbind the scheduler?");
}

void Scheduler::SetupConfig(const SchedulerConfig& cfg) {
    m_config = cfg;
}

void Scheduler::Bind() {
    sAssertMsg(IS_PTR_INVALID(s_bound_scheduler), SCHEDULER, "There is an existing scheduler bound already.");
    s_bound_scheduler = this;
}

void Scheduler::Unbind() {
    sAssertMsg(IS_PTR_VALID(s_bound_scheduler), SCHEDULER, "No scheduler was bound before.");
    s_bound_scheduler = nullptr;
}

void Scheduler::Begin() {
    // Allocate the slave workers
    m_slaves.resize(m_config.slave_thread_cnt);

    // Spawning new thread starting from the second slave worker, the first one is reserved
    // for the main thread, which will be converted to a slave worker as well.
    for (auto i = 1u; i < m_slaves.size(); ++i) {
        m_slaves[i].thread = std::thread([this, i]() {
            m_slaves[i].InitializeSlaveWorker(this);
        });
    }

    // Convert the current main thread to a slave worker thread
    m_slaves[0].InitializeSlaveWorker(this);
}

void Scheduler::Stop() {
    for (auto i = 1u; i < m_slaves.size(); ++i)
        m_slaves[i].thread.join();
}

void Scheduler::Enqueue(Task&& task) {
    std::lock_guard<std::mutex> lock(m_task_mutex);
    m_pending_task_pool.emplace_back(std::make_unique<Task>(std::move(task)));

    ++m_total_task_cnt;
    ++m_total_pending_task;
}

TaskContext* Scheduler::acquireTaskContext() {
    std::lock_guard<std::mutex> lock(m_tc_pool_mutex);

    if (m_idle_tc_pool.empty()) {
        std::unique_ptr<TaskContext>    tc = std::make_unique<TaskContext>();
        auto tc_ptr = tc.get();
        tc->fiber = createFiber(m_config.slave_fiber_stack_size, [this, tc_ptr](){
            while(true){
                tc_ptr->status = TaskContext::TCStatus::Executing;

                if(tc_ptr->task->function)
                    tc_ptr->task->function();
                
                // reset the task context state to prevent reuse
                tc_ptr->status = TaskContext::TCStatus::Idle;

                // Since the task is done, feed task context back to the pool for reuse
                // before doing that acquire the mutex first.
                // Note, this can't be done inside recycleTaskContext as the mutex has to
                // be held after the fiber switch. Otherwise, it is possible that before
                // the fiber switch happens, some other threads take away this idle 
                // task context and cause incorrect fiber switch first.
                Scheduler::GetBound()->m_tc_pool_mutex.lock();
                Scheduler::GetBound()->recycleTaskContext(tc_ptr);

                // we are done with the task, now return
                Scheduler::GetBound()->switchToFiber(g_slaveworker_context.background_fiber);
            }
        });

        m_tc_pool.push_back(std::move(tc));

        m_idle_tc_pool.push_back(m_tc_pool.back().get());
    }

    auto ret = m_idle_tc_pool.back();
    m_idle_tc_pool.pop_back();
    return ret;
}

void Scheduler::recycleTaskContext(TaskContext* tc){
    m_idle_tc_pool.push_back(tc);
}

void Scheduler::switchToFiber(Fiber* fiber) {
    // All switch fiber should be called here in the scheduler as this will gurrantee
    // that we always switch from the correct fiber, which is needed on Mac and Ubuntu
    // fiber implementation.
    Fiber* source_fiber = g_slaveworker_context.current_fiber;
    g_slaveworker_context.current_fiber = fiber;
    switchFiber(source_fiber, fiber);
}

std::unique_ptr<Task> Scheduler::pullTask() {
    std::lock_guard<std::mutex> lock(m_task_mutex);
    if (m_total_task_cnt == 0)
        return nullptr;
    --m_total_task_cnt;

    std::unique_ptr<Task> ptr = std::move(m_pending_task_pool.back());
    auto tt = std::move(m_pending_task_pool.back());
    m_pending_task_pool.pop_back();

    return std::move(ptr);
}