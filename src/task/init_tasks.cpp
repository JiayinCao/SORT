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

#include <string.h>
#include "init_tasks.h"
#include "core/timer.h"
#include "managers/matmanager.h"
#include "core/globalconfig.h"
#include "core/scene.h"

SORT_STATS_DEFINE_COUNTER(sPreprocessTimeMS)
SORT_STATS_TIME("Performance", "Pre-processing Time", sPreprocessTimeMS);

void Loading_Task::Execute(){
    TIMING_EVENT( "Serializing scene" );

	// Load materials from stream
	MatManager::GetSingleton().ParseMatFile(m_stream);

	// Serialize the scene entities
	m_scene.LoadScene(m_stream);
}

void SpatialAccelerationConstruction_Task::Execute(){
    TIMING_EVENT( "Spatial acceleration structure construction" );

    Timer timer;
    g_accelerator->Build( m_scene );
    sPreprocessTimeMS = timer.GetElapsedTime();
}
