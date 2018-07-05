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
#include "define.h"
#include "log/log.h"
#include "utility/sassert.h"
#include "utility/strhelper.h"

#ifdef SORT_ENABLE_STATS_COLLECTION
// This is a container holding all statsItem per thread
class StatsItemContainer {
public:
    void Register(const StatsItem& item) {
        container.push_back(item);
    }
    void FlushData() {
        sAssert(!flushed, LOG_GENERAL);
        for (const StatsItem& item : container)
            item.FlushData();
        flushed = true;
    }

private:
    // Container for all stats per thread
    std::vector<StatsItem> container;
    // To make sure we don't flush data twice
    bool flushed = false;
};

void StatsSummary::PrintStats() const {
    slog(INFO, GENERAL, "-------------------------Statistics-------------------------");
    std::map<std::string, std::map<std::string, std::string>> outputs;
    for (const auto& counterCat : counters) {
        for (const auto& counterItem : counterCat.second) {
            outputs[counterCat.first][counterItem.first] = to_string(counterItem.second);
        }
    }
    for (const auto& counterCat : countersFloat) {
        for (const auto& counterItem : counterCat.second) {
            outputs[counterCat.first][counterItem.first] = to_string(counterItem.second);
        }
    }

    for (const auto& counterCat : outputs) {
        slog(INFO, GENERAL, stringFormat("%s", counterCat.first.c_str()));
            for (const auto& counterItem : counterCat.second) {
                slog(INFO, GENERAL, stringFormat("    %-42s %s", counterItem.first.c_str() , counterItem.second.c_str()));
            }
    }
    slog(INFO, GENERAL, "-------------------------Statistics-------------------------");
}

static StatsSummary                     g_StatsSummary;
static Thread_Local StatsItemContainer  g_StatsItemContainer;

// Recording all necessary data in constructor
StatsItem::StatsItem( const stats_update f ): func(f) 
{
    static std::mutex statsMutex;
    std::lock_guard<std::mutex> lock(statsMutex);
    g_StatsItemContainer.Register(*this);
}

// Flush the data into StatsSummary
void StatsItem::FlushData() const
{
    sAssert(func, LOG_GENERAL);
    func(g_StatsSummary);
}
#endif

void FlushStatsData()
{
    SORT_STATS(g_StatsItemContainer.FlushData());
}
void PrintStatsData()
{
    SORT_STATS(g_StatsSummary.PrintStats());
}