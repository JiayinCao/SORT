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

#pragma once

#include <map>
#include <vector>
#include <set>

using Task_Priority = unsigned int;

struct TaskDesc {

};

//! @brief  Task system in SORT.
/**
 * Task system is responsible for scheduling tasks in the renderer. In game engines, it is commonly called
 * job system. Since job is reserved for another concept that I plan to implement in the renderer, this is
 * named differetly. And the essential functionalities are the same, to schedule tasks/jobs on multiple 
 * threads so that the renderer/game engine could be code it in a moduler way.
 *
 * This implementaion is heavily inspired by this talk in GDC 2015.
 * 'Parallelizing the Naughty Dog Engine Using Fibers'
 * https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
 *
 * NOTE, this implementation is what I came up with by watching the GDC talk. It has absolutely nothing to do
 * with Naughty Dog's in-house engine.
 */
class TaskSystem {
public:
    //! @brief  Initialize the task system.
    //!
    //! Initialization of the tasksystem, like setting up fibers, etc.
    void Initialize();

    //! @brief  Triggering the task system, multi-thread starts after this function gets called.
    //!
    //! Under the hood, this will spawn N - 1 number of threads to be running at the same time.
    //! N is the number of physical cores on the CPU.
    //! Also it will take over the owner ship of the main thread and convert it to a worker thread like others.
    //! This will freeze the main thread, but not by hanging it, but to sliently covert it from a main thread
    //! to a regular working fiber just like others so that the renderer doesn't even have a concept of main
    //! thread most of the time.
    //! This function will only give control back to the calling routine if requested, which commonly indicates
    //! the ray tracing evaluation is done.
    void StartRunning();

private:
    std::map<Task_Priority, TaskDesc>   m_new_tasks;        // These are the new tasks that are not being executed.
    std::map<Task_Priority, TaskDesc>   m_awake_tasks;      // These are tasks that have been woken up already.
    std::vector<TaskDesc>               m_waiting_tasks;    // These tasks have something pending on other tasks and they are currently paused.
    std::set<TaskDesc>                  m_low_freq_tasks;   // Low frequent tasks for IO operations in the middle of rendering.
};