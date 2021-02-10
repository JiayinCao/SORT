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

#include <memory>
#include "work.h"

RenderContext* Work::pullRenderContext() {
    // make sure only one thread is accessing this
    std::lock_guard<std::mutex> lock(m_rc_mutex);

    // if we are running out of render context, just create one
    if (m_available_render_context.empty()) {
        // make sure it is initialized after it is born
        std::unique_ptr<RenderContext> rc = std::make_unique<RenderContext>();
        rc->Init();

        RenderContext* pRc = rc.get();
        m_render_context_pool.push_back(std::move(rc));
        m_available_render_context.push_back(pRc);
    }

    auto ret = m_available_render_context.back();
    m_available_render_context.pop_back();

    m_running_render_context.insert(ret);

    // make sure this is a brand new render context before returning it
    return &(ret->Reset());
}

//! @brief  Recycle render context
void Work::recycleRenderContext(RenderContext* pRc) {
    // make sure only one thread is accessing this
    std::lock_guard<std::mutex> lock(m_rc_mutex);

    m_running_render_context.erase(pRc);
    m_available_render_context.push_back(pRc);
}