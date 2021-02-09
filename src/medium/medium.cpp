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

#include "medium.h"
#include "material/material.h"
#include "core/rand.h"

bool MediumStack::AddMedium(const Medium* medium) {
    // simply return false if there is no space, this should rarely happen unless there is more than 8 volumes overlap.
    if (m_mediumCnt >= MEDIUM_MAX_CNT)
        return false;

    // if there is existed medium in the stack, nothing needs to be done.
    const auto material_src = medium->GetMaterial();
    for (auto i = 0u; i < m_mediumCnt; ++i) {
        const auto material_i = m_mediums[i]->GetMaterial();
        if (material_i == material_src)
            return false;
    }

    m_mediums[m_mediumCnt++] = medium;
    return true;
}

bool MediumStack::RemoveMedium(const StringID medium_id) {
    // if there is no medium in the container, simply return false
    if (0 == m_mediumCnt)
        return false;

    // find the medium to be removed. Unfortunately, due to the lack of detail in data, the medium to be removed could be any
    // medium in the data structure, it is necessary to iterate through everything to find it.
    const Medium* last_medium = m_mediums[m_mediumCnt - 1];
    for (int i = m_mediumCnt - 1; i >= 0; --i) {
		const auto material_i	= m_mediums[i]->GetMaterial();
		const auto medium_id_i	= material_i ? material_i->GetUniqueID() : INVALID_SID;
        if (medium_id_i == medium_id) {
            m_mediums[i] = last_medium;
            --m_mediumCnt;
            return true;
        }
    }

    return false;
}

Spectrum MediumStack::Tr(const Ray& r, const float max_t, RenderContext& rc) const {
    if (0 == m_mediumCnt)
        return 1.0f;

    const auto k = clamp((int)(sort_rand<float>(rc) * m_mediumCnt), 0, m_mediumCnt - 1);
    const Medium* medium = m_mediums[k];
    return medium->Tr(r, max_t, rc) * m_mediumCnt;
}

Spectrum MediumStack::Sample(const Ray& r, const float max_t , MediumInteraction*& mi, Spectrum& emission, RenderContext& rc) const {
    if (0 == m_mediumCnt)
        return 1.0f;

    const auto k = clamp((int)(sort_rand<float>(rc) * m_mediumCnt), 0, m_mediumCnt - 1);
    const Medium* medium = m_mediums[k];
    return medium->Sample(r, max_t, mi, emission, rc) * m_mediumCnt;
}