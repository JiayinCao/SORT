/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "sort.h"
#include "core/globalconfig.h"
#include "thirdparty/gtest/gtest.h"
#include "task/init_tasks.h"
#include "core/scene.h"
#include "sampler/random.h"
#include "core/timer.h"
#include "stream/fstream.h"

SORT_STATS_DEFINE_COUNTER(sRenderingTimeMS)
SORT_STATS_DEFINE_COUNTER(sSamplePerPixel)
SORT_STATS_DEFINE_COUNTER(sThreadCnt)

SORT_STATS_TIME("Performance", "Rendering Time", sRenderingTimeMS);
SORT_STATS_AVG_RAY_SECOND("Performance", "Number of rays per second", sRayCount , sRenderingTimeMS);
SORT_STATS_COUNTER("Statistics", "Sample per Pixel", sSamplePerPixel);
SORT_STATS_COUNTER("Performance", "Worker thread number", sThreadCnt);

void SchedulTasks( Scene& scene , IStreamBase& stream ){
	SORT_PROFILE("Schedule Tasks");

	auto loading_task = SCHEDULE_TASK<Loading_Task>( "Loading" , DEFAULT_TASK_PRIORITY, {} , scene, stream);
	auto sac_task = SCHEDULE_TASK<SpatialAccelerationConstruction_Task>( "Spatial Data Structor Construction" , DEFAULT_TASK_PRIORITY, {loading_task} , scene);
	auto pre_render_task = SCHEDULE_TASK<PreRender_Task>( "Pre rendering pass" , DEFAULT_TASK_PRIORITY, {sac_task} , scene);

	// Push render task into the queue
	const auto tilesize = (int)g_tileSize;
	const auto width = (int)g_resultResollution[0];
	const auto height = (int)g_resultResollution[1];
	
	// get the number of total task
	Vector2i tile_num = Vector2i( (int)ceil(width / (float)tilesize) , (int)ceil(height / (float)tilesize) );

	// start tile from center instead of top-left corner
	Vector2i cur_pos( tile_num / 2 );
	int cur_dir = 0;
	int cur_len = 0;
	int cur_dir_len = 1;
	const Vector2i dir[4] = { Vector2i( 0 , -1 ) , Vector2i( -1 , 0 ) , Vector2i( 0 , 1 ) , Vector2i( 1 , 0 ) };

	unsigned int priority = DEFAULT_TASK_PRIORITY;
	while (true){
		// only process node inside the image region
		if (cur_pos.x >= 0 && cur_pos.x < tile_num.x && cur_pos.y >= 0 && cur_pos.y < tile_num.y ){
			Vector2i tl( cur_pos.x * tilesize , cur_pos.y * tilesize );
			Vector2i size( (tilesize < (width - tl.x)) ? tilesize : (width - tl.x) , 
						   (tilesize < (height - tl.y)) ? tilesize : (height - tl.y) );

			SCHEDULE_TASK<Render_Task>( "render task" , priority-- , {pre_render_task} , tl , size , scene );
		}

		// turn to the next direction
		if (cur_len >= cur_dir_len){
			cur_dir = (cur_dir + 1) % 4;
			cur_len = 0;
			cur_dir_len += 1 - cur_dir % 2;
		}

		cur_pos += dir[cur_dir];
		++cur_len;
		if( (cur_pos.x < 0 || cur_pos.x >= tile_num.x ) && (cur_pos.y < 0 || cur_pos.y >= tile_num.y ) )
			break;
	}
}

bool	RunSORT( int argc , char** argv ){
	// Parse command line arguments.
    bool valid_args = GlobalConfiguration::GetSingleton().ParseCommandLine( argc , argv );
    if (!valid_args) {
        slog(INFO, GENERAL, "There is not enough command line arguments.");
        slog(INFO, GENERAL, "  --input:<filename>   Specify the sort input file.");
        slog(INFO, GENERAL, "  --blendermode        SORT is triggered from Blender.");
        slog(INFO, GENERAL, "  --unitest            Run unit tests.");
        return false;
    }
	
	// Run in unit test mode if required.
    if( g_unitTestMode ){
        ::testing::InitGoogleTest(&argc, argv);
        auto ret = RUN_ALL_TESTS();
		slog( INFO , GENERAL , ( ret ? "There are broken tests." : "All tests are passed." ) ) ;
		return false;
    }

	// Load the global configuration from stream
	IFileStream stream( g_inputFilePath );
	GlobalConfiguration::GetSingleton().Serialize(stream);

	Scene scene;

	// Schedule all tasks.
    SchedulTasks( scene , stream );

    std::vector< std::unique_ptr<WorkerThread> > threads;
    for( unsigned i = 0 ; i < g_threadCnt ; ++i )
        threads.push_back( std::make_unique<WorkerThread>( i + 1 ) );

    // start all threads
    for_each( threads.begin() , threads.end() , []( std::unique_ptr<WorkerThread>& thread ) { thread->BeginThread(); } );

	{
		TIMING_EVENT_STAT( "" , sRenderingTimeMS );
		EXECUTING_TASKS();

		// wait for all the threads to be finished
    	for_each( threads.begin() , threads.end() , []( std::unique_ptr<WorkerThread>& thread ) { thread->Join(); } );
	}
    
	SORT_STATS(sSamplePerPixel = g_samplePerPixel);
	SORT_STATS(sThreadCnt = g_threadCnt);

    // Post process for image sensor
    g_imageSensor->PostProcess();

    return true;
}
