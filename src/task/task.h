/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "core/singleton.h"

// Default task priority is 100000.
#define DEFAULT_TASK_PRIORITY       100000

using TaskID = unsigned int;

//! @brief 	Basic unit task in SORT system.
/**
 * SORT is driven by a graph based task system. The tasks form a directed acyclic graph (DAG).
 * Only tasks without dependencies will get executed. There will be no cycles in the graph, otherwise 
 * the system will hang without proceeding. A task can range from rendering a piece of image to loading 
 * data from streams. Upon finishing of each task, it will remove its dependencies. Each task comes 
 * with a priority number. Default priority is 100000, higher priority task will be executed earlier 
 * than lower ones.
 */
class Task{
public:
    // Dependency container for task
    using Task_Container = std::unordered_set<const Task*>;
    using DependentTask_Container = std::unordered_set<Task*>;

    //! @brief  Default constructor.
    Task(   const char* name  , unsigned int priority = DEFAULT_TASK_PRIORITY , 
            const Task_Container& dependencies = {} ):
            m_name(name), m_dependencies(dependencies),m_priority(priority) {
        static std::mutex m;
        static unsigned int taskId = 0;

        std::lock_guard<std::mutex> lock(m);
        m_taskId = (TaskID)++taskId;
    }

    //! @brief  Virtual destructor.
    virtual             ~Task() {}

    //! @brief  Execute the task
    virtual void        Execute() = 0;

    //! @brief  Execute the task, this also includes outputting profiling data and removing dependencies.
    void                ExecuteTask();

    //! @brief  Get priority of the task.
    //!
    //! @return             Current priority of the task.
    inline unsigned int GetPriority() const { 
        return m_priority; 
    }

    //! @brief  Remove dependency from task.
    //!
    //! Upon the termination of any dependent task, it is necessary to remove it from its dependency.
    inline void         RemoveDependency( const Task* taskid ) { 
        m_dependencies.erase( taskid ); 
    }

    //! @brief  If there is no dependent task anymore.
    //!
    //! @return True if all dependent tasks are finished. Otherwise, return false.
    inline bool         NoDependency() const { 
        return m_dependencies.empty(); 
    }

    //! @brief  Get tasks depending on this task.
    //!
    //! @return Tasks this task depends on.
    inline const DependentTask_Container& GetDependents() const  {
        return m_dependents; 
    }

    //! @brief  Add dependent.
    //!
    //! @param  Task to be added as a dependent.
    inline void AddDependent( Task* task ){
        m_dependents.insert( task );
    }

    //! @brief  Get the id of the task
    //!
    //! @return Id of the current task.
    inline TaskID GetTaskID() const {
        return m_taskId;
    }

    //! @brief  Get tasks this task depends on.
    //!
    //! @return Tasks this task depends on.
    inline Task_Container& GetDependencies(){
        return m_dependencies;
    }

private:
    Task_Container              m_dependencies;     /**< Tasks this task depends on. */
    DependentTask_Container     m_dependents;       /**< Tasks depending on this task. */
    unsigned int                m_priority;         /**< Priority of the task. */
    const std::string           m_name;             /**< Name of the task. */
    TaskID                      m_taskId;           /**< This is to identify the task with id. */
};

//! @brief  Scheduler for scheduling tasks.
/**
 * Scheduler will pick a task without any dependencies that has highest priority. If there are
 * multiple candidate tasks, a random one will be picked. Each task dependencies will only be
 * removed after it is fully finished, not after it gets started.
 * Scheduler is thread-safe, which means that multiple threads can retrieve tasks from scheduler
 * concurrently.
 */
class Scheduler : public Singleton<Scheduler>{
    /**< Task comparison functor. */
    using Task_Comp = std::function<bool(const Task* , const Task*)>;
    /**< Static task comparison functor based on its priority. */
    static Task_Comp task_comp;
    /**< Task queue for available tasks is actually a heap. */
    using TaskQueue = std::priority_queue<Task*,std::vector<Task*>,decltype(task_comp)>;
    /**< Task container for back-up tasks is just a hash container. */
    using BackupTaskContainer = std::unordered_set<Task*>;
    /**< Task container for keeping tasks alive. */
    using TaskContainer = std::unordered_map<TaskID, std::unique_ptr<Task>>;

public:
    //! @brief  Schedule a task.
    //!
    //! @param  task        Task to be scheduled.
    //! @param              Raw pointer to the task.
    Task*    Schedule( std::unique_ptr<Task> task );

    //! @brief  Pick a task with highest priority, but no dependencies.
    //!
    //! The scheduler will try picking a task with highest priority, but no dependencies.
    //! If there is no such a task available for now, the scheduler will hang the thread
    //! and share its CPU resources to other threads for executing. In the case of a cycle
    //! graph tasks, it will hang forever. The task picked will be removed from the data
    //! structures in scheduler.
    //! If there is no task in the scheduler, nullptr will be returned.
    //!
    //! @return    The task picked from scheduler.
    Task*   PickTask();

    //! @brief  Remove dependencies for a task.
    //!
    //! Upon finish of each task, it needs to update scheduler it is finished so that other
    //! tasks depending on this task will get chance to be executed in the future.
    //!
    //! @param task     Task that is finished. This task should not be in the scheduler.
    void    TaskFinished( const Task* task );
    
private:
    //! @brief  Default constructor
    Scheduler():m_availbleTasks(task_comp){}

    TaskQueue                   m_availbleTasks;        /**< Heap of available tasks. */
    BackupTaskContainer         m_backupTasks;          /**< Container for all tasks not direct available. */
    std::mutex                  m_mutex;                /**< Mutex to make sure scheduler is thread-safe. */
    std::condition_variable     m_cv;                   /**< Conditional variable for pick task. */
    TaskContainer               m_tasks;                /**< This holds all tasks to keep them alive. */

    friend class Singleton<Scheduler>;
};

//! @brief      Schedule a task in task scheduler.
template<class T, typename... Args>
inline Task*  SCHEDULE_TASK( const char* name , unsigned int priority , const Task::Task_Container& dependencies , Args&&... args ){
    auto ret = std::make_unique<T>(args..., name, priority, dependencies);
    return Scheduler::GetSingleton().Schedule( std::move(ret) );
}

//! @brief      Executing tasks. It will exit if there is no other tasks.
inline void    EXECUTING_TASKS(){
    while( true ){
        // Pick a task that is available.
        auto task = Scheduler::GetSingleton().PickTask();

        // If there is no task to be picked, break out of the loop.
        if( task == nullptr )
            return;

        // Execute the task.
        task->ExecuteTask();
    }
}
