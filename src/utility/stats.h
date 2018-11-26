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
#include <unordered_map>
#include "utility/sassert.h"
#include "utility/strhelper.h"
#include "define.h"

#define StatsInt                            long long
#define StatsFloat                          float

#define SORT_CAT_PROXY(v0, v1)              v0 ## v1
#define SORT_CAT(v0, v1)                    SORT_CAT_PROXY(v0,v1)
#define SORT_STATS_UNIQUE_NAMESPACE(var)    SORT_CAT(SORT_CAT(sort_stats_namespace, __LINE__), var)

struct StatsData_Ratio{
    StatsInt& nominator;
    StatsInt& denominator;
    StatsData_Ratio& operator += ( const StatsData_Ratio& r ){
        nominator += r.nominator;
        denominator += r.denominator;
        return *this;
    }
    StatsData_Ratio(StatsInt& v0 , StatsInt& v1 ) : nominator( v0 ) , denominator( v1 ) {}
};

class StatsItemBase{
public:
    virtual std::string ToString() const = 0;
    virtual void Merge( const StatsItemBase* item ) = 0;
    virtual std::shared_ptr<StatsItemBase> MakeItem() const = 0;
};

#define SORT_STATS(eva) eva

#define SORT_STATS_DEFINE_COUNTER( var ) Thread_Local StatsInt var = 0l;
#define SORT_STATS_DEFINE_FCOUNTER( var ) Thread_Local StatsFloat var = 0.0f;

#define SORT_STATS_DECLARE_COUNTER( var ) extern Thread_Local StatsInt var;
#define SORT_STATS_DECLARE_FCOUNTER( var ) extern Thread_Local StatsFloat var;

#define SORT_STATS_ENABLE(category) \
    class StatsCategoryEnabler{ \
        public: \
            StatsCategoryEnabler() { SortStatsEnableCategory( category ); } \
    };\
    StatsCategoryEnabler stats_category_enabler;

#define SORT_STATS_ITEM( NAME , DATA ) \
template<class T>\
class NAME : public StatsItemBase{\
public:\
    NAME( DATA& d ) : data(d) {}\
    std::string ToString() const override{\
        return T::ToString(data);\
    }\
    void Merge( const StatsItemBase* item ) override{\
        auto p = dynamic_cast<const NAME*>(item);\
        sAssertMsg( p != nullptr , GENERAL , "Merging incorrect stats data." );\
        data += p->data;\
    }\
    std::shared_ptr<StatsItemBase> MakeItem() const override{\
        return std::make_shared<NAME>(g_Global_Default);\
    }\
    DATA& data;\
};

#define SORT_STATS_BASE_TYPE( cat , name , var , formatter , type , data_type )\
    SORT_STATS_ITEM( type , data_type )\
    static Thread_Local type<formatter> g_StatsItem(var); \
    static void update_counter(StatsSummary& ss) { ss.FlushCounter( cat , name , &g_StatsItem );}\
    static StatsItemRegister g_StatsItemRegister( update_counter , cat , name );

#define SORT_STATS_INT_TYPE( cat , name , var , formatter) \
    extern Thread_Local StatsInt var;\
    namespace SORT_STATS_UNIQUE_NAMESPACE(var){\
        static StatsInt g_Global_Default = 0l;\
        SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemInt , StatsInt );\
    }

#define SORT_STATS_FLOAT_TYPE( cat , name , var , formatter ) \
    extern Thread_Local StatsFloat var;\
    namespace SORT_STATS_UNIQUE_NAMESPACE(var){\
        static StatsFloat g_Global_Default = 0.0f;\
        SORT_STATS_BASE_TYPE( cat , name , var , formatter , StatsItemFloat , StatsFloat );\
    }

#define SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , formatter ) \
    extern Thread_Local StatsInt var0;\
    extern Thread_Local StatsInt var1;\
    namespace SORT_STATS_UNIQUE_NAMESPACE(g##var0##_##var1){\
        static Thread_Local StatsData_Ratio g##var0##_##var1( var0 , var1 );\
        static StatsInt g_Global_Var0 = 0l;\
        static StatsInt g_Global_Var1 = 0l;\
        static StatsData_Ratio g_Global_Default( g_Global_Var0 , g_Global_Var1 );\
        SORT_STATS_BASE_TYPE( cat , name , g##var0##_##var1 , formatter , StatsItemRatio , StatsData_Ratio);\
    }

#define SORT_STATS_COUNTER( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsFormatter_Int )
#define SORT_STATS_TIME( cat , name , var ) SORT_STATS_INT_TYPE( cat , name , var , StatsFormatter_ElaspedTime )
#define SORT_STATS_FCOUNTER( cat , name , var ) SORT_STATS_FLOAT_TYPE( cat , name , var , StatsFormatter_Float )
#define SORT_STATS_RATIO( cat , name , var0 , var1 ) SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , StatsFormatter_Ratio )
#define SORT_STATS_AVG_COUNT( cat , name , var0 , var1 ) SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , StatsFormatter_FloatRatio )
#define SORT_STATS_AVG_RAY_SECOND( cat , name , var0 , var1 ) SORT_STATS_RATIO_TYPE( cat , name , var0 , var1 , StatsFormatter_RayPerSecond )

#define SORT_STATS_FORMATTER( name , type ) class name{ public: static std::string ToString( type v ); };
SORT_STATS_FORMATTER( StatsFormatter_ElaspedTime , StatsInt )
SORT_STATS_FORMATTER( StatsFormatter_Int , StatsInt )
SORT_STATS_FORMATTER( StatsFormatter_Float , StatsFloat )
SORT_STATS_FORMATTER( StatsFormatter_FloatRatio , StatsData_Ratio  )
SORT_STATS_FORMATTER( StatsFormatter_Ratio , StatsData_Ratio )
SORT_STATS_FORMATTER( StatsFormatter_RayPerSecond , StatsData_Ratio  )

// StatsSummary keeps all stats data after the rendering is done
class StatsSummary {
public:
    void FlushCounter(const std::string& category, const std::string& varname, const StatsItemBase* var);
    void PrintStats() const;
    void EnableCategory(const std::string& s);

private:
    std::map<std::string, std::map<std::string, std::shared_ptr<StatsItemBase>>> counters;
    std::unordered_set<std::string> categories = { "Performance" , "Statistics" };
};

using stats_update = std::function<void(StatsSummary&)>;
class StatsItemRegister {
public:
    // Register all stats item before main function in constructors
    StatsItemRegister(const stats_update f , const std::string& cat , const std::string& name);
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
