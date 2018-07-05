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

#include "sort.h"

#ifdef SORT_ENABLE_STATS_COLLECTION
#include <functional>
#include <map>
#include <vector>
#include <mutex>

#define SORT_STATS(eva) eva;

#define SORT_DEFINE_COUNTER( cat , name , var ) \
    static thread_local int var = 0; \
    static void update_counter_##var(StatsSummary& ss) \
    {\
        ss.FlushCounter( cat , name , var );\
    }\
    static thread_local StatsItem g_Counter_Int_##var( update_counter_##var );

#define SORT_DEFINE_COUNTER_TYPE( cat , name , var , T ) \
    static thread_local float var = 0; \
    static void update_counter_##var(StatsSummary& ss) \
    {\
        ss.FlushCounter( cat , name , var );\
    }\
    static thread_local StatsItem g_Counter_Float_##var( update_counter_##var );

// StatsSummary keeps all stats data after the rendering is done
class StatsSummary {
public:
    void FlushCounter(const std::string& category, const std::string& varname, int var) {
        std::lock_guard<std::mutex> lock(mutex);
        counters[category][varname] += var;
    }
    void FlushCounter(const std::string& category, const std::string& varname, float var) {
        std::lock_guard<std::mutex> lock(mutex);
        countersFloat[category][varname] += var;
    }
    void PrintStats() const;

private:
    std::map<string, std::map<string, int>> counters;
    std::map<string, std::map<string, float>> countersFloat;
    std::mutex mutex;
};

using stats_update = std::function<void(StatsSummary&)>;
class StatsItem {
public:
    // Register all stats item before main function in constructors
    StatsItem(const stats_update f);
    // Flush the data into StatsSummary
    void FlushData() const;

private:
    // This is used to flush the data to final StatsSummary at the end of the thread
    const stats_update func;
};

#else
#define SORT_STATS(eva)
#define SORT_DEFINE_COUNTER( cat , name , var )
#endif

// Flush per-thread stats to StatsSummary, this should be called at the end of per-thread
void FlushStatsData();
// Print Stats Result, this should be called in main thread after all rendering thread is done
void PrintStatsData();