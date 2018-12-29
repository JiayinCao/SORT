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

#include "init_tasks.h"
#include "core/timer.h"
#include "stream/fstream.h"
#include "managers/matmanager.h"
#include "core/globalconfig.h"
#include "core/scene.h"

// to be removed.
#include "imagesensor/imagesensor.h"
#include "imagesensor/blenderimage.h"
#include "imagesensor/rendertargetimage.h"
extern ImageSensor* m_imagesensor ;
extern Scene g_scene;

SORT_STATS_DEFINE_COUNTER(sPreprocessTimeMS)
SORT_STATS_TIME("Performance", "Pre-processing Time", sPreprocessTimeMS);

void Loading_Task::Execute(){
    TIMING_EVENT( "Serializing scene" );

	IFileStream stream( g_inputFilePath );

	// Load the global configuration from stream
	GlobalConfiguration::GetSingleton().Serialize(stream);

	// Load materials from stream
	MatManager::GetSingleton().ParseMatFile(stream);

    // To be moved to somewhere else, this is very ugly.
	if( g_blenderMode )
		m_imagesensor = new BlenderImage();
	else
		m_imagesensor = new RenderTargetImage();
	m_imagesensor->SetSensorSize( g_resultResollution.x , g_resultResollution.y );

	// Serialize the scene entities
	g_scene.LoadScene(stream);

    // create shared memory
	int x_tile = (int)(ceil(m_imagesensor->GetWidth() / (float)g_tileSize));
	int y_tile = (int)(ceil(m_imagesensor->GetHeight() / (float)g_tileSize));
	int header_size = x_tile * y_tile;
	int size = header_size * g_tileSize * g_tileSize * 4 * sizeof(float) * 2	// image size
			+ header_size								// header size
			+ 2;										// progress data and final update flag

	// create shared memory
	const SharedMemory& sm = SMManager::GetSingleton().CreateSharedMemory("sharedmem.bin", size, SharedMmeory_All);
	// clear the memory first
	if (sm.bytes)
		memset(sm.bytes, 0, sm.size);
}

void SpatialAccelerationConstruction_Task::Execute(){
    TIMING_EVENT( "Spatial acceleration structure construction" );

    Timer timer;
    g_accelerator->Build( g_scene );
    sPreprocessTimeMS = timer.GetElapsedTime();
}
