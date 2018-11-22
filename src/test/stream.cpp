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

#include "thirdparty/gtest/gtest.h"
#include "stream/fstream.h"
#include "utility/rand.h"

#define STREAM_SAMPLE_COUNT 10000

TEST(STREAM, FileStream) {
    vector<float> vec;
    {
        OFileStream ofile("test.bin");
        for (unsigned i = 0; i < STREAM_SAMPLE_COUNT; ++i) {
            vec.push_back(sort_canonical());
            ofile << vec.back();
        }
    }

    {
        IFileStream ifile("test.bin");
        for (int i = 0; i < STREAM_SAMPLE_COUNT; ++i) {
            float tmp = 0.0f;
            ifile >> tmp;
            EXPECT_EQ(tmp, vec[i]);
        }
    }
}

TEST(STREAM, MemoryStream) {

}