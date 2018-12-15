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
#include "sampler/sample.h"
#include "math/vector2.h"

class Integrator;
class Scene;
class Sampler;
class Camera;
class RenderTarget;
class ImageSensor;

//! @brief  Render_Task is a baic rendering unit doing ray tracing.
//!
//! Each renderin task is usually responsible for a tile of image to be rendered in
//! most cases. In other cases, like light tracing, there is no difference between
//! different render_task.
class Render_Task : public Task{
public:
    //! Constructor
    //!
    //! @param priority     New priority of the task.
    Render_Task( unsigned int priority ) : Task( priority ) {}
    
    //! @brief  Execute the task
    void        Execute() override;

public:
    // the following parameters define where to calculate the image
    Vector2i ori;
	Vector2i size;
    
    // the task id
    unsigned		taskId = 0;
    bool*			taskDone = nullptr;	// used to show the progress
    
    // the pixel sample
    PixelSample*	pixelSamples = nullptr;
    unsigned		samplePerPixel = 0;
    
    // the sampler
    Sampler*		sampler = nullptr;
    // the camera
    Camera*			camera = nullptr;
    // the scene description
    const Scene*	scene;

    // integrator
    std::shared_ptr<Integrator> integrator = nullptr;
};