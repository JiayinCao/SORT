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
#include "system.h"
#include "core/log.h"
#include "core/stats.h"
#include "core/profile.h"
#include "core/path.h"
#include "core/globalconfig.h"
#include "thirdparty/gtest/gtest.h"

System g_System;

#include <regex>
using namespace std;

#ifdef SORT_IN_WINDOWS
int __cdecl main( int argc , char** argv )
#elif defined(SORT_IN_LINUX) || defined(SORT_IN_MAC)
int main(int argc, char** argv)
#endif
{
    // enable profiler
    SORT_PROFILE_ENABLE;
    SORT_PROFILE("Main Thread");
    
    addLogDispatcher(new StdOutLogDispatcher());
    addLogDispatcher(new FileLogDispatcher( "log.txt" ));

    // Parse command line arguments.
    GlobalConfiguration::GetSingleton().ParseCommandLine( argc , argv );

    slog(INFO, GENERAL, "Number of CPU cores %d" , NumSystemCores() );
#ifdef SORT_ENABLE_STATS_COLLECTION
    slog( INFO, GENERAL, "Stats collection is enabled." );
#else
    slog( INFO, GENERAL, "Stats collection is disabled." );
#endif
    slog( INFO, GENERAL, "Profiling system is %s." , SORT_PROFILE_ISENABLED ? "enabled" : "disabled" );

    // Run in unit test mode.
    if( g_unitTestMode ){
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    // setup the system
    if (g_System.Setup(argv[1])){
        // do ray tracing
        g_System.Render();
    }
    
    // Flush main thread data
    SortStatsFlushData(true);
    // Output stats data
    SortStatsPrintData();

    // unitialize the system
    g_System.Uninit();

    SORT_PROFILE_END; // Main Thread

    // dump profile data
    if (SORT_PROFILE_ISENABLED){
        const std::string filename("sort.prof");
        SORT_PROFILE_DUMP(filename.c_str());
        slog(INFO, GENERAL, "Profiling file: \"%s\"", GetFilePathInExeFolder(filename).c_str());
    }
    slog(INFO, GENERAL, "Log file: \"%s\"", GetFilePathInExeFolder("log.txt").c_str());

	return 0;
}
