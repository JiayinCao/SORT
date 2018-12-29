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

#include "sort.h"
#include "core/globalconfig.h"
#include "thirdparty/gtest/gtest.h"
#include "task/init_tasks.h"
#include "imagesensor/imagesensor.h"
#include "core/scene.h"
#include "sampler/random.h"
#include "core/timer.h"

SORT_STATS_DEFINE_COUNTER(sRenderingTimeMS)
SORT_STATS_DEFINE_COUNTER(sSamplePerPixel)
SORT_STATS_DEFINE_COUNTER(sThreadCnt)

SORT_STATS_TIME("Performance", "Rendering Time", sRenderingTimeMS);
SORT_STATS_AVG_RAY_SECOND("Performance", "Number of rays per second", sRayCount , sRenderingTimeMS);
SORT_STATS_COUNTER("Statistics", "Sample per Pixel", sSamplePerPixel);
SORT_STATS_COUNTER("Performance", "Worker thread number", sThreadCnt);

ImageSensor* m_imagesensor = nullptr;

void	RunSORT( int argc , char** argv ){
	// Parse command line arguments.
    GlobalConfiguration::GetSingleton().ParseCommandLine( argc , argv );
	
	// Run in unit test mode if required.
    if( g_unitTestMode ){
        ::testing::InitGoogleTest(&argc, argv);
        auto ret = RUN_ALL_TESTS();
		slog( INFO , GENERAL , ( ret ? "All tests are passed." : "There are broken tests." ) ) ;
		return;
    }

	Scene scene;

    auto loading_task = SCHEDULE_TASK<Loading_Task>( "Loading" , DEFAULT_TASK_PRIORITY, {} , &scene);
    SCHEDULE_TASK<SpatialAccelerationConstruction_Task>( "Spatial Data Structor Construction." , DEFAULT_TASK_PRIORITY, {loading_task} , &scene);
    EXECUTING_TASKS();

    g_integrator->PreProcess();
	g_integrator->SetupScene(&scene);

	Timer timer;

	// Push render task into the queue
	const auto tilesize = g_tileSize;
	const auto width = g_resultResollution[0];
	const auto height = g_resultResollution[1];
	
	// get the number of total task
	Vector2i tile_num = Vector2i( (int)ceil(width / (float)tilesize) , (int)ceil(height / (float)tilesize) );

	// start tile from center instead of top-left corner
	Vector2i cur_pos( tile_num / 2 );
	int cur_dir = 0;
	int cur_len = 0;
	int cur_dir_len = 1;
	const Vector2i dir[4] = { Vector2i( 0 , -1 ) , Vector2i( -1 , 0 ) , Vector2i( 0 , 1 ) , Vector2i( 1 , 0 ) };

	unsigned int priority = DEFAULT_TASK_PRIORITY;
	while (true)
	{
		// only process node inside the image region
		if (cur_pos.x >= 0 && cur_pos.x < tile_num.x && cur_pos.y >= 0 && cur_pos.y < tile_num.y )
		{
			Vector2i size , tl ;
			tl.x = cur_pos.x * tilesize;
			tl.y = cur_pos.y * tilesize;
			size.x = (tilesize < (width - tl.x)) ? tilesize : (width - tl.x);
			size.y = (tilesize < (height - tl.y)) ? tilesize : (height - tl.y);

			SCHEDULE_TASK<Render_Task>( "render task" , priority-- , {} , tl , size , &scene , new RandomSampler() , new PixelSample[g_samplePerPixel] );
		}

		// turn to the next direction
		if (cur_len >= cur_dir_len)
		{
			cur_dir = (cur_dir + 1) % 4;
			cur_len = 0;
			cur_dir_len += 1 - cur_dir % 2;
		}

		cur_pos += dir[cur_dir];

		++cur_len;

		if( (cur_pos.x < 0 || cur_pos.x >= tile_num.x ) && (cur_pos.y < 0 || cur_pos.y >= tile_num.y ) )
			break;
	}

    m_imagesensor->PreProcess();

	// pre allocate memory for the specific thread
	for( unsigned i = 0 ; i <= g_threadCnt ; ++i )
		MemManager::GetSingleton().PreMalloc( 1024 * 1024 * 1024 , i );
    
    std::vector< std::unique_ptr<WorkerThread> > threads;
    for( unsigned i = 0 ; i < g_threadCnt ; ++i )
        threads.push_back( std::unique_ptr<WorkerThread>( new WorkerThread( i + 1 ) ) );

    // start all threads
    for_each( threads.begin() , threads.end() , []( std::unique_ptr<WorkerThread>& thread ) { thread->BeginThread(); } );

	EXECUTING_TASKS();

	// wait for all the threads to be finished
    for_each( threads.begin() , threads.end() , []( std::unique_ptr<WorkerThread>& thread ) { thread->Join(); } );
    
	SORT_STATS(sRenderingTimeMS = timer.GetElapsedTime());
	SORT_STATS(sSamplePerPixel = g_samplePerPixel);
	SORT_STATS(sThreadCnt = g_threadCnt);

    // Post process for image sensor
    m_imagesensor->PostProcess();
}
