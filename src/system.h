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

#include "core/scene.h"

/////////////////////////////////////////////////////////////////////
//	definition of the system
class	System
{
public:
	// pre-process before rendering
	void PreProcess();
	// render the image
	void Render();

	// setup system from file
	bool Setup( const char* str );
	
	// get scene
	const Scene& GetScene() const { return m_Scene; }

	// uninitialize
	void Uninit();
	
    // ugly workaround, to be changed later.
    ImageSensor*    GetImageSensor() { return m_imagesensor; }

private:
    // image sensor
    ImageSensor*    m_imagesensor = nullptr;

    unsigned		m_totalTask = 0;
    bool*			m_taskDone = nullptr;
    char*			m_pProgress = nullptr;

	// the scene for rendering
	Scene			m_Scene;

	// output progress
	void	_outputProgress();
	// do ray tracing in a multithread enviroment
	void	_executeRenderingTasks();
	// push rendering task
	void	_pushRenderTask();
};
