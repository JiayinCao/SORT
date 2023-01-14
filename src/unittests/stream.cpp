/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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
#include "stream/mstream.h"
#include "core/rand.h"
#include "core/render_context.h"
#include "unittest_common.h"

using namespace unittest;

#define STREAM_SAMPLE_COUNT 10000

TEST(STREAM, FileStream) {
    RenderContext rc;
    rc.Init();

    std::vector<float>           vec_f;
    std::vector<int>             vec_i;
    std::vector<unsigned int>    vec_u;
    OFileStream ofile("test.bin");
    std::string str = "this is a random string";
    ofile<<str;
    bool flag = true;
    ofile<<flag;
    std::string empty_str = "";
    ofile<<empty_str;
    for (unsigned i = 0; i < STREAM_SAMPLE_COUNT; ++i) {
        vec_f.push_back( sort_rand<float>(rc) );
        vec_i.push_back( (int)( ( 2.0f * sort_rand<float>(rc) - 1.0f ) * STREAM_SAMPLE_COUNT ) );
        vec_u.push_back( (unsigned int)( sort_rand<float>(rc) * STREAM_SAMPLE_COUNT ) );
        ofile << vec_f.back() << vec_i.back() ;
        ofile << vec_u.back();
    }
    ofile.Close();

    IFileStream ifile("test.bin");
    std::string str_copy;
    ifile>>str_copy;
    EXPECT_EQ( str_copy , str );
    bool flag_copy = false;
    ifile>>flag_copy;
    EXPECT_EQ( flag_copy , flag );
    std::string empty_str_copy;
    ifile>>empty_str_copy;
    EXPECT_EQ( empty_str_copy , empty_str );
    for (int i = 0; i < STREAM_SAMPLE_COUNT; ++i) {
        float t0 = 0.0f;
        int t1 = 0;
        unsigned int t2 = 0;
        ifile >> t0 >> t1 >> t2;
        EXPECT_EQ(t0, vec_f[i]);
        EXPECT_EQ(t1, vec_i[i]);
        EXPECT_EQ(t2, vec_u[i]);
    }
}

TEST(STREAM, MemoryStream) {
    RenderContext rc;
    rc.Init();
    
    std::vector<float>           vec_f;
    std::vector<int>             vec_i;
    std::vector<unsigned int>    vec_u;
    OMemoryStream ostream;
    std::string str = "this is a random string";
    ostream <<str;
    bool flag = true;
    ostream <<flag;
    std::string empty_str = "";
    ostream <<empty_str;
    for (unsigned i = 0; i < STREAM_SAMPLE_COUNT; ++i) {
        vec_f.push_back( sort_rand<float>(rc) );
        vec_i.push_back( (int)( ( 2.0f * sort_rand<float>(rc) - 1.0f ) * STREAM_SAMPLE_COUNT ) );
        vec_u.push_back( (unsigned int)( sort_rand<float>(rc) * STREAM_SAMPLE_COUNT ) );
        ostream << vec_f.back() << vec_i.back() ;
        ostream << vec_u.back();
    }

    IMemoryStream istream(ostream.GetData(), ostream.GetDataSize());
    std::string str_copy;
    istream >>str_copy;
    EXPECT_EQ( str_copy , str );
    bool flag_copy = false;
    istream >>flag_copy;
    EXPECT_EQ( flag_copy , flag );
    std::string empty_str_copy;
    istream >>empty_str_copy;
    EXPECT_EQ( empty_str_copy , empty_str );
    for (int i = 0; i < STREAM_SAMPLE_COUNT; ++i) {
        float t0 = 0.0f;
        int t1 = 0;
        unsigned int t2 = 0;
        istream >> t0 >> t1 >> t2;
        EXPECT_EQ(t0, vec_f[i]);
        EXPECT_EQ(t1, vec_i[i]);
        EXPECT_EQ(t2, vec_u[i]);
    }
}