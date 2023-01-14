/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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
#include "core/scene.h"
#include "material/tsl_system.h"
#include "stream/stream.h"

template<class Context>
struct ContextHolder {
    std::mutex                            m_rc_mutex;                     // a mutex to make sure pool is not accessed by two threads at the same time.
    std::vector<std::unique_ptr<Context>> m_context_pool;                 // this only controls the life time of the contexts
    std::list<Context*>                   m_available_context;            // the render context that are available
    std::unordered_set<Context*>          m_running_context;              // the render context that are being ran
};

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
    virtual ~Work() = default;

    //! @brief  Start work evaluation.
    //!
    virtual void    StartRunning(int argc, char** argv) = 0;

    //! @brief  Wait for the work evaluation to be done.
    //!
    //! Ideally, if the task system supports it, it should take over the ownership of the main thread 
    //! and converting it to a worker fiber. So that the rest of the system has no concept of main thread
    //! at all. However, this doesn't seem to be supported in marl. I will have to workaround it to turn
    //! the main thread into some sort of 'background' thread that gains lower priority.
    //! This function will work synchronizely and once it returns the control back, the whole work is considered
    //! done.
    virtual int     WaitForWorkToBeDone() = 0;

protected:
    Scene   m_scene;

    ContextHolder<RenderContext>            m_rc_holder;
    ContextHolder<ShaderCompilingContext>   m_sc_holder;

    template<class Context>
    Context* pullContext(ContextHolder<Context>& context_holder) {
        // make sure only one thread is accessing this
        std::lock_guard<std::mutex> lock(context_holder.m_rc_mutex);

        // if we are running out of render context, just create one
        if (context_holder.m_available_context.empty()) {
            // make sure it is initialized after it is born
            std::unique_ptr<Context> context = std::make_unique<Context>();
            context->Init();

            Context* pContext = context.get();
            context_holder.m_context_pool.push_back(std::move(context));
            context_holder.m_available_context.push_back(pContext);
        }

        auto ret = context_holder.m_available_context.back();
        context_holder.m_available_context.pop_back();

        context_holder.m_running_context.insert(ret);

        // make sure this is a brand new render context before returning it
        return &(ret->Reset());
    }

    template<class Context>
    void recycleContext(ContextHolder<Context>& context_holder, Context* pContext) {
        // make sure only one thread is accessing this
        std::lock_guard<std::mutex> lock(context_holder.m_rc_mutex);

        context_holder.m_running_context.erase(pContext);
        context_holder.m_available_context.push_back(pContext);
    }
};