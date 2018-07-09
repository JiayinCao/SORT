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
void SortStatsFlushData( bool mainThread = false );
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

#define SORT_CAT_PROXY(v0, v1)              v0 ## v1
#define SORT_CAT(v0, v1)                    SORT_CAT_PROXY(v0,v1)
#define SORT_STATS_UNIQUE_NAMESPACE()       SORT_CAT(SORT_CAT(sort_stats_namespace, __LINE__), _namespace)

struct StatsData_Ratio{
    long long& nominator;
    long long& denominator;
    StatsData_Ratio& operator += ( const StatsData_Ratio& r ){
        nominator += r.nominator;
        denominator += r.denominator;
        return *this;
    }
    StatsData_Ratio( long long& v0 , long long& v1 ) : nominator( v0 ) , denominator( v1 ) {}
};

class StatsItemBase{
public:
    virtual std::string ToString() const = 0;
    virtual void Merge( const StatsItemBase* item ) = 0;
    virtual shared_ptr<StatsItemBase> MakeItem() const = 0;
};

#define SORT_STATS(eva) eva

#define SORT_STATS_DEFINE_COUNTER( var ) Thread_Local long long var = 0l;
#define SORT_STATS_DEFINE_FCOUNTER( var ) Thread_Local float var = 0.0f;

#define SORT_STATS_DECLARE_COUNTER( var ) extern Thread_Local long long var;
#define SORT_STATS_DECLARE_FCOUNTER( var ) extern Thread_Local float var;

#define SORT_STATS_ENABLE(category) \
    class StatsCategoryEnabler{ \
        public: \
            StatsCategoryEnabler() { SortStatsEnableCategory( category ); } \
    };\
    StatsCategoryEnabler stats_category_enabler;

#define SORT_STATS_ITEM( NAME , DATA , var ) \
template<class T>\
class NAME##var : public StatsItemBase{\
public:\
    NAME##var( DATA& d ) : data(d) {}\
    std::string ToString() const override{\
        return T::ToString(data);\
    }\
    void Merge( const StatsItemBase* item ) override{\
        auto p = dynamic_cast<const NAME##var*>(item);\
        sAssert( p != nullptr , "Merging incorrect stats data." );\
        data += p->data;\
    }\
    shared_ptr<StatsItemBase> MakeItem() const override{\
        return make_shared<NAME##var>(g_Global_Default);\
    }\
    DATA& data;\
};

#define SORT_STATS_BASE_TYPE( cat , name , var , formatter , type , data_type )\
    SORT_STATS_ITEM( type , data_type , var )\
    static Thread_Local type##var<formatter> g_StatsItem(var); \
    static void update_counter_##var(StatsSummary& ss) { ss.FlushCounter( cat , name , &g_StatsItem );}\
    static StatsItemRegister g_StatsItemRegister( update_counter_##var );

#define SORT_STATS_INT_TYPE( cat , name , var , formatter) \
    extern Thread_Local long long var;\
    namespace SORT_STATS_UNIQUE_NAMESPACE(){\
        static long long g_Global_Default = 0l;\
        SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemInt , long long );\
    }

#define SORT_STATS_FLOAT_TYPE( cat , name , var , formatter ) \
    extern Thread_Local float var;\
    namespace SORT_STATS_UNIQUE_NAMESPACE(){\
        static float g_Global_Default = 0.0f;\
        SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemFloat , float );\
    }

#define SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , formatter ) \
    extern Thread_Local long long var0;\
    extern Thread_Local long long var1;\
    namespace SORT_STATS_UNIQUE_NAMESPACE(){\
        static Thread_Local StatsData_Ratio g##var0##_##var1( var0 , var1 );\
        static long long g_Global_Var0 = 0l;\
        static long long g_Global_Var1 = 0l;\
        static StatsData_Ratio g_Global_Default( g_Global_Var0 , g_Global_Var1 );\
        SORT_STATS_BASE_TYPE( cat , name , g##var0##_##var1 , formatter , StatsItemRatio , StatsData_Ratio);\
    }

#define SORT_STATS_COUNTER( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsInt )
#define SORT_STATS_TIME( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsElaspedTime )
#define SORT_STATS_FCOUNTER( cat , name , var ) SORT_STATS_FLOAT_TYPE( cat , name , var , StatsFloat )
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

// StatsSummary keeps all stats data after the rendering is done
class StatsSummary {
public:
    void FlushCounter(const std::string& category, const std::string& varname, const StatsItemBase* var);
    void PrintStats() const;
    void EnableCategory(const std::string& s);

private:
    std::map<string, std::map<string, std::shared_ptr<StatsItemBase>>> counters;
    std::unordered_set<std::string> categories = { "Performance" , "Scene" };
};

using stats_update = std::function<void(StatsSummary&)>;
class StatsItemRegister {
public:
    // Register all stats item before main function in constructors
    StatsItemRegister(const stats_update f );
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
#define SORT_STATS_DEFINE_COUNTER( var )
#define SORT_STATS_DEFINE_FCOUNTER( var )
#define SORT_STATS_DECLARE_COUNTER( var )
#define SORT_STATS_DECLARE_FCOUNTER( var )
#endif
