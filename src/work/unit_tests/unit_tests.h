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

#pragma once

#include "../work.h"

//! @brief  This is a work for unit testing.
class UnitTests : public Work {
public:
    DEFINE_RTTI(UnitTests, Work);

    //! @brief  Start work evaluation.
    //!
    //! @param stream       The stream as input.
    void    StartRunning(int argc, char** argv) override;

    //! @brief  Wait for all the unit tests to be done.
    int     WaitForWorkToBeDone() override;

private:
    int     m_result = 0;
};