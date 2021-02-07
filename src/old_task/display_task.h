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

#include "task.h"

//! @brief  Display_Task is for displaying the intermediate result in display servers
/**
 * Ideally, this should be a low priority background task. But I don't have it supported yet.
 */
class Display_Task : public old_task::Task{
public:
    //! @brief Constructor
    //!
    //! @param priority     New priority of the task.
    Display_Task(const char* name , unsigned int priority , const old_task::Task::Task_Container& dependencies);

    //! @brief  Execute the task
    void        Execute() override;
};