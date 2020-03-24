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

#include "mediumdata.h"
#include "math/point.h"
#include "stream/stream.h"

float MediumDensity::Sample(const Point& uvw) const {
    return ImageTexture3D::Sample(uvw[0], uvw[1], uvw[2]);
}

void MediumDensity::Serialize(IStreamBase& stream) {
    stream >> m_width >> m_height >> m_depth;

    // make sure the dimension is valid.
    if (m_width == 0 || m_height == 0 || m_depth == 0)
        return;

    const auto tex_cnt = m_width * m_height * m_depth;

    m_memory = std::make_unique<ImgMemory<float>>();
    m_memory->m_texel = std::make_unique<float[]>(tex_cnt);
    stream.Load((char*)m_memory->m_texel.get(), sizeof(float) * tex_cnt);
}

Spectrum MediumColor::Sample(const Point& uvw) const {
    // pos needs to be transformed from world space to local space before taking a sample.
    // to be implemented
    return 0.0f;
}

void MediumColor::Serialize(IStreamBase& stream) {
    // do nothing for now
    //stream >> m_width >> m_height >> m_depth;
}