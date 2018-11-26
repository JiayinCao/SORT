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

#include "stats.h"

#ifdef SORT_ENABLE_STATS_COLLECTION

// This is a container holding all statsItem per thread
class StatsItemContainer {
public:
    void Register(const StatsItemRegister* item , const std::string& cat , const std::string& name ) {
        if( registered[cat].count(name) )
            return;
        registered[cat].insert(name);
        container.push_back(item);
    }
    void FlushData() const {
        for (const StatsItemRegister* item : container)
            item->FlushData();
    }

private:
    // Container for all stats per thread
    std::vector<const StatsItemRegister*> container;
    std::unordered_map<std::string,std::unordered_set<std::string>> registered;
};

void StatsSummary::PrintStats() const {
    slog(INFO, GENERAL, "----------------------------------------------------------------");
    std::map<std::string, std::map<std::string, std::string>> outputs;
    for (const auto& counterCat : counters) {
        if( categories.count( counterCat.first ) == 0 )
            continue;
        for (const auto& counterItem : counterCat.second) {
            outputs[counterCat.first][counterItem.first] = counterItem.second->ToString();
        }
    }

    for (const auto& counterCat : outputs) {
        slog(INFO, GENERAL, stringFormat("%s", counterCat.first.c_str()));
            for (const auto& counterItem : counterCat.second) {
                slog(INFO, GENERAL, stringFormat("    %-44s %s", counterItem.first.c_str() , counterItem.second.c_str()));
            }
    }
    slog(INFO, GENERAL, "----------------------------------------------------------------");
}

void StatsSummary::FlushCounter(const std::string& category, const std::string& varname, const StatsItemBase* var) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (counters[category].count(varname) == 0)
        counters[category][varname] = var->MakeItem();
    if (categories.count(category))
        counters[category][varname]->Merge(var);
}

void StatsSummary::EnableCategory(const std::string& s) {
    categories.insert(s);
}

static StatsSummary             g_StatsSummary;
static StatsItemContainer*      g_pStatsItemContainer = nullptr;

// Recording all necessary data in constructor
StatsItemRegister::StatsItemRegister( const stats_update f , const std::string& cat , const std::string& name ): func(f){
    static std::mutex statsMutex;
    std::lock_guard<std::mutex> lock(statsMutex);
    
    if( nullptr == g_pStatsItemContainer )
        g_pStatsItemContainer = new StatsItemContainer();
    g_pStatsItemContainer->Register(this, cat, name);
}

// Flush the data into StatsSummary
void StatsItemRegister::FlushData() const
{
    sAssert(func, GENERAL);
    func(g_StatsSummary);
}

std::string StatsFormatter_Int::ToString(StatsInt v){
    auto s = std::to_string(v);
    if( s.size() < 5 )
        return s;
    int len = (int)s.size() - 1;
    std::string ret( len + 1 + len / 3 , ',' );
    int i = 0 , j = (int)ret.size() - 1;
    while( i < (int)s.size() ){
        ret[j--] = s[len - (i++)];
        if( i % 3 == 0 )
            --j;
    }
    return ret;
}

std::string StatsFormatter_ElaspedTime::ToString(StatsInt v ){
    if( v < 1000 ) return stringFormat("%d(ms)" , v);
    if( v < 60000 ) return stringFormat("%.2f(s)" , (StatsFloat)v/1000.0f); v /= 1000;
    if( v < 3600 ) return stringFormat( "%d(m)%d(s)" , v/60 , v%60 ); v /= 60;
    if( v < 1440 ) return stringFormat( "%d(h)%d(m)" , v/60 , v%60 );
    return stringFormat( "%d(d)%d(h)%d(m)" , v / 1440 , ( v % 1440 ) / 60 , v % 60 );
}

std::string StatsFormatter_Float::ToString(StatsFloat v ){
    return stringFormat("%.2f",v);
}

std::string StatsFormatter_Ratio::ToString( StatsData_Ratio ratio ){
    StatsFloat r = (StatsFloat)ratio.nominator / (StatsFloat)ratio.denominator;
    return stringFormat("%.2f%%",r * 100);
}

std::string StatsFormatter_FloatRatio::ToString( StatsData_Ratio ratio ){
    StatsFloat r = (StatsFloat)ratio.nominator / (StatsFloat)ratio.denominator;
    return stringFormat("%.2f",r);
}

std::string StatsFormatter_RayPerSecond::ToString( StatsData_Ratio ratio ){
    StatsFloat r = (StatsFloat)ratio.nominator / (StatsFloat)ratio.denominator * 0.001f;
    return stringFormat("%.2f(MRay/s)",r);
}

#endif

void SortStatsFlushData( bool mainThread ){
#ifdef SORT_ENABLE_STATS_COLLECTION
    if( !g_pStatsItemContainer ) return;
    g_pStatsItemContainer->FlushData();
    if( mainThread ) SAFE_DELETE(g_pStatsItemContainer); // don't need it anymore
#endif
}
void SortStatsPrintData(){
    SORT_STATS(g_StatsSummary.PrintStats());
}
void SortStatsEnableCategory( const std::string& s ){
    SORT_STATS(g_StatsSummary.EnableCategory(s));
}
