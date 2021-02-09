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

#include "sort.h"
#include "core/globalconfig.h"
#include "thirdparty/gtest/gtest.h"
#include "core/scene.h"
#include "sampler/random.h"
#include "core/timer.h"
#include "stream/fstream.h"
#include "material/tsl_system.h"
#include "work/unit_tests/unit_tests.h"
#include "work/image_evaluation/image_evaluation.h"

int RunSORT( int argc , char** argv ){
    // Parse command line arguments.
    bool valid_args = GlobalConfiguration::GetSingleton().ParseCommandLine( argc , argv );

    // Disable profiling if necessary
    if( !g_profilingEnabled )
        SORT_PROFILE_DISABLE;

    if (!valid_args) {
        slog(INFO, GENERAL, "There is not enough command line arguments.");
        slog(INFO, GENERAL, "  --input:<filename>   Specify the sort input file.");
        slog(INFO, GENERAL, "  --blendermode        SORT is triggered from Blender.");
        slog(INFO, GENERAL, "  --unittest           Run unit tests.");
        slog(INFO, GENERAL, "  --nomaterial         Disable materials in SORT.");
        slog(INFO, GENERAL, "  --profiling:<on|off> Toggling profiling option, false by default.");
        return -1;
    }else{
        slog(INFO, GENERAL, "Number of CPU cores %d", std::thread::hardware_concurrency());
        #ifdef SORT_ENABLE_STATS_COLLECTION
            slog(INFO, GENERAL, "Stats collection is enabled.");
        #else
            slog(INFO, GENERAL, "Stats collection is disabled.");
        #endif
        slog(INFO, GENERAL, "Profiling system is %s.", SORT_PROFILE_ISENABLED ? "enabled" : "disabled");
    }

    std::unique_ptr<Work> work;

    // Run in unit test mode if required.
    if( g_unitTestMode ){
        work = std::make_unique<UnitTests>();
        work->StartRunning(argc, argv);
        return work->WaitForWorkToBeDone();
    }

    // we only support one other work for now
    work = std::make_unique<ImageEvaluation>();
    work->StartRunning(argc, argv);
    return work->WaitForWorkToBeDone();
}
