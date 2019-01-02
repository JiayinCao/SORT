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

#include "task.h"
#include "core/sassert.h"
#include "core/profile.h"

void Task::ExecuteTask(){
    SORT_PROFILE(m_name);

    // Execute the task.
    Execute();

    // Upon termination of a task, release its dependents' dependencies on this task.
    Scheduler::GetSingleton().TaskFinished( shared_from_this() );
}

// Compare tasks based on its priority, tasks with higher priority get executed earlier.
Scheduler::Task_Comp Scheduler::task_comp = []( const std::shared_ptr<Task> t0 , const std::shared_ptr<Task> t1 ){
    return t0->GetPriority() < t1->GetPriority();
};

void Scheduler::Schedule( const std::shared_ptr<Task> task ){
    if( task == nullptr )
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    if( task->NoDependency() )
        m_availbleTasks.push( task );
    else{
        auto dependencies = task->GetDependencies();
        for( auto dep : dependencies )
            dep->AddDependent( task );
        m_backupTasks.insert( task );
    }
}

std::shared_ptr<Task> Scheduler::PickTask(){
    std::unique_lock<std::mutex> lock(m_mutex);

    // Wait until this is at least one available task
    m_cv.wait( lock , [&](){ return (m_backupTasks.empty() && m_availbleTasks.empty()) || m_availbleTasks.size(); } );

    // Return nullptr if there is no task available in the scheduler
    if (m_backupTasks.empty() && m_availbleTasks.empty())
        return nullptr;

    // Get the available task that is with highest priority
    std::shared_ptr<Task> ret = m_availbleTasks.top();
    m_availbleTasks.pop();
    return ret;
}

void Scheduler::TaskFinished( const std::shared_ptr<Task> task ){
    std::lock_guard<std::mutex> lock(m_mutex);

    // Starting remove all dependencies.
    const auto dependent = task->GetDependents();
    for( auto dep : dependent ){
        // Remove its dependencies.
        dep->RemoveDependency( task );

        // There is no dependent task of this 'dep' task anymore, push it into the heap and remove it from the backup tasks.
        if( dep->NoDependency() ){
            m_backupTasks.erase( dep );
            m_availbleTasks.push( dep );

            // Notify one waiting thread to pick up task.
            m_cv.notify_one();
        }
    }
}