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

#include <atomic>
#include <thread>
#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include "core/define.h"
#include "core/sassert.h"
#include "fiber.h"

//! @brief  Data structure to config the scheduler
struct SchedulerConfig {
    SchedulerConfig();

    /* Number of slave thread needed, by default it is the number of hardware
       cores. If there are hybrid thread available, it would count them as 
       multiple as well. */
    unsigned int    slave_thread_cnt;

    /* Stack size for regular slave. */
    unsigned int    slave_fiber_stack_size;
};

//! @brief  Task
struct Task {
    using Function = std::function<void()>;

    SORT_FORCEINLINE Task() : function(nullptr) {}
    SORT_FORCEINLINE Task(const Function& function) : function(function) {}
    SORT_FORCEINLINE Task(Function&& function) : function(std::move(function)) {}

    SORT_FORCEINLINE void operator()() const { function(); }

    /**< Target function. */
    Function   function;
};

//! @brief TaskContext is the place that task runs on.
/**
 * The scheduler has to find a task context before executing the task. Without a
 * task context, the task can't be executed. In theory there could be unlimited
 * number of task context. However, in practice, it is limited by its memory 
 * usage, which is not too much actually, around 4k stack memory for fibers and
 * potentially another arena for memory allocation in TSL.
 * Task context is reused once a task is done executing and the context can be 
 * shared with a next task that the scheduler picks up.
 */
struct TaskContext {
    /**< The fiber the task will run on. */
    std::unique_ptr<Fiber>  fiber;

    /**< Status of the context. */
    enum class TCStatus : unsigned char {
        Idle,
        Executing,
        Pending,
        Paused
    };
    TCStatus status = TCStatus::Idle;

    /**< The task the context will be executing. */
    const Task* task = nullptr;

    /**< Cached scheduler. */
    class Scheduler* scheduler = nullptr;
};

//! @brief  Scheduler of the job system in SORT
/**
 * This is specifically for scheduling jobs in SORT. To explain it on a high level,
 * the scheduler would allocate N + 1 threads, with N being precisely the number of
 * physically cores available ( counting Intel hybrid tech ), the extra one core is
 * specifically for IO and it should rarely be activated. For the N threads, I call 
 * them slave threads. They keep picking tasks from the task pool and feed 
 * themselves with it.
 * 
 * At a very ligh level, following things are supported in this job system.
 *     - Schedule as many tasks as possible without memory overhead
 *     - Proper load balancing to make sure the tasks are well distributed
 *     - Task can be scheduled during another task's execution
 *     - Task can yield at any time at minimal costs
 *     - The specific algorithm to pick a new task running when one is yielding
 *       is defined in SORT, rather than the OS.
 */
class Scheduler {
public:
    //! @brief  Destructor
    ~Scheduler();

    //! @brief  Setup the scheduler
    //!
    //! Only N thread are actually allocated, with N-1 being the slave thread and
    //! one extra thread being the IO thread. Be noted that there is the main 
    //! thread existing as well. The main thread will be converted to a slave thread
    //! as well, making it exactly N slave threads together.
    void SetupConfig(const SchedulerConfig& cfg);

    //! @brief  Bind the current scheduler.
    //! 
    //! At one time, there can be only one single instance of scheduler bound.
    void Bind();

    //! @brief  Unbind the current scheduler instance.
    //! 
    //! Unbind the current scheduler so that the next scheduler can be bound.
    void Unbind();

    //! @brief  Start the scheduler
    void Begin();

    //! @brief  Stop the scheduling
    void Stop();

    //! @brief  Enqueue a task
    void Enqueue(Task&& task);

    //! @brief  Get the currently bound scheduler
    static Scheduler* GetBound() {
        sAssertMsg(IS_PTR_VALID(s_bound_scheduler), SCHEDULER, "No scheduler was bound before.");
        return s_bound_scheduler;
    }

    //! @brief  Slave worker is the virtual context for task execution.
    struct SlaveWorker {
        /**< Owner scheduler. */
        Scheduler*              scheduler;

        /**< Thread background fiber. All slave worker should get back to this fiber
             right after they finish their work. */
        std::unique_ptr<Fiber>  thread_fiber;

        /**< Background fiber that keeps pulling tasks. This is not entirely needed, but I like
             the way it hides the mainthread and making all the threads identical leaving no special
             main thread. */
        std::unique_ptr<Fiber>  background_fiber;

        /**< Dedicated thread for this slaveworker. */
        std::thread             thread;

        //! @brief  Initialize the slave worker.
        void InitializeSlaveWorker(Scheduler* scheduler);
    };

private:
    /**< A copy of the configuration of the current fiber. */
    SchedulerConfig                     m_config;

    /**< Slave workers. */
    std::vector<SlaveWorker>            m_slaves;

    /**< Task context pool for keeping its life time. */
    std::list<std::unique_ptr<TaskContext>> m_tc_pool;

    /**< Available task context pool. */
    std::list<TaskContext*>             m_idle_tc_pool;
    /**< Mutex to guard fiber pool. */
    std::mutex                          m_tc_pool_mutex;

    /**< Task pool. */
    std::list<std::unique_ptr<Task>>    m_pending_task_pool;
    /**< Mutex to guard task pool. */
    std::mutex                          m_task_mutex;

    /**< Task number. */
    std::atomic<int>                    m_total_task_cnt = 0;
    std::atomic<int>                    m_total_pending_task = 0;

    /**< IO thread. */
    std::thread                         m_io_thread;

    /**< The currently bound scheduler. */
    inline static Scheduler*            s_bound_scheduler = nullptr;

    //! @brief      Helper function to acquire a task context.
    TaskContext*  acquireTaskContext();

    //! @brief      Helper function to switch to a new fiber
    void    switchToFiber(Fiber* fiber);

    //! @brief      Pop a task that is pending
    std::unique_ptr<Task>   pullTask();
};

//! @brief  Schedule a task in the job system
template <typename Function, typename... Args>
SORT_FORCEINLINE void schedule_parallel(Function&& f, Args&&... args) {
    Scheduler::GetBound()->Enqueue(
        Task(std::bind(std::forward<Function>(f), 
                       std::forward<Args>(args)...))
    );
}