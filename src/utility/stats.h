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

// Flush per-thread stats to StatsSummary, this should be called at the end of per-thread
void SortStatsFlushData();
// Print Stats Result, this should be called in main thread after all rendering thread is done
void SortStatsPrintData();
// Enable specific category
void SortStatsEnableCategory( const std::string& s );

#ifdef SORT_ENABLE_STATS_COLLECTION
#include <functional>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <unordered_set>
#include "utility/sassert.h"
#include "utility/strhelper.h"
#include "define.h"

struct StatsData_Ratio{
    long long nominator = 0;
    long long denominator = 0;
    StatsData_Ratio& operator += ( const StatsData_Ratio& r ){
        nominator += r.nominator;
        denominator += r.denominator;
        return *this;
    }
};

class StatsItemBase{
public:
    virtual std::string ToString() const = 0;
    virtual void Merge( const StatsItemBase* item ) = 0;
    virtual shared_ptr<StatsItemBase> MakeItem() const = 0;
};

#define SORT_STATS(eva) eva

#define SORT_STATS_ENABLE(category) \
    class StatsCategoryEnabler{ \
        public: \
            StatsCategoryEnabler() { SortStatsEnableCategory( category ); } \
    };\
    StatsCategoryEnabler stats_category_enabler;

#define SORT_STATS_BASE_TYPE( cat , name , var , formatter , type )\
static Thread_Local type<formatter> g_StatsItem_##var; \
static void update_counter_##var(StatsSummary& ss) { ss.FlushCounter( cat , name , &g_StatsItem_##var );}\
static thread_local StatsItemRegister g_Counter_Int_##var( update_counter_##var );\

#define SORT_STATS_INT_TYPE( cat , name , var , formatter) \
SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemInt );\
Thread_Local long long& var = (g_StatsItem_##var).data;

#define SORT_STATS_FLOAT_TYPE( cat , name , var , formatter ) \
SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemFloat );\
Thread_Local float& var = (g_StatsItem_##var).data;

#define SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , formatter ) \
SORT_STATS_BASE_TYPE( cat , name , var0 , formatter , StatsItemRatio );\
Thread_Local long long& var0 = (g_StatsItem_##var0).data.nominator;\
Thread_Local long long& var1 = (g_StatsItem_##var0).data.denominator;

#define SORT_STATS_COUNTER( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsInt)
#define SORT_STATS_TIME( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsElaspedTime)
#define SORT_STATS_FCOUNTER( cat , name , var ) SORT_STATS_FLOAT_TYPE( cat , name , var , StatsFloat)
#define SORT_STATS_RATIO( cat , name , var0 , var1 ) SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , StatsRatio )
#define SORT_STATS_AVG_COUNT( cat , name , var0 , var1 ) SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , StatsFloatRatio )
#define SORT_STATS_AVG_RAY_SECOND( cat , name , var0 , var1 ) SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , StatsRayPerSecond )

#define SORT_STATS_FORMATTER( name , type ) class name{ public: static std::string ToString( type v ); };
SORT_STATS_FORMATTER( StatsElaspedTime , long long )
SORT_STATS_FORMATTER( StatsInt , long long )
SORT_STATS_FORMATTER( StatsFloat , float )
SORT_STATS_FORMATTER( StatsFloatRatio , StatsData_Ratio  )
SORT_STATS_FORMATTER( StatsRatio , StatsData_Ratio )
SORT_STATS_FORMATTER( StatsRayPerSecond , StatsData_Ratio  )

#define SORT_STATS_ITEM( NAME , DATA ) \
template<class T>\
class NAME : public StatsItemBase{\
public:\
    std::string ToString() const override{\
        return T::ToString(data);\
    }\
    void Merge( const StatsItemBase* item ) override{\
        auto p = dynamic_cast<const NAME*>(item);\
        sAssert( p , "Merging incorrect stats data." );\
        data += p->data;\
    }\
    shared_ptr<StatsItemBase> MakeItem() const override{\
        return make_shared<NAME>();\
    }\
    DATA data;\
};

SORT_STATS_ITEM( StatsItemRatio , StatsData_Ratio )
SORT_STATS_ITEM( StatsItemAvg , StatsData_Ratio )
SORT_STATS_ITEM( StatsItemFloat , float )
SORT_STATS_ITEM( StatsItemInt , long long )

#define SORT_STATS_DECLERE_COUNTER( var ) extern Thread_Local long long& var;
#define SORT_STATS_DECLERE_FCOUNTER( var ) extern Thread_Local float& var;

// StatsSummary keeps all stats data after the rendering is done
class StatsSummary {
public:
    void FlushCounter(const std::string& category, const std::string& varname, const StatsItemBase* var) {
        std::lock_guard<std::mutex> lock(mutex);
        if( counters[category].count(varname) == 0 )
            counters[category][varname] = var->MakeItem();
        counters[category][varname]->Merge(var);
    }
    void PrintStats() const;
    void EnableCategory( const std::string& s ){
        categories.insert(s);
    }

private:
    std::map<string, std::map<string, std::shared_ptr<StatsItemBase>>> counters;
    std::unordered_set<std::string> categories = { "Performance" , "Scene" };
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
#define SORT_STATS_ENABLE(eva)
#define SORT_STATS_COUNTER( cat , name , var )
#define SORT_STATS_FCOUNTER( cat , name , var )
#define SORT_STATS_TIME( cat , name , var )
#define SORT_STATS_RATIO( cat , name , var0 , var1 )
#define SORT_STATS_AVG_COUNT( cat , name , var0 , var1 )
#define SORT_STATS_AVG_RAY_SECOND( cat , name , var0 , var1 )
#define SORT_STATS_DECLERE_COUNTER( var )
#define SORT_STATS_DECLERE_FCOUNTER( var )
#endif
