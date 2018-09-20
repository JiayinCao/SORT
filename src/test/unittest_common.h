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

#include <functional>
#include <thread>
#include <vector>

// TN : thread number
// N :  task number in each thread
template<typename T , int TN , int N = 1024 * 1024 * 8 >
T ParrallReduction( std::function<T()> func ){
    // executing tasks in different threads
    std::vector<T> total( TN , 0.0f );
    std::thread threads[TN];
    for (int i = 0; i < TN; ++i)
        threads[i] = std::thread([&]( int tid ){
            T local = 0.0f;
            for (long long j = 0; j < N; ++j )
                local += func() * ( (double)1.0 / (double) ( N * TN ) );
            total[tid] += local;
        }, i);

    // make sure all threads are finished
    for (int i = 0; i < TN; ++i)
        threads[i].join();
    
    // collect results from each thread
    T final_total = 0.0f;
    for (int i = 0; i < TN; ++i)
        final_total += total[i];
    return final_total;
}

template<int TN , int N = 1024 * 1024 * 8 >
void ParrallRun( std::function<void()> func ){
    // executing tasks in different threads
    std::thread threads[TN];
    for (int i = 0; i < TN; ++i)
        threads[i] = std::thread([&](){
            for (long long j = 0; j < N; ++j )
                func();
        });

    // make sure all threads are finished
    for (int i = 0; i < TN; ++i)
        threads[i].join();
}