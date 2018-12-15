/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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

Task::Task( unsigned int priority , const std::unordered_set<std::shared_ptr<Task>>& dependencies ) 
 : m_priority(priority) {
    for( auto dep : dependencies ){
       // Add tasks this task depend on.
       m_dependencies.insert( dep->GetTaskID() );
       // Make sure this task also exist in its depencies dependent list.
       dep->m_dependents.insert( m_taskid );
    }

    static unsigned int id_counter = 0;
    m_taskid = id_counter++;
}

// Compare tasks based on its priority, tasks with higher priority get executed earlier.
Scheduler::Task_Comp Scheduler::task_comp = []( const std::shared_ptr<Task> t0 , const std::shared_ptr<Task> t1 ){
    return t0->GetPriority() < t1->GetPriority();
};

Scheduler::Scheduler():m_availbleTasks(task_comp){
}

void Scheduler::Schedule( const std::shared_ptr<Task> task ){
    if( task == nullptr )
        return;

    std::lock_guard<std::mutex> lock(m_mutex);
    if( task->NoDependency() )
        m_availbleTasks.push( task );
    else
        m_backupTasks.insert( make_pair( task->GetTaskID() , task ) );

    sAssert( 0 == m_allTasks.count( task->GetTaskID() ) , TASK );
    m_allTasks[task->GetTaskID()] = task;
}

std::shared_ptr<Task> Scheduler::PickTask(){
    std::unique_lock<std::mutex> lock(m_mutex);

    // Return nullptr if there is no task available in the scheduler
    if( m_backupTasks.empty() && m_availbleTasks.empty() )
        return nullptr;

    // Wait until this is at least one availble task
    m_cv.wait( lock , [&](){ return m_availbleTasks.size(); } );

    // Get the available task that is with highest priority
    std::shared_ptr<Task> ret = m_availbleTasks.top();
    m_availbleTasks.pop();
    return ret;
}

void Scheduler::TaskFinished( const std::shared_ptr<Task> task ){
    std::lock_guard<std::mutex> lock(m_mutex);

    // starting remove all dependencies.
    const auto dependent = task->GetDependents();
    for( auto id : dependent ){
        auto dep = m_backupTasks[id];
        // Remove its dependencies.
        dep->RemoveDependency( task->GetTaskID() );

        // There is no dependent task of this 'dep' task anymore, push it into the heap and remove it from the backup tasks.
        if( dep->NoDependency() ){
            m_backupTasks.erase( dep->GetTaskID() );
            m_availbleTasks.push( dep );

            // Notify one waiting thread to pick up task.
            m_cv.notify_one();
        }
    }
}