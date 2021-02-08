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

#include "core/rtti.h"
#include "stream/stream.h"

//! @brief  Work to be evaluated.
/**
 * Work is just a set of evaluations to be performed. Commonly, it will parse some sort of inputs,
 * do some ray tracing evaluation and then store the results to somewhere.
 *
 * This is a new abstractuion I added in my renderer to isolate all image generation logic so that
 * the renderer could be used in other applications like light probe evaluation, light map generation,
 * instead of just image generation.
 *
 * By the time this code was written, only one single instance of work is supposed to be running each time
 * SORT gets run. But this could be extended for other purposes in the future to allow more possibilities.
 */
class Work {
public:
    //! @brief  Make sure the destructor is virtual
    ~Work() = default;

    //! @brief  Start work evaluation.
    //!
    //! @param stream       The stream as input.
    virtual void    StartRunning(int argc, char** argv, IStreamBase& stream) = 0;

    //! @brief  Wait for the work evaluation to be done.
    //!
    //! Ideally, if the task system supports it, it should take over the ownership of the main thread 
    //! and converting it to a worker fiber. So that the rest of the system has no concept of main thread
    //! at all. However, this doesn't seem to be supported in marl. I will have to workaround it to turn
    //! the main thread into some sort of 'background' thread that gains lower priority.
    //! This function will work synchronizely and once it returns the control back, the whole work is considered
    //! done.
    virtual int     WaitForWorkToBeDone() = 0;
};