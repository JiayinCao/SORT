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

#include "../work.h"

//! @brief  Generating an image using ray tracing algorithms.
/**
 * This class has all the image generation specific logic inside, including parsing streamed input,
 * spawning render tasks for image tiles, storing the results to an image and send it to display server
 * through sockets if needed.
 */
class ImageEvaluation : public Work {
public:
    DEFINE_RTTI(ImageEvaluation, Work);

    //! @brief  Start work evaluation.
    //!
    //! @param stream       The stream as input.
    void    StartRunning(int argc, char** argv, IStreamBase& stream) override;

    //! @brief  Wait for the work evaluation to be done.
    //!
    //! Ideally, if the task system supports it, it should take over the ownership of the main thread 
    //! and converting it to a worker fiber. So that the rest of the system has no concept of main thread
    //! at all. However, this doesn't seem to be supported in marl. I will have to workaround it to turn
    //! the main thread into some sort of 'background' thread that gains lower priority.
    //! This function will work synchronizely and once it returns the control back, the whole work is considered
    //! done.
    int     WaitForWorkToBeDone() override;

private:
};