/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "core/define.h"
#include "sort.h"
#include "core/log.h"
#include "core/stats.h"
#include "core/profile.h"
#include "core/path.h"

#ifdef SORT_IN_WINDOWS
int __cdecl main( int argc , char** argv )
#elif defined(SORT_IN_LINUX) || defined(SORT_IN_MAC)
int main(int argc, char* argv[])
#endif
{
    /*
    for( int i = 0 ; i < 256 ; ++i ){
        if( i % 10 == 0 )
            std::cout<<std::endl;

        const int N = 10000;
        float n = i / 255.0f * 30.0f;
        float sum = 0;
        for( int k = 0 ; k < N ; ++k ){
            auto r = sort_canonical() * HALF_PI;
            sum += pow( 1.0f - sin( r * 0.5f ) , n ) * sin( r );
        }
        sum *= TWO_PI / N;
        std::cout<<sum<<",";
    }
    std::cout<<std::endl;
    return 0;
    */
   
    // enable profiler
    SORT_PROFILE_ENABLE;
    SORT_PROFILE("Main Thread");

    addLogDispatcher(std::make_unique<StdOutLogDispatcher>());
    addLogDispatcher(std::make_unique<FileLogDispatcher>("log.txt"));

    const auto ret = RunSORT(argc, argv);

    // Flush main thread data
    SortStatsFlushData(true);
    // Output stats data
    if(ret == 0 && !g_unitTestMode)
        SortStatsPrintData();

    SORT_PROFILE_END; // Main Thread

    // dump profile data
    if (SORT_PROFILE_ISENABLED && ret == 0 ){
        const std::string filename("sort.prof");
        SORT_PROFILE_DUMP(filename.c_str());
        slog(INFO, GENERAL, "Profiling file: \"%s\"", GetFilePathInExeFolder(filename).c_str());
    }
    slog(INFO, GENERAL, "Log file: \"%s\"", GetFilePathInExeFolder("log.txt").c_str());

    return ret;
}
