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

#pragma once

#include "task.h"
#include "sampler/sampler.h"
#include "math/vector2.h"
#include "core/scene.h"

//! @brief  Render_Task is a basic rendering unit doing ray tracing.
//!
//! Each render task is usually responsible for a tile of image to be rendered in
//! most cases. In other cases, like light tracing, there is no difference between
//! different render_task.
class Render_Task : public Task{
public:
    //! @brief Constructor
    //!
    //! @param priority     New priority of the task.
    Render_Task(const Vector2i& ori , const Vector2i& size , const Scene& scene ,
                const char* name , unsigned int priority , const std::unordered_set<std::shared_ptr<Task>>& dependencies );
    
    //! @brief  Execute the task
    void        Execute() override;

    //! @brief  Get the coordinate of the tile, top-left corner.
    //!
    //! @return Top-left corner of the tile.
    inline Vector2i    GetTopLeft() const {
        return m_coord;
    }
    
    //! @brief  Get the size of the tile.
    //!
    //! @return The size of the current tile.
    inline Vector2i    GetTileSize() const {
        return m_size;
    }

private:
    Vector2i                            m_coord;            /**< Top-left corner of the current tile. */
    Vector2i                            m_size;             /**< Size of the current tile to be rendered. */
    const Scene&	                    m_scene;            /**< Scene for ray tracing. */
    std::unique_ptr<Sampler>            m_sampler;          /**< Sampler for taking samples. Currently not used. */
    std::unique_ptr<PixelSample[]>      m_pixelSamples;     /**< Samples to take. Currently not used. */
};

//! @brief  PreRender_Task provides a chance for integrators to preprocess some data before rendering.
//!
//! One example of such a case is to shoot virtual point light before evaluating rendering equation
//! in a second stage in an instance radiosity algorithm.
class PreRender_Task : public Task {
public:
    //! @brief Constructor
    //!
    //! @param priority     New priority of the task.
    PreRender_Task( const Scene& scene , const char* name , unsigned int priority , 
                    const std::unordered_set<std::shared_ptr<Task>>& dependencies ) : 
                    Task( name , priority , dependencies ), m_scene(scene){}
    
    //! @brief  Execute the task
    void        Execute() override;

private:
    const Scene&   m_scene;
};