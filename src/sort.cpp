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

#include <thread>
#include "sort.h"
#include "work/image_evaluation/image_evaluation.h"
#include "work/unit_tests/unit_tests.h"
#include "core/parse_args.h"

int RunSORT(int argc, char** argv) {
    // Parse command line arguments.
    const auto& args = parse_args(argc, argv);

    bool profiling_enabled = false;
    bool unit_test_mode = false;
    bool valid_args = false;

    for (auto& arg : args) {
        const auto& key_str = arg.first;
        const auto& value_str = arg.second;

        if (key_str == "input") {
            valid_args = true;
        }
        else if (key_str == "unittest") {
            unit_test_mode = true;
            valid_args = true;
        }
        else if (key_str == "profiling") {
            profiling_enabled = value_str == "on";
        }
    }

    // Disable profiling if necessary
    if (!profiling_enabled)
        SORT_PROFILE_DISABLE;

    if (!valid_args) {
        slog(INFO, GENERAL, "There is not enough command line arguments.");
        slog(INFO, GENERAL, "  --input:<filename>   Specify the sort input file.");
        slog(INFO, GENERAL, "  --blendermode        SORT is triggered from Blender.");
        slog(INFO, GENERAL, "  --unittest           Run unit tests.");
        slog(INFO, GENERAL, "  --nomaterial         Disable materials in SORT.");
        slog(INFO, GENERAL, "  --profiling:<on|off> Toggling profiling option, false by default.");
        return -1;
    }
    else {
        slog(INFO, GENERAL, "Number of CPU cores %d", std::thread::hardware_concurrency());
#ifdef SORT_ENABLE_STATS_COLLECTION
        slog(INFO, GENERAL, "Stats collection is enabled.");
#else
        slog(INFO, GENERAL, "Stats collection is disabled.");
#endif
        slog(INFO, GENERAL, "Profiling system is %s.", SORT_PROFILE_ISENABLED ? "enabled" : "disabled");
    }

    // Run in unit test mode if required.
    std::unique_ptr<Work> work;
    if (unit_test_mode)
        work = std::make_unique<UnitTests>();
    else
        work = std::make_unique<ImageEvaluation>();
    work->StartRunning(argc, argv);
    auto ret = work->WaitForWorkToBeDone();

    // Flush main thread data
    SortStatsFlushData(true);
    // Output stats data
    if (ret == 0 && !unit_test_mode)
        SortStatsPrintData();

    // this sucks
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return ret;
}
