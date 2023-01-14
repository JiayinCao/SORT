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

#pragma once

#include <atomic>
#include <marl/scheduler.h>
#include "work/work.h"
#include "core/timer.h"
#include "integrator/integrator.h"
#include "texture/rendertarget.h"

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
    void    StartRunning(int argc, char** argv) override;

    //! @brief  Wait for the work evaluation to be done.
    //!
    //! Ideally, if the task system supports it, it should take over the ownership of the main thread 
    //! and converting it to a worker fiber. So that the rest of the system has no concept of main thread
    //! at all. However, this doesn't seem to be supported in marl. I will have to workaround it to turn
    //! the main thread into some sort of 'background' thread that gains lower priority.
    //! This function will work synchronizely and once it returns the control back, the whole work is considered
    //! done.
    int     WaitForWorkToBeDone() override;

    //! @bried  Update image
    //!
    //! This is only for bidirectional path tracing and light tracing.
    void    UpdateImage(const Vector2i& coord, const Spectrum& value);

private:
    // Input file name
    std::string     m_input_file;
    // image title, this is only for TEV
    std::string     m_image_title;
    // Blender mode
    bool            m_blender_mode = false;
    // Enable profiling
    bool            m_enable_profiling = false;
    // No material mode
    bool            m_no_material_mode = false;
    // whether we need a render target
    bool            m_need_render_target = false;

    std::string     m_resource_path;            // resource path
    std::string     m_display_server_ip;        // display server ip
    std::string     m_display_server_port;      // display server port
    bool            m_has_display_server;       // whether it has a display server
    unsigned        m_thread_cnt = 6;           // thread cnt
    unsigned        m_sample_per_pixel = 16;    // sample per pixel to be evaluated.
    unsigned        m_image_width = 0;          // width of the image to be generated
    unsigned        m_image_height = 0;         // height of the image to be generated
    float           m_clampping = 0.0f;         // radiance can't go higher than this, this is the cheapest way to do firefly reduction.

    std::unique_ptr<Integrator>         m_integrator;       // the algorithm used for ray tracing
    std::atomic<int>                    m_tile_cnt;         // number of total tiles
    std::unique_ptr<RenderTarget>       m_render_target;    // a temporary buffer for saving out the result
    std::unique_ptr<marl::Scheduler>    m_scheduler;        // job system scheduler
    std::mutex                          m_image_lock;       // image lock, ideally we should have a lock for each pixel
    Timer                               m_timer;            // timer to evaluate the rendering time.

    void    parseCommandArgs(int argc, char** argv);
    void    loadConfig(IStreamBase& stream);
};