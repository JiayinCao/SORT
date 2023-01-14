/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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
#include "unit_tests.h"

void UnitTests::StartRunning(int argc, char** argv) {
    // we don't care the about the stream anymore, which is probably not even valid depending on how we get here
    // simply run all unit tests and see what happens.
    ::testing::InitGoogleTest(&argc, argv);
    m_result = RUN_ALL_TESTS();
}

int UnitTests::WaitForWorkToBeDone() {
    return m_result;
}