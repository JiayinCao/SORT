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
#include <memory>
#include "utility/sassert.h"
#include "utility/strhelper.h"
#include "define.h"

#define SORT_STATS(eva) eva

#define SORT_STATS_BASE_TYPE( cat , name , var , formatter , type )\
static Thread_Local type<formatter> g_StatsItem_##var; \
static void update_counter_##var(StatsSummary& ss) { ss.FlushCounter( cat , name , &g_StatsItem_##var );}\
static thread_local StatsItemRegister g_Counter_Int_##var( update_counter_##var );\

#define SORT_STATS_INT_TYPE( cat , name , var , formatter) \
SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemInt );\
static Thread_Local long long& var = (g_StatsItem_##var).value;

#define SORT_STATS_FLOAT_TYPE( cat , name , var , formatter ) \
SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemFloat );\
static Thread_Local float& var = (g_StatsItem_##var).value;

#define SORT_STATS_COUNTER( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsInt)
#define SORT_STATS_TIME( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsElaspedTime)
#define SORT_STATS_FCOUNTER( cat , name , var ) SORT_STATS_FLOAT_TYPE( cat , name , var , StatsFloat)

#define STATS_FORMATTER( name , type ) class name{ public: static std::string ToString( type v ); };
STATS_FORMATTER( StatsElaspedTime , long long )
STATS_FORMATTER( StatsInt , long long )
STATS_FORMATTER( StatsFloat , float )

class StatsItem{
public:
    virtual std::string ToString() const = 0;
    virtual void Merge( const StatsItem* item ) = 0;
    virtual shared_ptr<StatsItem> MakeItem() const = 0;
};

template<class T>
class StatsItemInt : public StatsItem{
public:
    std::string ToString() const override{
        return T::ToString(value);
    }
    void Merge( const StatsItem* item ) override{
        auto p = dynamic_cast<const StatsItemInt*>(item);
        sAssert( p , "Merging incorrect stats data." );
        value += p->value;
    }
    shared_ptr<StatsItem> MakeItem() const override{
        return make_shared<StatsItemInt>();
    }
    long long value = 0;
};
template<class T>
class StatsItemFloat : public StatsItem{
public:
    std::string ToString() const override{
        return T::ToString(value);
    }
    void Merge( const StatsItem* item ) override{
        auto p = dynamic_cast<const StatsItemFloat*>(item);
        sAssert( p , "Merging incorrect stats data." );
        value += p->value;
    }
    shared_ptr<StatsItem> MakeItem() const override{
        return make_shared<StatsItemFloat>();
    }
    float value = 0.0f;
};

// StatsSummary keeps all stats data after the rendering is done
class StatsSummary {
public:
    void FlushCounter(const std::string& category, const std::string& varname, const StatsItem* var) {
        std::lock_guard<std::mutex> lock(mutex);
        if( counters[category].count(varname) == 0 )
            counters[category][varname] = var->MakeItem();
        counters[category][varname]->Merge(var);
    }
    void PrintStats() const;

private:
    std::map<string, std::map<string, std::shared_ptr<StatsItem>>> counters;
    std::mutex mutex;
};

using stats_update = std::function<void(StatsSummary&)>;
class StatsItemRegister {
public:
    // Register all stats item before main function in constructors
    StatsItemRegister(const stats_update f);
    // Flush the data into StatsSummary
    void FlushData() const;

private:
    // This is used to flush the data to final StatsSummary at the end of the thread
    const stats_update func;
};

#else
#define SORT_STATS(eva)
#define SORT_STATS_COUNTER( cat , name , var )
#define SORT_STATS_FCOUNTER( cat , name , var )
#define SORT_STATS_TIME( cat , name , var )
#endif

// Flush per-thread stats to StatsSummary, this should be called at the end of per-thread
void FlushStatsData();
// Print Stats Result, this should be called in main thread after all rendering thread is done
void PrintStatsData();
