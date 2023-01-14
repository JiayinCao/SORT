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

#include "imagetexture3d.h"

template class ImageTexture3D<float>;
template class ImageTexture3D<Spectrum>;

template<class T>
T ImageTexture3D<T>::Sample(int x, int y, int z) const {
    if (x < 0 || x >= (int)Texture3DBase<T>::m_width || y < 0 || y >= (int)Texture3DBase<T>::m_height || z < 0 || z >= (int)Texture3DBase<T>::m_depth)
        return 0.0f;

    const auto offset = z * Texture3DBase<T>::m_width * Texture3DBase<T>::m_height + y * Texture3DBase<T>::m_width + x;
    return m_memory->m_texel[offset];
}

template<class T>
T ImageTexture3D<T>::Sample(float u, float v, float w) const{
    // There should have been proper filtering algorithms
    // However, since this is mainly for medium density for now, there will be no filter supported.
    // If the uvw is out of range, just retuen 0.0.
    if (u < 0.0f || u >= 1.0f || v < 0.0f || v >= 1.0f || w < 0.0f || w >= 1.0f)
        return 0.0f;

    const auto width    = Texture3DBase<T>::m_width;
    const auto height   = Texture3DBase<T>::m_height;
    const auto depth    = Texture3DBase<T>::m_depth;

    const auto fx = u * width - 0.5f;
    const auto fy = v * height - 0.5f;
    const auto fz = w * depth - 0.5f;

    const auto x = (unsigned)(fx);
    const auto y = (unsigned)(fy);
    const auto z = (unsigned)(fz);

    const auto dx = fx - x;
    const auto dy = fy - y;
    const auto dz = fz - z;

    const auto slice_strand = width * height;
    const auto offset00 = z * slice_strand + y * width + x;
    const auto offset01 = (x < width - 1) ? offset00 + 1 : offset00;
    const auto offset10 = (z < depth - 1) ? offset00 + slice_strand : offset00;
    const auto offset11 = (x < width - 1) ? offset10 + 1 : offset10;
    const auto offset20 = (y < height - 1) ? offset00 + width : offset00;
    const auto offset21 = (x < width - 1) ? offset20 + 1 : offset20;
    const auto offset30 = (z < depth - 1) ? offset20 + slice_strand : offset20;
    const auto offset31 = (x < width - 1) ? offset30 + 1 : offset30;

    const auto t0 = slerp(m_memory->m_texel[offset00], m_memory->m_texel[offset01], dx);
    const auto t1 = slerp(m_memory->m_texel[offset10], m_memory->m_texel[offset11], dx);
    const auto t2 = slerp(m_memory->m_texel[offset20], m_memory->m_texel[offset21], dx);
    const auto t3 = slerp(m_memory->m_texel[offset30], m_memory->m_texel[offset31], dx);

    const auto t02 = slerp(t0, t2, dy);
    const auto t13 = slerp(t1, t3, dy);
    
    return slerp(t02, t13, dz);
}