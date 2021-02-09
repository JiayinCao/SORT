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

#pragma once

struct RenderContext;

// Ideally, I should do some research and have a proper random number generation algorithm since this is so
// important in lots of parts of the renderer. Since it works, I'll leave it this way for now.
#define MT_CNT  624
class RandomNumberGenerator{
public:
    RandomNumberGenerator();

    // Generate a random unsigned number
    unsigned    rand();

private:
    unsigned long mt[MT_CNT]; /* the array for the state vector  */
    int mti;
    bool seed_setup = false;

    void init();
};

template<class T>
T sort_rand( RenderContext& rc );
